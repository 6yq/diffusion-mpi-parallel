SHELL=bash
CPP?=mpic++
# on remote:
# CPP=mpiicc

CFLAGS=-lm
COPTFLAGS=-O3 -ffast-math -fopenmp -flto -march=native -funroll-loops
OPTFLAGS=-O3 -ffast-math -fopenmp -flto -march=native -funroll-loops
MPIFLAGS=-D_MPI
DEBUGFLAGS=-g -pg
INTELFLAGS?=
# on remote:
# INTELFLAGS = -diag-disable=10441

BUILD:=build
SRC:=src
RES:=res

HOSTNAME:=$(shell hostname)
USERNAME:=$(shell whoami)

SAMPLES:=256 512 1024 2048 4096
REFS:=$(SAMPLES:%=res/base%.out)

# ---
# REF
# ---

.PHONY: ref serial mpi run clean

ref: $(REFS)

$(RES)/base%.out: sh/run.sh
	@echo "[Make] Generating $@ using run.sh"
	bash sh/run.sh serial $* --genRef

# ------
# SERIAL
# ------

serial: $(BUILD)/serial.exe

$(BUILD)/serial.exe: $(SRC)/mainSerial.cpp $(SRC)/DiffusionSolver.cpp $(SRC)/scenarios.cpp \
                     $(SRC)/DiffusionSolver.hpp $(SRC)/scenarios.hpp $(SRC)/note.hpp
	@mkdir -p $(BUILD)
	$(CPP) $(CFLAGS) $(OPTFLAGS) $^ -o $@

# ---
# MPI
# ---

mpi: $(BUILD)/mpi.exe

$(BUILD)/mpi.exe: $(SRC)/mainMPI.cpp $(SRC)/DiffusionSolver.cpp $(SRC)/scenarios.cpp \
                  $(SRC)/DiffusionSolver.hpp $(SRC)/scenarios.hpp $(SRC)/note.hpp
	@mkdir -p $(BUILD)
	$(CPP) $(CFLAGS) $(OPTFLAGS) $(MPIFLAGS) $^ -o $@

# ---
# REF
# ---

$(RES)/base%.out: $(BUILD)/serial.exe
	@mkdir -p $(RES)
	./build/serial.exe --ratio $* --genRef

# ---
# RUN
# ---

run: $(REFS)
	sbatch serial.sbatch 4096
	#sbatch  mpi.sbatch 4096    # mpi

clean:
	rm -f *.out
	rm -f *.log
	rm -rf $(BUILD)/*.exe
