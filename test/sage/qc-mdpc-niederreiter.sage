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
# 

import argparse
import libnacl
import sys


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
        [], blocks = matrix.subdivisions();
        return matrix.subdivision(0, len(blocks) - 1);


class DecodeFailure(Exception):
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
        private_key = self._paritycheck(first_rows_ids)
        public_key = self._systematic(private_key)
        return public_key, private_key

    def _paritycheck(self, first_rows_ids=None):
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

    def _systematic(self, paritycheck):
        last_block = Util.last_block(paritycheck).dense_matrix()
        last_block_inv = last_block.inverse()
        return last_block_inv * paritycheck

    def gen_error(self, indices=None):
        return Util.gen_vec(self.size, self.error_weight, indices)

    def encrypt(self, error, systematic):
        assert len(error) == self.size, "len(error) is incorrect"
        assert error.hamming_weight() == self.error_weight, "weight(error) is incorrect"
        return systematic * error
    
    def decrypt(self, public_syndrome, private_key):
        private_syndrome = self._private_syndrome(public_syndrome, private_key)
        return self._decode(private_syndrome, private_key)

    def _private_syndrome(self, public_syndrome, paritycheck):
        last_block = Util.last_block(paritycheck)
        return last_block * public_syndrome
    
    def _decode(self, private_syndrome, private_key):
        error_candidate = (GF(2)^self.size)(0)
        private_key = private_key.dense_matrix() # speed up computation
        for threshold in self.thresholds:
            for column_index, column in enumerate(private_key.columns()):
                error_count = column.pairwise_product(private_syndrome).hamming_weight()
                if error_count >= threshold:
                    error_candidate[column_index] += 1
                    private_syndrome += column
        if private_syndrome == 0:
            return error_candidate
        raise DecodeFailure("Could not decode syndrome")


class DEM:
    @staticmethod
    def encrypt(plaintext, secret_key):
        # TODO: nonce==0 => IND-CCA?
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
    def __init__(block_count, block_size, row_weight, error_weight, thresholds):
        self.N = Niederreiter(block_count, block_size, row_weight, error_weight, thresholds)
        self.KD = KEMDEM(block_count, block_size, row_weight, error_weight, thresholds)
    
    def test_correctness(self):
        pass
    
    def test_KEM_encrypt(self, e, pub, expected):
        pass
    
    def test_KEM_decrypt(self, input, expected):
        pass

def main():
    args = parseargs();
    print "TODO: testing"
    
if __name__ == "__main__":
    main()
