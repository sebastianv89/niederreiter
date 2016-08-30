#!/usr/bin/env sage
# 
# Reference implementation of the Niederreiter hybrid cryptosystem.
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

import argparse
import libnacl
import struct
import sys

args = argparse.Namespace(verbose=1)

class Util:
    @staticmethod
    def gen_vec(length, weight, indices=None):
        if indices is None:
            indices = Permutations(length, weight).random_element()
        assert len(indices) == weight, "Indices have incorrect weight"

        return vector(GF(2), length, dict((index,True) for index in indices))

    @staticmethod
    def gen_cyclic(first_row):
        size = len(first_row)
        first_row_dict = first_row.dict()
        matrix_dict = dict(((x,(x+y)%size), True) for y in first_row_dict for x in xrange(size))
        return matrix(GF(2), size, size, matrix_dict, sparse=True)
        
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
    def unpack_vec(string, size, is_sparse):
        if is_sparse:
            weight = len(string) // 2
            indices = struct.unpack('!' + 'H' * weight, string)
            return Util.gen_vec(size, weight, indices)
        else:
            vec = (GF(2)^size)(0)
            byte_array = struct.unpack('!' + 'B' * len(string), string)
            for i in xrange(size):
                vec[i] = (byte_array[i//8] >> (7-i%8)) & 1
            return vec

    @staticmethod
    def pack_pubkey():
        pass
    
    @staticmethod
    def unpack_pubkey():
        pass
        
    @staticmethod
    def pack_privkey():
        pass

    @staticmethod
    def unpack_privkey():
        pass


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
    
    def keygen(self, first_rows_ids=None):
        private_key = self._parity_check(first_rows_ids)
        public_key = self._systematic(private_key)
        return public_key, private_key

    def _parity_check(self, first_rows_ids=None):
        first_rows = self._gen_first_rows(first_rows_ids)
        blocks = [Util.gen_cyclic(first_row) for first_row in first_rows]
        while not blocks[-1].is_invertible():
            blocks[-1] = Util.gen_cyclic(self._gen_first_row(None))
        return block_matrix(blocks, ncols=self.block_count)

    def _gen_first_rows(self, indices_list=None):
        if indices_list is None:
            return [self._gen_first_row() for _ in xrange(self.block_count)]
        assert len(indices_list) == self.block_count, "indices list has incorrect length"
        return [self._gen_first_row(indices) for indices in indices_list]
        
    def _gen_first_row(self, indices=None):
        return Util.gen_vec(self.block_size, self.row_weight, indices)

    def _systematic(self, parity_check):
        last_block = Util.last_block(parity_check).dense_matrix()
        last_block_inv = last_block.inverse()
        return last_block_inv * parity_check

    def gen_error(self, indices=None):
        return Util.gen_vec(self.size, self.error_weight, indices)

    def encrypt(self, error, systematic):
        assert len(error) == self.size, "len(error) is incorrect"
        assert error.hamming_weight() == self.error_weight, "weight(error) is incorrect"
        return systematic * error
    
    def decrypt(self, public_syndrome, private_key):
        print "pub syn hw (", public_syndrome.hamming_weight(), ")"
        private_syndrome = self._private_syndrome(public_syndrome, private_key)
        print "priv syn hw (", private_syndrome.hamming_weight(), ")"
        return self._decode(private_syndrome, private_key)

    def _private_syndrome(self, public_syndrome, parity_check):
        last_block = Util.last_block(parity_check)
        return last_block * public_syndrome
    
    def _decode(self, private_syndrome, private_key):
        error_candidate = (GF(2)^self.size)(0)
        for threshold in self.thresholds:
            count = 0
            print "threshold", threshold
            syndrome_update = (GF(2)^self.block_size)(0)
            for column_index, column in enumerate(private_key.columns()):
                error_count = column.pairwise_product(private_syndrome).hamming_weight()
                if error_count >= threshold:
                    count += 1
                    error_candidate[column_index] += 1
                    syndrome_update += column
            print "potential errors found", count
            private_syndrome += syndrome_update
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
    def encrypt(self, plaintext, public_key):
        error = self.gen_error()
        secret_key = libnacl.crypto_hash(str(error))[:libnacl.crypto_secretbox_KEYBYTES]
        public_syndrome = super(KEMDEM, self).encrypt(error, public_key)
        ciphertext = DEM.encrypt(plaintext, secret_key)
        return public_syndrome, ciphertext
        
    def decrypt(self, (public_syndrome, ciphertext), private_key):
        error = super(KEMDEM, self).decrypt(public_syndrome, private_key)
        secret_key = libnacl.crypto_hash(str(error))[:libnacl.crypto_secretbox_KEYBYTES]
        return DEM.decrypt(ciphertext, secret_key)


class Test:
    def __init__(self, block_count, block_size, row_weight, error_weight, thresholds):
        self.N = Niederreiter(block_count, block_size, row_weight, error_weight, thresholds)
        self.KD = KEMDEM(block_count, block_size, row_weight, error_weight, thresholds)
        
    def self_test_pack_unpack(self):
        pass
    
    def self_test_correctness(self, keys, messages):
        error = False
        dfs = 0
        for k in xrange(keys):
            if args.verbose >= 1:
                print("Testing key {}".format(k)) 
            pub, priv = self.N.keygen()
            assert Util.last_block(pub) == 1, "Error in keygen"
            for m in xrange(messages):
                if args.verbose >= 1:
                    print "Testing KEM {}.{}:".format(k,m),
                    sys.stdout.flush()
                e = self.N.gen_error()
                ct = self.N.encrypt(e, pub)
                try:
                    pt = self.N.decrypt(ct, priv)
                except DecodingFailure as df:
                    print(df)
                    # TODO: print failure values
                    dfs += 1
                else:
                    if e != pt:
                        print("Decoded to wrong error")
                        # TODO: print erronous values
                        error = True
                    elif args.verbose >= 1:
                        print("Test succeeded")

                if args.verbose >= 1:
                    print "Testing KEM/DEM: {}".format(m), ; sys.stdout.flush()
                msg = libnacl.randombytes(int(64))
                ct = self.KD.encrypt(msg, pub)
                try:
                    pt = self.KD.decrypt(ct, priv)
                except DecodingFailure as df:
                    print(df)
                    # TODO: print failure values
                    dfs += 1
                except ValueError as ve:
                    print(ve)
                    # TODO: print erronous values
                    error = True
                else:
                    if msg != pt:
                        print("Decrypted to wrong value")
                        # TODO: print erronous values
                        error = True
                    elif args.verbose >= 1:
                        print("Test succeeded")
                        
        if args.verbose >= 1:
            if dfs > 0:
                print("{} decoding failures".format(dfs))
            if not error:
                print("All tests passed")
        return dfs > 0 or error
    
    def test_KEM_encrypt(self, e, pub, expected):
        pass
    
    def test_KEM_decrypt(self, input, expected):
        pass


def main():
    global args
    err_code = 0

    ap = argparse.ArgumentParser()
    ap.add_argument("-s", "--selftest",
                    action="store_true",
                    help="Run internal consistency tests")
    ap.add_argument("-k", "--keys",
                    type=int, default=5,
                    help="Number of random keys to be tested (default: 5)")
    ap.add_argument("-m", "--messages",
                    type=int, default=10,
                    help="Number of random messages to be tested (default: 5)")
    ap.add_argument("-v", "--verbose",
                    action="count", default=0,
                    help="increase output verbosity")
    args = ap.parse_args()
    if args.selftest:
        test = Test(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        err_code = test.self_test_correctness(args.keys, args.messages)
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
