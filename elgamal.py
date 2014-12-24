#!/usr/bin/env python
#coding: UTF-8

import argparse
import sys

import bigNumber.bigNumber as bigNumber
import primes


def generate_keys(bitlen, name):
    """Генерация ключей
        Генерируется случайное простое число p длины n битов.
        Выбирается случайный элемент g поля Zp.
        Выбирается случайное целое число x такое, что 1 < x < p-1.
        Вычисляется y = g^x (mod p).
        Открытым ключом является тройка (p,g,y), закрытым ключом — число x.
    """
    p = primes.GeneratePrime(bitlen)
    g = bigNumber.GenerateRandomMax(p)
    x = bigNumber.GenerateRandomMax(p)
    y = bigNumber.Pow(g, x, p)

    pub_key = "\n".join(map(str, (p, g, y)))
    priv_key = "\n".join(map(str, (p, x)))

    with open(name + '.pub', 'w') as pub_file:
        pub_file.write(pub_key)

    with open(name + '.priv', 'w') as priv_file:
        priv_file.write(priv_key)


def decrypt(p, x, a, b):
    m = (b * bigNumber.Pow(a, p - 1 - x, p)) % p
    return m


def encrypt(p, g, y, m):
    """Сообщение M шифруется следующим образом:
        Выбирается сессионный ключ — случайное целое число k такое, что 1 < k < p - 1
        Вычисляются числа a = g^k (mod p) и b = y^k * M (mod p)
        Пара чисел (a, b) является шифротекстом.
    """
    if m >= p:
        raise ValueError('Message is too large!')
    k = bigNumber.GenerateRandomMax(p - 2) + 1
    a = bigNumber.Pow(g, k, p)
    b = bigNumber.Pow(y, k, p)
    b = (b * m) % p
    return a, b
    

def get_args():
    parser = argparse.ArgumentParser(
        prog="elgamal", description="ElGamal crypter/decrypter")
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
            (p, g, y) = key_file.read().split("\n")
            
        p = bigNumber.bigNumber(p)
        g = bigNumber.bigNumber(g)
        y = bigNumber.bigNumber(y)
        m = bigNumber.bigNumber()
        m.GetNumberFromBinFile(args.input)
        a, b = encrypt(p, g, y, m)

        result = "\n".join(map(str, (a, b)))
        with open(args.output, 'w') as out_file:
            out_file.write(result)

    else:
        with open(args.keychain + '.priv') as key_file:
            (p, x) = key_file.read().split("\n")
            
        with open(args.input) as in_file:
            (a, b) = in_file.read().split("\n")

        p = bigNumber.bigNumber(p)
        x = bigNumber.bigNumber(x)
        a = bigNumber.bigNumber(a)
        b = bigNumber.bigNumber(b)

        m = decrypt(p, x, a, b)
        m.SaveNumberInBinFile(args.output)
