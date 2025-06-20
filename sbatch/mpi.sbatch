#!/bin/bash
#SBATCH --partition=intel
#SBATCH --nodes=4
#SBATCH --ntasks=32
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=8
mpirun  ./build/mpi.exe --n-theta $1 --n-phi $1 $2
