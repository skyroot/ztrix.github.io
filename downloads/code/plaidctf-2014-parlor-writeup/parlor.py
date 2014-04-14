#!/usr/bin/env python2
#-*- coding:utf-8 -*-

import os, sys
import subprocess

# https://github.com/zTrix/zio
from zio import *

def md5_ext(s, a):
    tl = len(s) * 8
    ret = s + '\x80'
    while len(ret) % 64 != 56:
        ret += '\x00'
    ret += l32(tl)
    ret += l32(0)
    return ret + a

host = '54.197.195.247'
port = 4321

io = zio((host, port), print_write = COLORED(REPR))

io.read_until('6) quit')
io.writeline('1')
io.read_until('and 100):')
io.writeline('100')

io.read_until('6) quit')
io.writeline('3')
io.read_until('nonce for this round')
io.write('a\n')
io.read_until('we generated ')
remain1 = int(io.read_until(',')[:-1])
remain1_str = '0' * 7 + hex(remain1)[2:].rstrip('L')
print remain1, remain1_str

ext = md5_ext('k' * 16 + 'a\n', 'b\n')
io.read_until('6) quit')
io.writeline('3')
io.read_until('nonce for this round')
io.write(ext[16:])
io.read_until('we generated ')
remain2 = int(io.read_until(',')[:-1])
remain2_str = '0' * 7 + hex(remain2)[2:].rstrip('L')
print remain2, remain2_str

if False:
    io.read_until('6) quit')
    io.writeline('5')
    io.read_until('the nonce has been ')
    nonce = int(io.readline().strip(), 16)
    print hex(nonce)[2:].rstrip('L')

log(' '.join(['./search', remain1_str, remain2_str]), 'red')
prefix = subprocess.check_output(['./search', remain1_str, remain2_str])
assert len(prefix.strip()) == 7

first_md5 = prefix.strip() + remain1_str[7:]

io.read_until('6) quit')
io.writeline('1')
io.read_until('between 1 and 100):')
io.writeline('1')

log(' '.join(['./even', first_md5]), 'red')
candi = subprocess.check_output(['./even', first_md5])
for line in candi.strip().splitlines():
    ary = line.split(', ')
    c = chr(int(ary[0]))
    if c == 'b': continue
    ext = md5_ext('k' * 16 + 'a\n', c + '\n')
    log('hash expectation: %s' % ary[1], 'red')

    io.read_until('6) quit')
    io.writeline('2')
    io.read_until(' and ')
    balance = io.read_until('):')[:-2]
    io.writeline(balance)

    io.read_until('6) quit')
    io.writeline('3')
    io.read_until('nonce for this round')
    io.write(ext[16:])

    rs = io.read_until('====')
    if 'Too bad' in rs:
        break
    elif 'key' in rs:
        break
    
io.close()

