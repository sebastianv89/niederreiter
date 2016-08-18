def degree(f):
    deg = 0
    while f:
        f >>= 1
        deg += 1
    return deg

def poly_mul(a, b):
    c = 0
    for shift in range(degree(b)):
        if b & (1 << shift):
            c ^= a << shift
    return c
    
def poly_div_mod(num, denom):
    div = 0
    shift = degree(num) - degree(denom)
    while (shift >= 0):
        num ^= denom << shift
        div |= 1 << shift
        shift = degree(num) - degree(denom)
    return div, num

def poly_mod(num, mod):
    return poly_div_mod(num, mod)[1]

def poly_xgcd(f, g):
    x_prev, x = 0, 1
    y_prev, y = 1, 0
    r_prev, r = f, g
    
    while r > 0:
        div, mod = poly_div_mod(r_prev, r)
        r_prev, r = r, mod
        x_prev, x = x, x_prev ^ poly_mul(div, x)
        y_prev, y = y, y_prev ^ poly_mul(div, y)
    
    return r_prev, x_prev, y_prev

def poly_inv(f, mod):
    gcd, inv, _ = poly_xgcd(mod, f)
    if gcd > 1:
        return 0
    return inv


def bin_xgcd(a, b):
    assert b & 1, "b is even"
    assert b >= a, "a is larger than b"
    
    A, B = a, b
    aa, ab = 1, 0
    ba, bb = 0, 1

    assert a == aa*A + ab*B, "initial a invariant failed"
    assert b == ba*A + bb*B, "initial b invariant failed"

    while a != b:
        if not (a & 1):
            a >>= 1
            if aa & 1 or ab & 1:
                aa, ab = (aa + B) >> 1, (ab - A) >> 1
            else:
                aa, ab = aa >> 1, ab >> 1
        elif not (b & 1):
            b >>= 1
            if ba & 1 or bb & 1:
                ba, bb = (ba + B) >> 1, (bb - A) >> 1
            else:
                ba, bb = ba >> 1, bb >> 1
        elif a > b:
            a = (a - b) >> 1
            aa, ab = aa - ba, ab - bb
            if aa & 1 or ab & 1:
                aa, ab = (aa + B) >> 1, (ab - A) >> 1
            else:
                aa, ab = aa >> 1, ab >> 1
        else:
            b = (b - a) >> 1
            ba, bb = ba - aa, bb - ab
            if ba & 1 or bb & 1:
                ba, bb = (ba + B) >> 1, (bb - A) >> 1
            else:
                ba, bb = ba >> 1, bb >> 1

        assert a == aa*A + ab*B, "a invariant failed"
        assert b == ba*A + bb*B, "b invariant failed"
        
    return a, ba, bb

def bin_xgcd_alt(a, b):
    # Same as bin_xgcd(), but don't bother computing bb (don't need it for modular inverse)

    assert b & 1, "b is even"
    assert b >= a, "a is larger than b"
    
    B = b
    aa = 1
    ba = 0

    while a != b:
        if not (a & 1):
            a >>= 1
            if aa & 1:
                aa = (aa + B) >> 1
            else:
                aa >>= 1
        elif not (b & 1):
            b >>= 1
            if ba & 1:
                ba = (ba + B) >> 1
            else:
                ba >>= 1
        elif a > b:
            a = (a - b) >> 1
            aa = aa - ba
            if aa & 1:
                aa = (aa + B) >> 1
            else:
                aa >>= 1
        else:
            b = (b - a) >> 1
            ba = ba - aa
            if ba & 1:
                ba = (ba + B) >> 1
            else:
                ba = ba >> 1

    return a, ba

def poly_bin_xgcd(f, g):
    assert g & 1, "g is even"
    assert f <= g, "f is larger than g"

    G = g
    a, b = 1, 0
    
    while f != g:
        print "{:b}, {:b}, {:b}, {:b}".format(f, g, a, b)
        if not (f & 1):
            f >>= 1
            if a & 1:
                a = (a ^ G) >> 1
            else:
                a >>= 1
        elif not (g & 1):
            g >>= 1
            if b & 1:
                b = (b ^ G) >> 1
            else:
                b = b >> 1
        elif f > g:
            f = (f ^ g) >> 1
            a = a ^ b 
            if a & 1:
                a = (a ^ G) >> 1
            else:
                a = a >> 1
        else:
            g = (g ^ f) >> 1
            b = b ^ a
            if b & 1:
                b = (b ^ G) >> 1
            else:
                b = b >> 1

    return f, b

def poly_bin_xgcd_2(f, g):
    assert g & 1, "g is even"
    assert f <= g, "f is larger than g"

    G = g
    a, b = 1, 0
    
    while f != g:
        print "{:b}, {:b}, {:b}, {:b}".format(f, g, a, b)
        if not (f & 1):
            f >>= 1
            if a & 1:
                a = (a ^ G) >> 1
            else:
                a >>= 1
        else:
            f = (f ^ g) >> 1
            a = a ^ b
            if a & 1:
                a = (a ^ G) >> 1
            else:
                a = a >> 1

    return f, b


def poly_modinv(f, g):
    gcd, inv = poly_bin_xgcd_alt(f, g)
    if gcd > 1:
        return 0
    return inv
