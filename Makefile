CPP=mpic++
# DO NOT FORGET to replace on remote:
# CPP=mpiicc

CFLAGS=-lm
COPTFLAGS=-O3 -ffast-math -fopenmp -flto -march=native -funroll-loops -diag-disable=10441
OPTFLAGS=-O3 -ffast-math -fopenmp -flto -march=native -funroll-loops -diag-disable=10441
MPIFLAGS=-D_MPI
DEBUGFLAGS=-g -pg

SBATCHS:=256 512 1024 2048 4096
REFS:=$(SBATCHS:%=res/base%.out)
