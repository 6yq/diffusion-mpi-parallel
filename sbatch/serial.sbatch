#!/bin/bash
#SBATCH --partition=intel
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
./build/serial.exe --n-theta $1 --n-phi $1 $2
