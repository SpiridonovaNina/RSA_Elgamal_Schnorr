#!/usr/bin/env python
#coding: UTF-8

import bigNumber.bigNumber as bigNumber
import elgamal
import rsa
import schnorr
    
def keys_schnorr(bitlen, name):
    pass


def get_args():
    import argparse

    parser = argparse.ArgumentParser(
        prog="gen_keys", description="Key generator for RSA, ElGamal, Schnorr")
    parser.add_argument('type', choices=['rsa', 'elgamal', 'schnorr'],
                        help="cipher type")
    parser.add_argument('len', choices=['128', '256', '512', '1024', '2048'],
                        help="key lenght (bites)")
    parser.add_argument('keychain_name', nargs='?', default='key',
                        help="[name].pub & [name].priv, default=key")
    return parser.parse_args()


if __name__ == "__main__":
    args = get_args()
    funcs = {'rsa': rsa.generate_keys, 'elgamal': elgamal.generate_keys, 'schnorr': schnorr.generate_keys}
    funcs[args.type](int(args.len), args.keychain_name)
    print "keychain {} for {} generated".format(args.keychain_name, args.type)
