import os
import struct
import sys
import numpy as np

print(sys.argv, os.getcwd())
n, k, m = 8, 8, 8


def gen(n, m) -> np.ndarray:
    return np.random.randint(low=0, high=255, size=(n, m))


def save(mat: np.ndarray, path: str) -> None:
    with open(path, "wb") as fout:
        print(mat.shape)
        fout.write(struct.pack("2I", *mat.shape))
        fout.write(struct.pack(f"{mat.size}i", *mat.reshape(-1)))


matA = gen(n, k)
matB = gen(k, m)
matC = matA @ matB

save(matA, "proj1/data/matA2.in")
save(matB, "proj1/data/matB2.in")
save(matC, "proj1/data/matC2.ans")
print(matA)
