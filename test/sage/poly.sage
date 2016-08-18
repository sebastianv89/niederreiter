# Sage can work with polynomials directly.  Let's use that to test
# (parts of) the C implementation.

r = 4801

PR.<x> = PolynomialRing(GF(2))
P.<y> = PR.quotient(x^r-1)

p = P.random_element()
try:
    p_inv = p^-1
except ZeroDivisionError as zde:
    print zde
else:
    print p * p_inv
