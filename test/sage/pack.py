from sage.all import *

import struct

# TODO: assumes type H (16-bit) for indices
bytes_per_index = 2

def sparse_list_to_vector(indices, bit_length):
    return vector(GF(2), bit_length, dict((index, True) for index in indices))

def vector_to_cyclic_matrix(vec):
    """vec is the first column of the cyclic matrix"""
    n = len(vec)
    if vec.is_sparse():
        matrix_dict = dict((((x+y)%n, y), True) for x in vec.dict() for y in xrange(n))
        return matrix(GF(2), n, n, matrix_dict)
    vec_list = vec.list()
    matrix_lists = [vec_list[-i:] + vec_list[:-i] for i in xrange(n)]
    return matrix(GF(2), n, n, matrix_lists)

def pack_binary_vector(vec):
    if vec.is_sparse():
        return struct.pack('!' + 'H' * vec.hamming_weight(), *(vec.dict().keys()))
    size = (vec.length() + 7) / 8
    byte_array = [0] * size
    for i, bit in enumerate(vec):
        byte_array[i/8] |= int(bit) << (i%8)
    return struct.pack('!' + 'B' * size, *byte_array)

def unpack_binary_vector(string, bit_length, is_sparse=False):
    if is_sparse:
        weight = len(string) / bytes_per_index
        indices = struct.unpack('!' + 'H' * weight, string)
        return sparse_list_to_vector(indices, bit_length)
    vec = (GF(2)**bit_length)(0)
    byte_array = struct.unpack('!' + 'B' * len(string), string)
    for i in xrange(bit_length):
        vec[i] = byte_array[i/8] >> (i%8) 
    return vec

def pack_public_key(pubkey):
    [], blocks = pubkey.subdivisions()
    packed = ''
    for b in xrange(len(blocks)):
        packed += pack_binary_vector(pubkey.subdivision(0, b).column(0))
    return packed

def unpack_public_key(packed, block_count, block_bits):
    block_bytes = (block_bits + 7) / 8
    blocks = [None] * block_count
    for b in xrange(block_count - 1):
        substring = packed[b * block_bytes : (b+1) * block_bytes]
        blocks[b] = vector_to_cyclic_matrix(unpack_binary_vector(substring, block_bits, False))
    blocks[-1] = matrix.identity(block_bits)
    return block_matrix(blocks, ncols=block_count)

def pack_private_key(privkey):
    [], blocks = privkey.subdivisions()
    packed = ''
    for b in xrange(len(blocks)+1):
        packed += pack_binary_vector(privkey.subdivision(0, b).column(0))
    return packed

def unpack_private_key(packed, block_count, block_weight, block_bits):
    block_bytes = block_weight * index_bytes
    blocks = [None] * block_count
    for b in xrange(block_count):
        substring = packed[b * block_bytes : (b+1) * block_bytes]
        blocks[b] = vector_to_cyclic_matrix(unpack_binary_vector(substring, block_bits, True))
    return block_matrix(blocks, ncols=block_count)

def pack_error(error, block_count=None):
    if error.is_sparse():
        return pack_binary_vector(error)
    packed = ''
    block_bits = len(error) / block_count
    for b in xrange(block_count):
        error_slice = error[b * block_bits : (b+1) * block_bits]
        packed += pack_binary_vector(error_slice)
    return packed

def unpack_error(packed, block_count, block_bits, is_sparse=False):
    if is_sparse:
        return unpack_binary_vector(packed, block_count * block_bits, True)
    block_bytes = (block_bits + 7) / 8
    lists = [None] * block_count
    for b in xrange(block_count):
        substring = packed[b * block_bytes : (b+1) * block_bytes]
        lists[b] = unpack_binary_vector(substring, block_bits, False)
    return vector(GF(2), [bit for vec in lists for bit in vec])
