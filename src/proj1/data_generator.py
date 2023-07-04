import os
import struct
import sys
import numpy as np

print(sys.argv, os.getcwd())
n, k, m = map(int, sys.argv[1 : 4])


def gen(n, m) -> np.ndarray:
    return np.random.randint(low=0, high=255, size=(n, m))


def save(mat: np.ndarray, path: str) -> None:
    with open(path, "wb") as fout:
        print(mat.shape)
        fout.write(struct.pack(f"{mat.size}i", *mat.reshape(-1)))
        fout.write(struct.pack("2I", *mat.shape))


matA = gen(n, k)
matB = gen(k, m)
matC = matA @ matB

save(matA, "output/proj1/data/matA.in")
save(matB, "output/proj1/data/matB.in")
save(matC, "output/proj1/data/matC.ans")
