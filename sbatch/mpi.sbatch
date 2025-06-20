#!/bin/bash
#SBATCH --partition=intel
#SBATCH --nodes=4
#SBATCH --ntasks=32
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=8
#SBATCH --output=mpi.log
mpirun  ./build/mpi.exe --n-theta $1 --n-phi $1 $2

SIZE=$1
MODE=$2
if [[ "$MODE" == "--genRef" ]]; then
  mv ref/base.out ref/base${SIZE}.out
  mv serial.log ref/serial${SIZE}.log
else
  mv opt/opt.out opt/opt${SIZE}.out
  mv serial.log opt/serial${SIZE}.log
fi
