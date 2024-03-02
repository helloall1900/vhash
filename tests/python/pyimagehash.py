#!/usr/bin/env python3

from PIL import Image
import imagehash
import sys
import time


IMAGE_FILE = 'tests/testdata/lena.png'


def bench_it(iter=100):
    start = time.time()
    for i in range(iter):
        image = Image.open(IMAGE_FILE)
        _ = imagehash.average_hash(image)
    elapsed = int(((time.time() - start) * 1000000000) / iter)
    print(f'ahash: {elapsed} ns')

    start = time.time()
    for i in range(iter):
        image = Image.open(IMAGE_FILE)
        _ = imagehash.phash(image)
    elapsed = int(((time.time() - start) * 1000000000) / iter)
    print(f'phash: {elapsed} ns')

    start = time.time()
    for i in range(iter):
        image = Image.open(IMAGE_FILE)
        _ = imagehash.dhash(image)
    elapsed = int(((time.time() - start) * 1000000000) / iter)
    print(f'dhash: {elapsed} ns')

    start = time.time()
    for i in range(iter):
        image = Image.open(IMAGE_FILE)
        _ = imagehash.whash(image)
    elapsed = int(((time.time() - start) * 1000000000) / iter)
    print(f'whash: {elapsed} ns')


def test_it():
    image = Image.open(IMAGE_FILE)
    hash = imagehash.average_hash(image)
    print('ahash: ', hash)

    hash = imagehash.phash(image)
    print('phash: ', hash)

    hash = imagehash.dhash(image)
    print('dhash: ', hash)

    hash = imagehash.whash(image)
    print('whash: ', hash)


if len(sys.argv) > 1 and sys.argv[1] == 'bench':
    bench_it()
else:
    test_it()
