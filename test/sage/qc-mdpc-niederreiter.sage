#!/usr/bin/env sage
# 
# Reference implementation of the Niederreiter hybrid cryptosystem.
# 
# For the asymmetric part, the system uses Quasi-Cyclic Medium-Density
# Parity-Check codes (QC-MDPC) for the Key Encapsulation Method
# (KEM). For the symmetric part, the system uses the default secretbox
# implementation from libnacl for the Data Encapsulation Method (DEM).
# The DEM secret key is derived from the KEM plaintext error using the
# default hash function from libnacl.
#
# THIS IS NOT A SECURE IMPLEMENTATION!!! USE THIS ONLY FOR
# DEBUGGING/TESTING!!!
# 
# In particular, the returned exception leaks if decryption failed
# because of a decoding failure or because of a MAC failure.  All
# operations are in non-constant time, leading to side-channel
# vulnerabilities.  Besides insecure, this implementation is also
# horribly slow.
# 
# Author: Sebastian R. Verschoor (srverschoor@uwaterloo.ca)
# License: TODO

# TODO: some bug prevents this from working...
# from __future__ import print_function

import sys
import struct
import base64
import argparse
import libnacl

index_bytes = 2

args = argparse.Namespace(verbose=1)

class Util:
    @staticmethod
    def gen_vec(length, weight):
        indices = Permutations(length, weight).random_element()
        return vector(GF(2), length, dict((index,True) for index in indices))

    @staticmethod
    def gen_cyclic(first_row):
        size = len(first_row)
        if (first_row.is_sparse()):
            first_row_dict = first_row.dict()
            matrix_dict = dict(((x,(x+y)%size), True) for y in first_row_dict for x in xrange(size))
            return matrix(GF(2), size, size, matrix_dict)
        else:
            return matrix(GF(2), size, size, lambda x, y: first_row[(y-x)%size])

    @staticmethod
    def last_block(matrix):
        [], blocks = matrix.subdivisions()
        return matrix.subdivision(0, len(blocks))
    
    @staticmethod
    def pack_vec(vec):
        if vec.is_sparse():
            return struct.pack('!' + 'H' * vec.hamming_weight(), *(vec.dict().keys()))
        else:
            size = (vec.length() + 7) // 8
            byte_array = [0] * size
            for i, bit in enumerate(vec):
                byte_array[i//8] |= int(bit) << (7-i%8)
            return struct.pack('!' + 'B' * size, *byte_array)

    @staticmethod
    def unpack_vec(string, length, is_sparse):
        global index_bytes
        if is_sparse:
            weight = len(string) // index_bytes
            indices = struct.unpack('!' + 'H' * weight, string)
            return vector(GF(2), length, dict((index, True) for index in indices))
        else:
            vec = (GF(2)^length)(0)
            byte_array = struct.unpack('!' + 'B' * len(string), string)
            for i in xrange(length):
                vec[i] = (byte_array[i//8] >> (7-i%8)) & 1
            return vec
    
    @staticmethod
    def pack_pub_key(pub_key):
        [], blocks = pub_key.subdivisions()
        return "".join([Util.pack_vec(pub_key.subdivision(0, b)[0]) for b in xrange(len(blocks))])

    @staticmethod
    def unpack_pub_key(string, block_count, block_size):
        block_bytes = (block_size + 7) // 8
        blocks = [None] * block_count
        for b in xrange(block_count - 1):
            substring = string[b * block_bytes : (b+1) * block_bytes]
            blocks[b] = Util.gen_cyclic(Util.unpack_vec(substring, block_size, False))
        blocks[-1] = 1
        return block_matrix(blocks, ncols=block_count)

    @staticmethod
    def pack_priv_key(priv_key):
        [], blocks = priv_key.subdivisions()
        string = ""
        for b in xrange(len(blocks)):
            string += Util.pack_vec(priv_key.subdivision(0, b).transpose()[0])
        string += Util.pack_vec(priv_key.subdivision(0, len(blocks))[0])
        return string

    @staticmethod
    def unpack_priv_key(string, block_count, block_weight, block_size):
        global index_bytes
        block_bytes = block_weight * index_bytes
        blocks = [None] * block_count
        for b in xrange(block_count - 1):
            substring = string[b * block_bytes : (b+1) * block_bytes]
            blocks[b] = Util.gen_cyclic(Util.unpack_vec(substring, block_size, True)).transpose()
        substring = string[(block_count - 1) * block_bytes :] 
        blocks[block_count - 1] = Util.gen_cyclic(Util.unpack_vec(substring, block_size, True))
        return block_matrix(blocks, ncols=block_count)

    @staticmethod
    def pack_error(error, block_count, block_size):
        string = ""
        for b in xrange(block_count):
            error_slice = error[b * block_size : (b+1) * block_size]
            string += Util.pack_vec(error_slice.dense_vector())
        return string
    
    @staticmethod
    def pack_error_sparse(error):
        return Util.pack_vec(error)

    @staticmethod
    def unpack_error_sparse(string, block_count, block_size):
        return Util.unpack_vec(string, block_count * block_size, True)
        
    @staticmethod
    def print_vec(vec):
        if vec.is_sparse():
            print(sorted(vec.dict().keys()))
        else:
            print base64.b64encode(Util.pack_vec(vec))

    @staticmethod
    def parse_input_selftest(block_count, block_size, block_weight, error_weight):
        f_in = sys.stdin
        keys = struct.unpack('H', f_in.read(2))[0]
        messages = struct.unpack('H', f_in.read(2))[0]
        data = [type('', (), {})] * keys # [extendable_object] * keys
        for k in range(keys):
            data[k].priv = Util.unpack_priv_key(f_in.read(key_len), block_count, block_size)
            data[k].err = [None] * messages
            data[k].msg = [None] * messages
            for m in range(messages):
                data[k].err[m] = Util.unpack_error(f_in.read(err_len), block_count, block_size)
                data[k].msg[m] = f_in.read(msg_len)
        return keys, messages, data


class DecodingFailure(Exception):
    pass
    

class Niederreiter(object):
    def __init__(self, block_count, block_size, row_weight, error_weight, thresholds):
        assert is_odd(row_weight), "row weight is even (non-invertible)"
        self.block_count = block_count
        self.block_size = block_size
        self.row_weight = row_weight
        self.size = block_count * block_size
        self.error_weight = error_weight
        self.thresholds = thresholds
    
    def keygen(self, private_key=None):
        if private_key is None:
            private_key = self._parity_check()
        public_key = self._systematic(private_key)
        return public_key, private_key

    def _parity_check(self):
        first_rows = self._gen_first_rows()
        blocks = [Util.gen_cyclic(first_row) for first_row in first_rows]
        while not blocks[-1].is_invertible():
            blocks[-1] = Util.gen_cyclic(self._gen_first_row())
        return block_matrix(blocks, ncols=self.block_count)

    def _gen_first_rows(self):
        return [self._gen_first_row() for _ in xrange(self.block_count)]
        
    def _gen_first_row(self):
        return Util.gen_vec(self.block_size, self.row_weight)

    def _systematic(self, parity_check):
        last_block = Util.last_block(parity_check).dense_matrix()
        return (1 / last_block) * parity_check

    def gen_error(self):
        return Util.gen_vec(self.size, self.error_weight)

    def encrypt(self, error, systematic):
        assert len(error) == self.size, "len(error) is incorrect"
        assert error.hamming_weight() == self.error_weight, "weight(error) is incorrect"
        return systematic * error
    
    def decrypt(self, public_syndrome, private_key):
        private_syndrome = self._private_syndrome(public_syndrome, private_key)
        if args.verbose >= 2:
            print("### Private syndrome")
            Util.print_vec(private_syndrome.dense_vector())
        return self._decode(private_syndrome, private_key)

    def _private_syndrome(self, public_syndrome, parity_check):
        last_block = Util.last_block(parity_check)
        return last_block * public_syndrome
    
    def _decode(self, private_syndrome, private_key):
        error_candidate = (GF(2)^self.size)(0)
        if args.verbose >= 3:
            print("### Decoding")
        for i, threshold in enumerate(self.thresholds):
            if args.verbose >= 3:
                print("#### Decoder round {} (threshold {})".format(i, threshold))
            syndrome_update = (GF(2)^self.block_size)(0)
            for column_index, column in enumerate(private_key.columns()):
                error_count = column.pairwise_product(private_syndrome).hamming_weight()
                if error_count >= threshold:
                    error_candidate[column_index] += 1
                    syndrome_update += column
            private_syndrome += syndrome_update
            if args.verbose >= 3:
                print("error candidate:")
                Util.print_vec(error_candidate.sparse_vector())
                print("syndrome update:")
                Util.print_vec(syndrome_update)
                print("syndrome:")
                Util.print_vec(private_syndrome.dense_vector())
        if args.verbose >= 3:
            print("")
        if private_syndrome == 0:
            return error_candidate
        raise DecodeFailure("Could not decode syndrome")


class DEM:
    @staticmethod
    def encrypt(plaintext, secret_key):
        nonce = b"\x00" * libnacl.crypto_secretbox_NONCEBYTES
        return libnacl.crypto_secretbox(plaintext, nonce, secret_key)
    
    @staticmethod
    def decrypt(ciphertext, secret_key):
        nonce = b"\x00" * libnacl.crypto_secretbox_NONCEBYTES
        return libnacl.crypto_secretbox_open(ciphertext, nonce, secret_key)


class KEMDEM(Niederreiter):
    def encrypt(self, plaintext, public_key, error=None):
        if error is None:
            error = self.gen_error()
        error_bytes = Util.pack_error(error, self.block_count, self.block_size)
        secret_key = libnacl.crypto_hash(error_bytes)[:libnacl.crypto_secretbox_KEYBYTES]
        if args.verbose >= 2:
            print("#### Secret key")
            print(base64.b64encode(secret_key))
        public_syndrome = super(KEMDEM, self).encrypt(error, public_key)
        ciphertext = DEM.encrypt(plaintext, secret_key)
        return public_syndrome, ciphertext
        
    def decrypt(self, (public_syndrome, ciphertext), private_key):
        error = super(KEMDEM, self).decrypt(public_syndrome, private_key)
        error_bytes = Util.pack_error(error, self.block_count, self.block_size)
        secret_key = libnacl.crypto_hash(error_bytes)[:libnacl.crypto_secretbox_KEYBYTES]
        return DEM.decrypt(ciphertext, secret_key)

class Test:
    def __init__(self, block_count, block_size, row_weight, error_weight, thresholds):
        self.N = Niederreiter(block_count, block_size, row_weight, error_weight, thresholds)
        self.KD = KEMDEM(block_count, block_size, row_weight, error_weight, thresholds)
        if args.verbose >= 1:
            print("Test initialized with parameters:")
            print("block_count: {}".format(block_count))
            print("block_size: {}".format(block_size))
            print("row_weight: {}".format(row_weight))
            print("error_weight: {}".format(error_weight))
            print("thresholds: {}".format(thresholds))
        
    def self_test_pack_unpack(self, test_rounds=5):
        block_count = self.N.block_count
        block_size = self.N.block_size
        row_weight = self.N.row_weight
        error_weight = self.N.error_weight
        for _ in xrange(test_rounds):
            vec = Util.gen_vec(block_size, row_weight)
            copy = Util.unpack_vec(Util.pack_vec(vec), block_size, True)
            assert copy == vec, "failed to pack/unpack sparse vector"
            vec = vec.dense_vector()
            copy = Util.unpack_vec(Util.pack_vec(vec), block_size, False)
            assert copy == vec, "failed to pack/unpack dense vector"
            pub, priv = self.N.keygen()
            copy = Util.unpack_pub_key(Util.pack_pub_key(pub),
                                       block_count, block_size)
            assert copy == pub, "failed to pack/unpack public key"
            copy = Util.unpack_priv_key(Util.pack_priv_key(priv),
                                        block_count, row_weight, block_size)
            assert copy == priv, "failed to pack/unpack private key"
            err = self.N.gen_error()
            copy = Util.unpack_error_sparse(Util.pack_error_sparse(err),
                                            block_count, block_size)
            assert copy == err, "failed to pack/unpack (sparse) error"
        return True

    def test_KEM_keygen(self, priv, expected):
        pass

    def test_KEM_encrypt(self, err, pub, expected):
        pass
    
    def test_KEM_decrypt(self, syn, priv, expected):
        pass

    def test_KEMDEM_encrypt(self, (pt, err), pub, expected):
        pass

    def test_KEMDEM_decrypt(self, (syn, ct), priv, expected):
        pass
    
    def self_test_correctness(self, (keys, messages, data)):
        test_failed = False
        decoding_failures = 0
        for k in xrange(keys):
            if args.verbose >= 1:
                print("Testing key {}".format(k)) 
            if data is None:
                pub, priv = self.N.keygen()
            else:
                pub, priv = self.N.keygen(data.key[k])
            assert Util.last_block(pub) == 1, "keygen failed (last block != 1)"
            for m in xrange(messages):
                if args.verbose >= 1:
                    print("Testing KEM {}.{}:".format(k,m))
                if data is None:
                    e = self.N.gen_error()
                else:
                    e = data.error[m]
                ct = self.N.encrypt(e, pub)
                try:
                    pt = self.N.decrypt(ct, priv)
                except DecodingFailure as df:
                    if args.verbose >= 1:
                        print(df)
                        print("priv_key: {}".format(priv))
                        print("error: {}".format(e))
                    decoding_failures += 1
                else:
                    if e != pt:
                        if args.verbose >= 1:
                            print("Decoded to wrong error")
                            print("priv_key: {}".format(priv))
                            print("error: {}".format(e))
                        test_failed = True
                    elif args.verbose >= 1:
                        print("Test succeeded")

                if args.verbose >= 1:
                    print("Testing KEM/DEM: {}".format(m))
                if data is None:
                    msg = libnacl.randombytes(int(64))
                else:
                    msg = data.msg[m]
                ct = self.KD.encrypt(msg, pub)
                try:
                    pt = self.KD.decrypt(ct, priv)
                except DecodingFailure as df:
                    if args.verbose >= 1:
                        print(df)
                        print("priv_key: {}".format(priv))
                        print("error: {}".format(e))
                        print("msg: {}".format(msg))
                    decoding_failures += 1
                except ValueError as ve:
                    if args.verbose >= 1:
                        print(ve)
                        print("priv_key: {}".format(priv))
                        print("error: {}".format(e))
                        print("msg: {}".format(msg))
                    test_failed = True
                else:
                    if msg != pt:
                        if args.verbose >= 1:
                            print("Decrypted to wrong value")
                            print("priv_key: {}".format(priv))
                            print("error: {}".format(e))
                            print("msg: {}".format(msg))
                        test_failed = True
                    elif args.verbose >= 1:
                        print("Test succeeded")
                        
        if args.verbose >= 1:
            if decoding_failures > 0:
                print("{} decoding failures".format(decoding_failures))
            if not test_failed:
                print("All tests passed")

        return decoding_failures > 0 or test_failed 
    

def gen_test_vectors(keys, messages):
    block_count = 2
    block_size = 4801
    row_weight = 45
    error_weight = 84
    thresholds = [29, 27, 25, 24, 23, 23]
    msg_bytes = int(16)
    KD = KEMDEM(block_count, block_size, row_weight, error_weight, thresholds)
    
    print("# Parameters")
    print("block_count: {}".format(block_count))
    print("block_size: {}".format(block_size))
    print("row_weight: {}".format(row_weight))
    print("error_weight: {}".format(error_weight))
    print("thresholds: {}".format(thresholds))
    print("")
    for k in xrange(keys):
        print("## Key generation")
        pub, priv = KD.keygen()
        print("### Private key")
        for b in xrange(block_count):
            print("block {}:".format(b))
            Util.print_vec(priv.subdivision(0,b)[0])
        print("### Public key")
        for b in xrange(block_count - 1):
            print("block {}".format(b))
            Util.print_vec(pub.subdivision(0, b)[0])
        print("")
        for m in xrange(messages):
            print("## Encryption")
            err = KD.gen_error()
            print("### Error")
            Util.print_vec(err)
            msg = libnacl.randombytes(msg_bytes)
            print("### Message")
            print(base64.b64encode(msg))
            pub_syn, ct = KD.encrypt(msg, pub, err)
            print("### Public syndrome")
            Util.print_vec(pub_syn)
            print("### Ciphertext")
            print(base64.b64encode(ct))
            print("")
            print("## Decryption")
            pt = KD.decrypt((pub_syn, ct), priv)
            print("")

def main():
    global args
    err_code = 0

    ap = argparse.ArgumentParser()
    ap.add_argument("-s", "--selftest",
                    action="store_true",
                    help="run internal consistency tests")
    ap.add_argument("-g", "--generate",
                    action="store_true",
                    help="generate test vectors")
    ap.add_argument("-k", "--keys",
                    type=int, default=1,
                    help="number of random keys to be tested (default: 5)")
    ap.add_argument("-m", "--messages",
                    type=int, default=2,
                    help="number of random messages to be tested (default: 5)")
    ap.add_argument("-d", "--data",
                    action="store_true",
                    help="provide data from stdin (default: random data)")
    ap.add_argument("-v", "--verbose",
                    action="count", default=0,
                    help="increase output verbosity")
    args = ap.parse_args()
    if args.selftest:
        test = Test(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        if args.data:
            data = Util.parse_input_selftest(4801);
            err_code  = test.self_test_correctness(data);
            err_code |= test.self_test_pack_unpack()
        else:
            err_code  = test.self_test_correctness((args.keys, args.messages, None))
            err_code |= test.self_test_pack_unpack()
    elif args.generate:
        gen_test_vectors(args.keys, args.messages)

    sys.exit(err_code)


if __name__ == "__main__":
    try:
        get_ipython()
    except NameError:
        # only run automatically when not in the repl
        main()
    else:
        # set globals as helper variables in the repl
        global gN
        global gKD
        global gT
        gN = Niederreiter(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        gKD = KEMDEM(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        gT = Test(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
