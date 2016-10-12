from sage.all import *
from ctypes import cdll

from config import Config
import pack
import qcmdpc

#gKD = qcmdpc.KEMDEM(Config);

def foo():
    libc = cdll.LoadLibrary('../../libtest.so')
    c_inv = c_ulonglong * POLY_BITS
    c_priv = (c_ushort * POLY_WEIGHT) * POLY_COUNT
    libc.kem_rand_par_ch(c_inv, c_priv)
    print(c_inv, c_priv)

# TODO: C.psp_to_dense == sage.poly.to_dense

# TODO: C.transpose . C.transpose == id

# TODO: C.mul == python.poly.mul

# TODO: C.inv == python.poly.inv

# TODO: C.xgcd == python.poly.xgcd

# TODO: C.err

# TODO: C.kem with userdata == python.kem

# TODO: C.encrypt with userdata == python.kemdem

# TODO: C.kex with userdata == python.kex

# TODO: poly_hw

# TODO: poly_compare

if __name__ == '__main__':
    foo()
