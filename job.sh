#!/bin/bash
#SBATCH -N 1
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

mpirun -np 1 ./build/exec_v1 ./data/data_64_64_64_3.txt 1 1 1 64 64 64 3 ./results/v0/output_64_64_64_3_1__v0.txt

mpirun -np 8 ./build/exec_v1 ./data/data_64_64_64_3.txt 2 2 2 64 64 64 3 ./results/v1/output_64_64_64_3_8_v1.txt
mpirun -np 16 ./build/exec_v1 ./data/data_64_64_64_3.txt 4 2 2 64 64 64 3 ./results/v1/output_64_64_64_3_16_v1.txt
mpirun -np 32 ./build/exec_v1 ./data/data_64_64_64_3.txt 4 4 2 64 64 64 3 ./results/v1/output_64_64_64_3_32_v1.txt
#mpirun -np 64 ./build/exec_v1 ./data/data_64_64_64_3.txt 4 4 4 64 64 64 3 ./results/v1/output_64_64_64_3_64_v1.txt

mpirun -np 8 ./build/exec_v2 ./data/data_64_64_64_3.bin.txt 2 2 2 64 64 64 3 ./results/v2/output_64_64_64_3_8_v2.bin.txt
mpirun -np 16 ./build/exec_v2 ./data/data_64_64_64_3.bin.txt 4 2 2 64 64 64 3 ./results/v2/output_64_64_64_3_16_v2.bin.txt
mpirun -np 32 ./build/exec_v2 ./data/data_64_64_64_3.bin.txt 4 4 2 64 64 64 3 ./results/v2/output_64_64_64_3_32_v2.bin.txt
#mpirun -np 64 ./build/exec_v2 ./data/data_64_64_64_3.bin.txt 4 4 4 64 64 64 3 ./results/v2/output_64_64_64_3_64_v2.bin.txt


# 64 64 96 7

mpirun -np 1 ./build/exec_v1 ./data/data_64_64_96_7.txt 1 1 1 64 64 96 7 ./results/v0/output_64_64_96_7_1__v0.txt

mpirun -np 8 ./build/exec_v1 ./data/data_64_64_96_7.txt 2 2 2 64 64 96 7 ./results/v1/output_64_64_96_7_8_v1.txt
mpirun -np 16 ./build/exec_v1 ./data/data_64_64_96_7.txt 4 2 2 64 64 96 7 ./results/v1/output_64_64_96_7_16_v1.txt
mpirun -np 32 ./build/exec_v1 ./data/data_64_64_96_7.txt 4 4 2 64 64 96 7 ./results/v1/output_64_64_96_7_32_v1.txt
#mpirun -np 64 ./build/exec_v1 ./data/data_64_64_96_7.txt 4 4 4 64 64 96 7 ./results/v1/output_64_64_96_7_64_v1.txt

mpirun -np 8 ./build/exec_v2 ./data/data_64_64_96_7.bin.txt 2 2 2 64 64 96 7 ./results/v2/output_64_64_96_7_8_v2.bin.txt
mpirun -np 16 ./build/exec_v2 ./data/data_64_64_96_7.bin.txt 4 2 2 64 64 96 7 ./results/v2/output_64_64_96_7_16_v2.bin.txt
mpirun -np 32 ./build/exec_v2 ./data/data_64_64_96_7.bin.txt 4 4 2 64 64 96 7 ./results/v2/output_64_64_96_7_32_v2.bin.txt
#mpirun -np 64 ./build/exec_v2 ./data/data_64_64_96_7.bin.txt 4 4 4 64 64 96 7 ./results/v2/output_64_64_96_7_64_v2.bin.txt

#mpirun -np 16 ./build/exec_v2 ./data/data_100_100_100_60.bin.txt 4 2 2 100 100 100 60 ./results/output_100_100_100_60_v2.txt
