#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Fractionの独立参照。Python fractionsの任意精度演算で期待値を作る。"""
from fractions import Fraction
from pathlib import Path
import argparse
import random

MIN = -(1 << 63)
MAX = (1 << 63) - 1
RNG = random.Random(0x4652414354494F4E)

def encoded(x):
    if not MIN <= x.numerator <= MAX or not 1 <= x.denominator <= MAX:
        return 'OVERFLOW'
    return f'{x.numerator}/{x.denominator}'

def good_pair(n, d):
    if d == 0:
        return False
    return encoded(Fraction(n, d)) != 'OVERFLOW'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    lines = []
    def emit(op, operands, result):
        lines.append(' '.join([op, *map(str, operands), str(result)]))
    def constructor(n, d):
        emit('CTOR', [n, d], 'ZERO' if d == 0 else encoded(Fraction(n, d)))
    def scalar(n, d):
        a = Fraction(n, d)
        q = abs(a.numerator) // a.denominator
        if a.numerator < 0:
            q = -q
        expected = f'{a.numerator // a.denominator},{-((-a.numerator) // a.denominator)},{q},{(a > 0) - (a < 0)},{int(a.denominator == 1)}'
        emit('SCALAR', [n, d], expected)
        for op, f in [('NEG', lambda x: -x), ('ABS', abs), ('INV', lambda x: 1 / x)]:
            try:
                result = encoded(f(a))
            except ZeroDivisionError:
                result = 'ZERO'
            emit(op, [n, d], result)
    def binary(x, y):
        n, d = x
        m, e = y
        a, b = Fraction(n, d), Fraction(m, e)
        for op, f in [('ADD', lambda: a+b), ('SUB', lambda: a-b), ('MUL', lambda: a*b), ('DIV', lambda: a/b)]:
            try:
                answer = encoded(f())
            except ZeroDivisionError:
                answer = 'ZERO'
            emit(op, [n, d, m, e], answer)
            emit('I'+op, [n, d, m, e], answer)
        emit('CMP', [n, d, m, e], (a > b) - (a < b))
    edges = [MIN, MIN+1, MIN+2, -(1<<62), -MAX//2, -1000000007, -2147483648, -2, -1, 0, 1, 2, 2147483647, 1000000007, MAX//2, (1<<62), MAX-2, MAX-1, MAX]
    for n in edges:
        for d in edges:
            constructor(n, d)
    for _ in range(12000):
        n = RNG.randint(MIN, MAX)
        d = RNG.choice(edges) if RNG.randrange(4) == 0 else RNG.randint(MIN, MAX)
        constructor(n, d)
    pairs = [(n,d) for n in edges for d in edges if good_pair(n,d)]
    for x in pairs:
        scalar(*x)
    # 小さい全組合せ。演算結果が収まり、符号・順序・丸めを網羅する。
    small = [(n,d) for n in range(-6,7) for d in range(1,7)]
    for x in small:
        for y in small:
            binary(x,y)
    for _ in range(6000):
        scale = RNG.choice([10**3, 10**6, 10**9, MAX])
        x = (RNG.randint(-scale,scale), RNG.randint(1,scale))
        mode = RNG.randrange(5)
        if mode == 0:
            y = (RNG.randint(-scale,scale), x[1])
        elif mode == 1 and x[0] != 0:
            y = (x[1], x[0])
        elif mode == 2:
            y = (RNG.choice([-1,1])*x[0], x[1])
        elif mode == 3:
            y = RNG.choice(pairs)
        else:
            y = (RNG.randint(-scale,scale), RNG.randint(1,scale))
        binary(x,y)
        if RNG.randrange(4)==0:
            scalar(*x)
    for _ in range(4000):
        binary(RNG.choice(pairs), RNG.choice(pairs))
    # 大きな中間積が最終約分で収まるもの、1刻みの境界。
    for d in [MAX, MAX-1, MAX-2, 1<<62, (1<<62)-1, 2147483647]:
        for n in [MIN, MIN+1, -d, -d+1, -1, 0, 1, d-1, d]:
            if good_pair(n,d):
                scalar(n,d)
                for m in [MIN, MIN+1, -n if n != MIN else MAX, -1, 0, 1, MAX]:
                    binary((n,d),(m,d))
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text('\n'.join(lines)+'\n')
    print(f'{len(lines)} independent Python-fractions cases: {args.output}')
if __name__ == '__main__':
    main()
