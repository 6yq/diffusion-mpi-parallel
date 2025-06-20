SHELL = bash

CPP ?= mpic++
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
OUTDIR  := output
SHDIR   := sh

HOSTNAME := $(shell hostname)
USERNAME := $(shell whoami)

SAMPLES := 256 512 1024 2048 4096
REFS    := $(SAMPLES:%=$(REFDIR)/base%.out)
OUTS    := $(SAMPLES:%=$(OUTDIR)/output%.out)

# --------------------
# Top-level targets
# --------------------
.PHONY: all serial mpi ref run clean

all: serial

ref: $(REFS)
run: $(OUTS)

# --------------------
# Build serial target
# --------------------
serial: $(BUILD)/serial.exe

$(BUILD)/serial.exe: $(SRC)/mainSerial.cpp $(SRC)/DiffusionSolver.cpp $(SRC)/scenarios.cpp \
                     $(SRC)/DiffusionSolver.hpp $(SRC)/scenarios.hpp $(SRC)/note.hpp
	@mkdir -p $(BUILD)
	$(CPP) $(CFLAGS) $(OPTFLAGS) $(INTELFLAGS) $^ -o $@

# --------------------
# Build mpi target
# --------------------
mpi: $(BUILD)/mpi.exe

$(BUILD)/mpi.exe: $(SRC)/mainMPI.cpp $(SRC)/DiffusionSolver.cpp $(SRC)/scenarios.cpp \
                  $(SRC)/DiffusionSolver.hpp $(SRC)/scenarios.hpp $(SRC)/note.hpp
	@mkdir -p $(BUILD)
	$(CPP) $(CFLAGS) $(OPTFLAGS) $(MPIFLAGS) $(INTELFLAGS) $^ -o $@

# --------------------
# Generate reference results
# --------------------
$(REFDIR)/base%.out: $(BUILD)/serial.exe $(SHDIR)/run.sh
	@mkdir -p $(REFDIR)
	bash $(SHDIR)/run.sh serial $* --genRef

# --------------------
# Generate output results (test run)
# --------------------
$(OUTDIR)/output%.out: $(BUILD)/serial.exe $(REFS) $(SHDIR)/run.sh
	@mkdir -p $(OUTDIR)
	bash $(SHDIR)/run.sh serial $*

# --------------------
# Clean up
# --------------------
clean:
	rm -f *.out
	rm -f *.log
	rm -rf $(BUILD)/*.exe $(REFDIR)/*.out $(OUTDIR)/*.out
