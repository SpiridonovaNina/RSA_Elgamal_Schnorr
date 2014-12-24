#!/usr/bin/env python
#coding: UTF-8

import bigNumber.bigNumber as bigNumber
bigNumber.initRandom() # инициализация генератора больших случайных чисел

def MillerRabinPass(a, s, t, n):
    """Один шаг теста Миллера-Рабина
    """
    x = bigNumber.Pow(a, t, n)
    if x == 1:
        return True

    i = bigNumber.bigNumber(0)
    while i < s - 1:
        x = (x * x) % n
        if x == n - 1:
            return True
        i = i + 1

    return x == n - 1


def MillerRabin(m):
    """Тест Миллера-Рабина на простоту числа
    """
    t = m - 1
    s = bigNumber.bigNumber(0)
    while t % 2 == 0:
        t /= 2
        s += 1
        
    for repeat in range(20):
        a = 0
        while a == 0:
            a = bigNumber.GenerateRandomMax(m - 4) + 2
        if not MillerRabinPass(a, s, t, m):
            return False
            
    return True


def GeneratePrime(bitLen):
    """ Генерация большого простого числа заданной длины
    """
    P = bigNumber.GenerateRandomLen(bitLen)
    while not MillerRabin(P):
        P += 1
    return P