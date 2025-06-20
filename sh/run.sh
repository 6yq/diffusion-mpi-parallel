#!/bin/bash

# --------
# Platform
# --------
HOSTNAME=$(hostname)
USERNAME=$(whoami)

if [[ "$USERNAME" == "lynn" && "$HOSTNAME" == "Cyborg" ]]; then
  PLATFORM="local"
else
  PLATFORM="remote"
fi

echo "[Info] Detected platform: $PLATFORM"

# -----
# FLAGS
# -----
MODE=$1         # serial / mpi / gpu
SIZE=$2
GENREF=$3       # optional：--genRef

MODE=${MODE:-serial}
SIZE=${SIZE:-0.0}
GENREF_FLAG=""
[[ "$GENREF" == "--genRef" ]] && GENREF_FLAG="--genRef"

# -----
# build
# -----
if [[ "$PLATFORM" == "remote" ]]; then
  echo "[Info] Loading modules for Intel on remote"
  source setenv.sh
  make $MODE CPP=mpiicc INTELFLAGS="-diag-disable=10441"
else
  echo "[Info] Using default Makefile on local"
  make $MODE CPP=mpic++
fi

# ---
# run
# ---
if [[ "$PLATFORM" == "remote" ]]; then
  echo "[Info] Submitting sbatch job"
  if [[ "$MODE" == "serial" ]]; then
    sbatch --output=ref/serial${SIZE}.log ./sbatch/${MODE}.sbatch $SIZE $GENREF_FLAG
  elif [[ "$MODE" == "mpi" ]]; then
    sbatch --output=ref/mpi${SIZE}.log ./sbatch/${MODE}.sbatch $SIZE $GENREF_FLAG
  fi
else
  echo "[Info] Running locally"
  if [[ "$MODE" == "serial" ]]; then
    ./build/serial.exe --n-theta $SIZE --n-phi $SIZE $GENREF_FLAG
  elif [[ "$MODE" == "mpi" ]]; then
    mpirun -np 4 ./build/mpi.exe --n-theta $SIZE --n-phi $SIZE
  else
    echo "[Error] Unknown mode: $MODE"
    exit 1
  fi
fi
