# THIS IS NOT A SECURE IMPLEMENTATION!!! USE THIS ONLY FOR
# DEBUGGING/TESTING!!!
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
# In particular, the returned exception leaks if decryption failed
# because of a decoding failure or because of a MAC failure.  All
# operations are in non-constant time, leading to side-channel
# vulnerabilities.  Besides insecure, this implementation is also
# horribly slow.
# 
# Author: Sebastian R. Verschoor (srverschoor@uwaterloo.ca)
# License: TODO
# 

from sage.all import *

import libnacl

args = argparse.Namespace(verbose=1)

class Util:
    @staticmethod
    def gen_vec(length, weight):
        indices = Permutations(length, weight).random_element()
        return vector(GF(2), length, dict((index,True) for index in indices))

    @staticmethod
    def last_block(matrix):
        [], blocks = matrix.subdivisions()
        return matrix.subdivision(0, len(blocks))
        
    @staticmethod
    def print_vec(vec):
        if vec.is_sparse():
            print(sorted(vec.dict().keys()))
        else:
            print(base64.b64encode(Util.pack_vec(vec)))
        print("hamming weight: {}".format(vec.hamming_weight()))
    
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
        if args.verbose >= 3:
            print("### Private syndrome")
            Util.print_vec(private_syndrome.dense_vector())
        return self._decode(private_syndrome, private_key)

    def _private_syndrome(self, public_syndrome, parity_check):
        last_block = Util.last_block(parity_check)
        return last_block * public_syndrome
    
    def _decode(self, private_syndrome, private_key):
        error_candidate = (GF(2)^self.size)(0)
        for i, threshold in enumerate(self.thresholds):
            err_found = 0
            if args.verbose >= 4:
                print("#### Decoder round {} (threshold {})".format(i, threshold))
            syndrome_update = (GF(2)^self.block_size)(0)
            for column_index, column in enumerate(private_key.columns()):
                error_count = column.pairwise_product(private_syndrome).hamming_weight()
                if error_count >= threshold:
                    error_candidate[column_index] += 1
                    syndrome_update += column
                    err_found += 1
            private_syndrome += syndrome_update
            if args.verbose >= 5:
                print("error candidate:"); Util.print_vec(error_candidate.sparse_vector())
                print("syndrome update:"); Util.print_vec(syndrome_update)
                print("syndrome:"); Util.print_vec(private_syndrome.dense_vector())
            print("It {}, {} errors found".format(i, err_found))
        if private_syndrome == 0:
            return error_candidate
        raise DecodingFailure("Could not decode syndrome")


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
        if args.verbose >= 3:
            print("#### Packed error"); print(base64.b64encode(error_bytes))
        secret_key = libnacl.crypto_hash(error_bytes)[:libnacl.crypto_secretbox_KEYBYTES]
        if args.verbose >= 3:
            print("#### Secret key"); print(base64.b64encode(secret_key))
        public_syndrome = super(KEMDEM, self).encrypt(error, public_key)
        ciphertext = DEM.encrypt(plaintext, secret_key)
        return public_syndrome, ciphertext
        
    def decrypt(self, (public_syndrome, ciphertext), private_key):
        error = super(KEMDEM, self).decrypt(public_syndrome, private_key)
        if args.verbose >= 3:
            print("#### Decrypted error"); Util.print_vec(error)
        error_bytes = Util.pack_error(error, self.block_count, self.block_size)
        if args.verbose >= 3:
            print("#### Decrypted packed error"); print(base64.b64encode(error_bytes))
        secret_key = libnacl.crypto_hash(error_bytes)[:libnacl.crypto_secretbox_KEYBYTES]
        if args.verbose >= 3:
            print("#### Derived secret key"); print(base64.b64encode(secret_key))
        return DEM.decrypt(ciphertext, secret_key)

# class Test:
#     def __init__(self, block_count, block_size, row_weight, error_weight, thresholds):
#         self.N = Niederreiter(block_count, block_size, row_weight, error_weight, thresholds)
#         self.KD = KEMDEM(block_count, block_size, row_weight, error_weight, thresholds)
#         if args.verbose >= 1:
#             print("Test initialized with parameters:")
#             print("block_count: {}".format(block_count))
#             print("block_size: {}".format(block_size))
#             print("row_weight: {}".format(row_weight))
#             print("error_weight: {}".format(error_weight))
#             print("thresholds: {}".format(thresholds))
#         
#     def self_test_pack_unpack(self, test_rounds=5):
#         block_count = self.N.block_count
#         block_size = self.N.block_size
#         row_weight = self.N.row_weight
#         error_weight = self.N.error_weight
#         for r in xrange(test_rounds):
#             if verbose >= 2:
#                 print("Test pack/unpack, round {}".format(r));
#             vec = Util.gen_vec(block_size, row_weight)
#             copy = Util.unpack_vec(Util.pack_vec(vec), block_size, True)
#             assert copy == vec, "failed to pack/unpack sparse vector"
#             vec = vec.dense_vector()
#             copy = Util.unpack_vec(Util.pack_vec(vec), block_size, False)
#             assert copy == vec, "failed to pack/unpack dense vector"
#             pub, priv = self.N.keygen()
#             copy = Util.unpack_pub_key(Util.pack_pub_key(pub),
#                                        block_count, block_size)
#             assert copy == pub, "failed to pack/unpack public key"
#             copy = Util.unpack_priv_key(Util.pack_priv_key(priv),
#                                         block_count, row_weight, block_size)
#             assert copy == priv, "failed to pack/unpack private key"
#             err = self.N.gen_error()
#             copy = Util.unpack_error_sparse(Util.pack_error_sparse(err),
#                                             block_count, block_size)
#             assert copy == err, "failed to pack/unpack (sparse) error"
#         return True
    
#     def self_test_correctness(self, (keys, messages, data)):
#         test_failed = False
#         decoding_failures = 0
#         for k in xrange(keys):
#             if args.verbose >= 1:
#                 print("Testing key {}".format(k)) 
#             if data is None:
#                 pub, priv = self.N.keygen()
#             else:
#                 pub, priv = self.N.keygen(data.key[k])
#             assert Util.last_block(pub) == 1, "keygen failed (last block != 1)"
#             if args.verbose >= 2:
#                 print("priv:"); Util.print_vec(priv[0])
#                 print("pub:"); Util.print_vec(pub[0])
#             for m in xrange(messages):
#                 if args.verbose >= 1:
#                     print("Testing KEM {}.{}:".format(k,m))
#                 if data is None:
#                     e = self.N.gen_error()
#                 else:
#                     e = data.error[m]
#                 if args.verbose >= 2:
#                     print("error:"); Util.print_vec(e)
#                 ct = self.N.encrypt(e, pub)
#                 if args.verbose >= 2:
#                     print("ciphertext:"); Util.print_vec(ct)
#                 try:
#                     pt = self.N.decrypt(ct, priv)
#                     if args.verbose >= 2:
#                         print("plaintext:"); Util.print_vec(pt)
#                 except DecodingFailure as df:
#                     if args.verbose >= 1:
#                         print(df)
#                     if args.verbose == 1:
#                         print("priv_key:"); Util.print_vec(priv[0])
#                         print("error:"); Util.print_vec(e)
#                     decoding_failures += 1
#                 else:
#                     if e != pt:
#                         if args.verbose >= 1:
#                             print("Decoded to wrong error")
#                         if args.verbose == 1:
#                             print("priv_key:"); Util.print_vec(priv[0])
#                             print("error:"); Util.print_vec(e)
#                             print("pt:"); Util.print_vec(pt)
#                         test_failed = True
#                     elif args.verbose >= 1:
#                         print("Test succeeded")
# 
#                 if args.verbose >= 1:
#                     print("Testing KEM/DEM: {}".format(m))
#                 if data is None:
#                     msg = libnacl.randombytes(int(64))
#                 else:
#                     msg = data.msg[m]
#                 if args.verbose >= 2:
#                     print("## Message"); print(base64.b64encode(msg))
#                 ct = self.KD.encrypt(msg, pub)
#                 if args.verbose >= 2:
#                     (ct_syn, ct_msg) = ct
#                     print("## Ciphertext public syndrome"); Util.print_vec(ct_syn)
#                     print("## Ciphertext"); print(base64.b64encode(ct_msg))
#                 try:
#                     pt = self.KD.decrypt(ct, priv)
#                     if args.verbose >= 2:
#                         print("## Plaintext"); print(base64.b64encode(pt))
#                 except DecodingFailure as df:
#                     if args.verbose >= 1:
#                         print(df)
#                     if args.verbose == 1:
#                         (ct_syn, ct_msg) = ct
#                         print("## Message"); print(base64.b64encode(msg))
#                         print("## Ciphertext public syndrome"); Util.print_vec(ct_syn)
#                         print("## Ciphertext"); print(base64.b64encode(ct_msg))
#                     decoding_failures += 1
#                 except ValueError as ve:
#                     if args.verbose >= 1:
#                         print(ve)
#                     if args.verbose == 1:
#                         (ct_syn, ct_msg) = ct
#                         print("## Message"); print(base64.b64encode(msg))
#                         print("## Ciphertext public syndrome"); Util.print_vec(ct_syn)
#                         print("## Ciphertext"); print(base64.b64encode(ct_msg))
#                     test_failed = True
#                 else:
#                     if msg != pt:
#                         if args.verbose >= 1:
#                             print("Decrypted to wrong value")
#                         if args.verbose == 1:
#                             print("## Message"); print(base64.b64encode(msg))
#                             print("## Ciphertext"); print(base64.b64encode(ct))
#                             print("## Plaintext"); print(base64.b64encode(pt))
#                         test_failed = True
#                     elif args.verbose >= 1:
#                         print("Test succeeded")
# 
#         if args.verbose >= 1:
#             if decoding_failures > 0:
#                 print("{} decoding failures".format(decoding_failures))
#             elif not test_failed:
#                 print("All tests passed")
# 
#         return not (decoding_failures > 0 or test_failed)

# class Data():
#     def _repr_(self):
#         return 'Data({' + repr(parameters) + ',' + repr(keys) + '})'
#     class Parameters:
#         def _repr_(self):
#             return 'Parameters(' 
    
# def gen_test_vectors(keys, messages):
#     block_count = 2
#     block_size = 4801
#     row_weight = 45
#     error_weight = 84
#     thresholds = [29, 27, 25, 24, 23, 23]
#     msg_bytes = int(16)
#     KD = KEMDEM(block_count, block_size, row_weight, error_weight, thresholds)
#     
#     data = Data()
#     data.parameters = Data.Parameters()
#     data.parameters.block_count = block_count
#     data.parameters.block_size = block_size
#     data.parameters.row_weight = row_weight
#     data.parameters.error_weight = error_weight
#     data.parameters.thresholds = thresholds
#     data.keys = [type('', (), {})] * keys
#     for k in range(keys):
#         key = data.keys[k]
#         key.pub, key.priv = KD.keygen()
#         key.msg = [type('', (), {})] * messages
#         save(key.pub, 'pub')
#         for m in range(messages):
#             msg = key.msg[m]
#             msg.err = KD.gen_error()
#             msg.pt = libnacl.randombytes(msg_bytes)
#             msg.pub_sy, msg.ct = KD.encrypt(msg.pt, key.pub, msg.err)
#     print(vars(data))
#     print(data)
#     f = open('data.pkl','w')
#     pickle.dump(data, f)
#     f.close()
#     save(data, 'file.txt')
#     
#     return
#     
#     print("# Parameters")
#     print("block_count: {}".format(block_count))
#     print("block_size: {}".format(block_size))
#     print("row_weight: {}".format(row_weight))
#     print("error_weight: {}".format(error_weight))
#     print("thresholds: {}".format(thresholds))
#     print("")
#     for k in xrange(keys):
#         print("## Key generation")
#         pub, priv = KD.keygen()
#         print("### Private key")
#         for b in xrange(block_count):
#             print("block {}:".format(b))
#             Util.print_vec(priv.subdivision(0,b)[0])
#         print("### Public key")
#         for b in xrange(block_count - 1):
#             print("block {}".format(b))
#             Util.print_vec(pub.subdivision(0, b)[0])
#         print("")
#         for m in xrange(messages):
#             print("## Encryption")
#             err = KD.gen_error()
#             print("### Error")
#             Util.print_vec(err)
#             msg = libnacl.randombytes(msg_bytes)
#             print("### Message")
#             print(base64.b64encode(msg))
#             pub_syn, ct = KD.encrypt(msg, pub, err)
#             print("### Public syndrome")
#             Util.print_vec(pub_syn)
#             print("### Ciphertext")
#             print(base64.b64encode(ct))
#             print("")
#             print("## Decryption")
#             pt = KD.decrypt((pub_syn, ct), priv)
#             print("")

if __name__ == "__main__":
    # set globals as helper variables in the repl
    global gN
    global gKD
    global gT
    gN = Niederreiter(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
    gKD = KEMDEM(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
    gT = Test(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
    # gN = Niederreiter(2, 71, 3, 4, [3, 2, 2, 1, 1, 1])
    # gKD = KEMDEM(2, 71, 3, 4, [3, 2, 2, 1, 1, 1])
    # gT = Test(2, 71, 3, 4, [3, 2, 2, 1, 1, 1])

    main()
