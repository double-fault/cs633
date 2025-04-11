#!/bin/bash

make clean
make SRC_DIRS=v1
make SRC_DIRS=v2

sbatch job.sh
sbatch job2.sh
