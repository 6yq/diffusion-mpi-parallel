SHELL = bash

CPP = mpic++
# on remote:
# CPP = mpiicc

CFLAGS     = -lm
COPTFLAGS  = -O3 -ffast-math -fopenmp -flto -march=native -funroll-loops
OPTFLAGS   = -O3 -ffast-math -fopenmp -flto -march=native -funroll-loops
MPIFLAGS   = -D_MPI
DEBUGFLAGS = -g -pg
INTELFLAGS ?=
# on remote:
# INTELFLAGS = -diag-disable=10441

BUILD   := build
SRC     := src
REFDIR  := ref
OUTDIR  := opt
SHDIR   := sh

HOSTNAME := $(shell hostname)
USERNAME := $(shell whoami)

SAMPLES := 256 512 1024 2048 4096
REFS    := $(SAMPLES:%=$(REFDIR)/base%.out)
OUTS    := $(SAMPLES:%=$(OUTDIR)/opt%.out)
OPTMODE ?= mpi

# --------------------
# Top-level targets
# --------------------
.PHONY: serial mpi ref run clean

ref: $(REFS)
run: $(OUTS)

COMMON_SRC := $(SRC)/scenarios.cpp $(SRC)/correctness.cpp
COMMON_HDR := $(SRC)/DiffusionSolver.hpp $(SRC)/scenarios.hpp $(SRC)/note.hpp $(SRC)/correctness.hpp

# --------------------
# Build serial target
# --------------------
serial: $(BUILD)/serial.exe

$(BUILD)/serial.exe: $(SRC)/mainSerial.cpp $(SRC)/DiffusionSolver.cpp $(COMMON_SRC) $(COMMON_HDR)
	@mkdir -p $(BUILD)
	$(CPP) $(SRC)/mainSerial.cpp $(SRC)/DiffusionSolver.cpp $(COMMON_SRC) -o $@ $(CFLAGS) $(OPTFLAGS) $(INTELFLAGS)

# --------------------
# Build mpi target
# --------------------
mpi: $(BUILD)/mpi.exe

$(BUILD)/mpi.exe: $(SRC)/mainMPI.cpp $(SRC)/DiffusionSolverMPI.cpp $(COMMON_SRC) $(COMMON_HDR)
	@mkdir -p $(BUILD)
	$(CPP) $(SRC)/mainSerial.cpp $(SRC)/DiffusionSolverMPI.cpp $(COMMON_SRC) -o $@ $(CFLAGS) $(OPTFLAGS) $(MPIFLAGS) $(INTELFLAGS)

# --------------------
# Generate reference results
# --------------------
$(REFDIR)/base%.out: $(SHDIR)/run.sh
	@mkdir -p $(REFDIR)
	bash $(SHDIR)/run.sh serial $* --genRef
	mv $(REFDIR)/base.out $@

# --------------------
# Generate output results (test run)
# --------------------
$(OUTDIR)/opt%.out: $(SHDIR)/run.sh
	@mkdir -p $(OUTDIR)
	bash $(SHDIR)/run.sh $(OPTMODE) $*
	mv $(OUTDIR)/opt.out $@

# --------------------
# Clean up
# --------------------
clean:
	rm -f *.out
	rm -f *.log
	rm -rf $(BUILD)/*.exe $(REFDIR)/*.out $(OUTDIR)/*.out
