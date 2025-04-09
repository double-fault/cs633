import random
import sys

def main():
    if len(sys.argv) != 5:
        print("Usage: python script.py x y z m")
        return

    x, y, z, m = map(int, sys.argv[1:])
    n = x * y * z
    filename = f"data_{x}_{y}_{z}_{m}.txt"

    with open(filename, 'w') as f:
        for _ in range(n):
            row = [f"{random.uniform(-50, 50):.2f}" for _ in range(m)]
            f.write(' '.join(row) + '\n')

if __name__ == "__main__":
    main()

