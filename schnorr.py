#!/usr/bin/env python
#coding: UTF-8

import argparse
import sys

import bigNumber.bigNumber as bigNumber
import primes


def generate_keys(bitlen, name):
    p_const = "48731"
    q_const = "443"
    g_const = "11444"
    p = bigNumber.bigNumber(p_const)
    q = bigNumber.bigNumber(q_const)
    g = bigNumber.bigNumber(g_const)
    
    w = bigNumber.GenerateRandomMax(q)
    y = bigNumber.Pow(g, q - w, p)
    
    pub_key = "\n".join(map(str, (p, q, g, y)))
    priv_key = str(w)

    with open(name + '.pub', 'w') as pub_file:
        pub_file.write(pub_key)

    with open(name + '.priv', 'w') as priv_file:
        priv_file.write(priv_key)
        

def get_args():
    parser = argparse.ArgumentParser(
        prog="schnorr", description="scnorr keychain checker")
    parser.add_argument('keychain', help="key chain name")
    return parser.parse_args()      
        

if __name__ == "__main__":
    args = get_args()
    
    with open(args.keychain + '.pub') as key_file:
        (p, q, g, y) = key_file.read().split("\n")
    p = bigNumber.bigNumber(p)
    q = bigNumber.bigNumber(q)
    g = bigNumber.bigNumber(g)
    y = bigNumber.bigNumber(y)
        
    with open(args.keychain + '.priv') as key_file:
        w = key_file.read().split("\n")
    w = bigNumber.bigNumber(w[0])

    # Алгоритм работы протокола
    # Предварительная обработка. Алиса выбирает случайное число r, меньшее q, 
    #   и вычисляет x = g^r mod p. 
    r = bigNumber.GenerateRandomMax(q)
    x = bigNumber.Pow(g, r, p)
    
    # Инициирование. Алиса посылает x Бобу.
    # Боб выбирает случайное число e из диапазона от 0 до 2^t-1 и отправляет его Алисе.
    pow2t = bigNumber.GenerateRandomLen(72)
    e = bigNumber.GenerateRandomMax(pow2t)
    
    # Алиса вычисляет s=r+we mod q и посылает s Бобу.
    s = (r + w * e) % q

    # Подтверждение. Боб проверяет что x=g^s * y^e mod p
    from_alice = (bigNumber.Pow(g, s, p) * bigNumber.Pow(y, e, p)) % p

    if x == from_alice:
        print "Access granted"
    else:
        print "Access denied"
