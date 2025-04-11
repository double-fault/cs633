#!/bin/bash
#SBATCH -N 2
#SBATCH --ntasks-per-node=32
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --time=00:10:00		## wall-clock time limit	
#SBATCH --partition=standard 	## can be "standard" or "cpu"
#SBATCH --exclusive

mkdir -p results/v0
mkdir -p results/v1
mkdir -p results/v2

# 64 64 64 3 

mpirun -np 64 ./build/exec_v1 ./data/data_64_64_64_3.txt 4 4 4 64 64 64 3 ./results/v1/output_64_64_64_3_64_v1.txt
mpirun -np 64 ./build/exec_v2 ./data/data_64_64_64_3.bin.txt 4 4 4 64 64 64 3 ./results/v2/output_64_64_64_3_64_v2.bin.txt

# 64 64 96 7

mpirun -np 64 ./build/exec_v1 ./data/data_64_64_96_7.txt 4 4 4 64 64 96 7 ./results/v1/output_64_64_96_7_64_v1.txt
mpirun -np 64 ./build/exec_v2 ./data/data_64_64_96_7.bin.txt 4 4 4 64 64 96 7 ./results/v2/output_64_64_96_7_64_v2.bin.txt
