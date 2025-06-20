#!/bin/bash
#SBATCH --partition=intel
#SBATCH --nodes=4
#SBATCH --ntasks=32
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=8
#SBATCH --output=mpi.log
mpirun  ./build/mpi.exe  --nx $1 --ny $1
