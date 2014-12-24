#!/usr/bin/env python
#coding: UTF-8

import argparse
import sys

import bigNumber.bigNumber as bigNumber
import primes


def GCDEx(a, b):        
    if b == 0:
        return a, 1, 0
    if a == 0:
        return b, 1, 0
    
    x1 = bigNumber.bigNumber(0)
    x2 = bigNumber.bigNumber(1)
    y1 = bigNumber.bigNumber(1)
    y2 = bigNumber.bigNumber(0)
    
    while b != 0:
        q = a / b
        r = a % b
        a = b
        b = r
        
        xx = x2 - x1 * q
        yy = y2 - y1 * q
        x2 = x1
        x1 = xx
        y2 = y1
        y1 = yy
    x = x2
    y = y2

    return a, x, y

    
def LinCon (a, b, m):
    """Решение линейного сравнение ax = b mod m
    """
    d, t1, t2 = GCDEx(a, m)    
    if b % d != 0:
        return False, 0
    
    if (a == 0 and b % m == 0) or (b == 0 and a % m == 0):
        return True, -1
    
    # находим одно из решений, используя расширенный алгоритм Евклида
    a1 = a / d
    b1 = b / d
    m1 = m / d
    d1, x, y = GCDEx(a1, m1)    
    res0 = ( b1 * x ) % m
    while res0 < 0:
        res0 += m

    # теперь находим оставшиеся d-1 решения
    resAll = []
    resAll.append(res0)
    d = d - 1
    while d > 0:
        resAll.append( (resAll[-1] + m1) % m )
        if resAll[-1] < 0:
            resAll[-1] += m
        d -= 1
    
    return True, resAll


def generate_keys(bitlen, name):
    p = primes.GeneratePrime(bitlen)
    q = primes.GeneratePrime(bitlen)
    while p == q:
        q = primes.GeneratePrime(bitlen)
        
    n = p * q
    Fi = (p-1) * (q-1)
    e = bigNumber.bigNumber(65537)
    
    Solved, d = LinCon(e, bigNumber.bigNumber(1), Fi)
    if not Solved or d == -1:
        raise ValueError('Something went worng...')
    d = d[0]
    
    pub_key = "\n".join(map(str, (e, n)))
    priv_key = "\n".join(map(str, (d, n)))

    with open(name + '.pub', 'w') as pub_file:
        pub_file.write(pub_key)

    with open(name + '.priv', 'w') as priv_file:
        priv_file.write(priv_key)
        

def decrypt(d, n, c):
    m = bigNumber.Pow(c, d, n)
    return m


def encrypt(e, n, m):
    if m >= n:
        raise ValueError('Message is too large!')
    
    c = bigNumber.Pow(m, e, n)
    return c


def get_args():
    parser = argparse.ArgumentParser(
        prog="rsa", description="RSA crypter/decrypter")
    parser.add_argument('input', help="file name")
    parser.add_argument('output', help="file name")
    parser.add_argument('keychain', help="key chain name")
    parser.add_argument('mode', choices=['e', 'd'],
                        help="e - encrypt, d - decrypt")
    return parser.parse_args()      
        
if __name__ == "__main__":
    args = get_args()

    if args.mode == 'e':
        with open(args.keychain + '.pub') as key_file:
            (e, n) = key_file.read().split("\n")
            
        e = bigNumber.bigNumber(e)
        n = bigNumber.bigNumber(n)
        m = bigNumber.bigNumber()
        m.GetNumberFromBinFile(args.input)
        c = encrypt(e, n, m)

        result = str(c)
        with open(args.output, 'w') as out_file:
            out_file.write(result)

    else:
        with open(args.keychain + '.priv') as key_file:
            (d, n) = key_file.read().split("\n")
            
        with open(args.input) as in_file:
            c = in_file.read()

        d = bigNumber.bigNumber(d)
        n = bigNumber.bigNumber(n)
        c = bigNumber.bigNumber(c)

        m = decrypt(d, n, c,)
        m.SaveNumberInBinFile(args.output)
