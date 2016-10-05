from sage.all import *

import struct
import binascii

# TODO: assumes type H (16-bit) for indices
bytes_per_index = 2

class ByteOrder:
    Native = '='
    LittleEndian = '<'
    BigEndian = '>'
    Network = '!'

def vector_to_cyclic_matrix(vec):
    n = len(vec)
    if vec.is_sparse():
        matrix_dict = dict(((x, (x+y)%n), True) for y in vec.dict() for x in xrange(n))
        return matrix(GF(2), n, n, matrix_dict)
    vec_list = vec.list()
    matrix_lists = [vec_list[-i:] + vec_list[:-i] for i in xrange(n)]
    return matrix(GF(2), n, n, matrix_lists)

def pack_binary_vector(vec, byte_order=ByteOrder.Native):
    if vec.is_sparse():
        return struct.pack(byte_order + 'H' * vec.hamming_weight(), *(vec.dict().keys()))
    size = (vec.length() + 7) / 8
    byte_array = [0] * size
    for i, bit in enumerate(vec):
        byte_array[i/8] |= int(bit) << (7-i%8)
    return struct.pack(byte_order + 'B' * size, *byte_array)
    
def unpack_binary_vector(string, bit_length, is_sparse=False, byte_order=ByteOrder.Native):
    if is_sparse:
        weight = len(string) / bytes_per_index
        indices = struct.unpack(byte_order + 'H' * weight, string)
        return vector(GF(2), bit_length, dict((index, True) for index in indices))
    vec = (GF(2)**bit_length)(0)
    byte_array = struct.unpack(byte_order + 'B' * len(string), string)
    for i in xrange(bit_length):
        vec[i] = (byte_array[i/8] >> (7-i%8)) & 1
    return vec

def pack_public_key(pubkey, byte_order=ByteOrder.Native):
    [], blocks = pubkey.subdivisions()
    packed = ''
    for b in xrange(len(blocks)):
        packed += pack_binary_vector(pubkey.subdivision(0, b)[0], byte_order)
    return packed

def unpack_public_key(packed, block_count, block_bits, byte_order=ByteOrder.Native):
    block_bytes = (block_bits + 7) / 8
    blocks = [None] * block_count
    for b in xrange(block_count - 1):
        substring = packed[b * block_bytes : (b+1) * block_bytes]
        blocks[b] = vector_to_cyclic_matrix(unpack_binary_vector(substring, block_bits, False, byte_order))
    blocks[-1] = matrix.identity(block_bits)
    return block_matrix(blocks, ncols=block_count)

def pack_private_key(privkey, byte_order=ByteOrder.Native):
    [], blocks = privkey.subdivisions()
    packed = ''
    for b in xrange(len(blocks)+1):
        packed += pack_binary_vector(privkey.subdivision(0, b)[0])
    return packed

def unpack_private_key(packed, block_count, block_weight, block_bits, byte_order=ByteOrder.Native):
    block_bytes = block_weight * index_bytes
    blocks = [None] * block_count
    for b in xrange(block_count):
        substring = packed[b * block_bytes : (b+1) * block_bytes]
        blocks[b] = vector_to_cyclic_matrix(unpack_binary_vector(substring, block_bits, True, byte_order))
    return block_matrix(blocks, ncols=block_count)

def pack_error(error, block_count=None, byte_order=ByteOrder.Native):
    if error.is_sparse():
        return pack_binary_vector(error, byte_order)
    packed = ''
    block_bits = len(error) / block_count
    for b in xrange(block_count):
        error_slice = error[b * block_bits : (b+1) * block_bits]
        packed += pack_binary_vector(error_slice, byte_order)
    return packed

def unpack_error(packed, block_count, block_bits, is_sparse=False, byte_order=ByteOrder.Native):
    if is_sparse:
        return unpack_binary_vector(packed, block_count * block_bits, True, byte_order)
    block_bytes = (block_bits + 7) / 8
    lists = [None] * block_count
    for b in xrange(block_count):
        substring = packed[b * block_bytes : (b+1) * block_bytes]
        lists[b] = unpack_binary_vector(substring, block_bits, False, byte_order)
    return vector(GF(2), [bit for vec in lists for bit in vec])

if __name__ == '__main__':
    v = vector(GF(2), [1,1,0,0, 1,0,1,0, 1,1,1,1, 1,1,1,0, \
                       1,0,1,1, 1,0,1,0, 0,0,0,0, 1,0,0,0, \
                       1])
    w = unpack_binary_vector(pack_binary_vector(v), 33)
    assert v == w, 'Cannot pack/unpack (defaults)'
    x = unpack_binary_vector(pack_binary_vector(v.sparse_vector()), 33, True)
    assert v == x, 'Cannot pack/unpack (sparse)'
    y = unpack_binary_vector(pack_binary_vector(v, byte_order=ByteOrder.Network), \
                             33, byte_order=ByteOrder.Network)
    assert v == y, 'Cannot pack/unpack (network byte order)'

    s = binascii.unhexlify('deadbeef')
    t = pack_binary_vector(unpack_binary_vector(s, 32))
    assert s == t, 'Cannot unpack/pack (defaults)'
