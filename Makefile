CPP=mpic++
# DO NOT FORGET to replace on remote:
# CPP=mpiicc

CFLAGS=-lm
COPTFLAGS=-O3 -ffast-math -fopenmp -flto -march=native -funroll-loops
OPTFLAGS=-O3 -ffast-math -fopenmp -flto -march=native -funroll-loops
MPIFLAGS=-D_MPI
DEBUGFLAGS=-g -pg

# DO NOT FORGET to uncomment on remote:
# INTELFLAGS = -diag-disable=10441

BUILD:=build
SRC:=src
RES:=res

SBATCHS:=256 512 1024 2048 4096
REFS:=$(SBATCHS:%=res/base%.out)

# ------------
# MAIN ENTRIES
# ------------

.PHONY: ref

ref: $(REFS)

all: serial mpi

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
# RUN
# ---

run: ref
	sbatch serial.sbatch 4096
	#sbatch  mpi.sbatch 4096    # mpi

clean:
	rm -f *.out
	rm -f *.log
	rm -rf $(BUILD)/*.exe
