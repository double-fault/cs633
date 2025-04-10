import random
import sys
import struct

def main():
    if len(sys.argv) != 5:
        print("Usage: python script.py x y z m")
        return

    x, y, z, m = map(int, sys.argv[1:])
    n = x * y * z
    base_filename = f"data_{x}_{y}_{z}_{m}"
    txt_filename = base_filename + ".txt"
    bin_filename = base_filename + "_bin.txt"

    with open(txt_filename, 'w') as txt_file, open(bin_filename, 'wb') as bin_file:
        for _ in range(n):
            row = [round(random.uniform(-50, 50), 2) for _ in range(m)]
            # Write to text file
            txt_file.write(' '.join(f"{val:.2f}" for val in row) + '\n')
            # Write to binary file (64-bit doubles)
            for val in row:
                bin_file.write(struct.pack('d', val))

if __name__ == "__main__":
    main()

