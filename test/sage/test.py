from sage.all import *

import ctypes
import pack
import qcmdpc

gKD = qcmdpc.KEMDEM(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])

def foo():
    

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
