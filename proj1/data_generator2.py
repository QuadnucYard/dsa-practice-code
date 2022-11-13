import os
import struct
import sys
import numpy as np


def gen(n, m) -> np.ndarray:
    return np.random.randint(low=0, high=255, size=(n, m))


def save(mat: np.ndarray, path: str) -> None:
    with open(path, "wb") as fout:
        print(mat.shape)
        fout.write(struct.pack("2I", *mat.shape))
        fout.write(struct.pack(f"{mat.size}i", *mat.reshape(-1)))


sizes = [1 << 6, 1 << 7, 1 << 8, 1 << 9, 1 << 10]

for i, n in enumerate(sizes):

    matA = gen(n, n)
    matB = gen(n, n)
    matC = matA @ matB

    save(matA, f"proj1/data/mat_{i}_A.in")
    save(matB, f"proj1/data/mat_{i}_B.in")
    save(matC, f"proj1/data/mat_{i}_C.ans")
    print(matA)
