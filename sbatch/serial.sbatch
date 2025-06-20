#!/bin/bash
#SBATCH --partition=intel
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --output=serial.log
./build/serial.exe --nx $1 --ny $1
