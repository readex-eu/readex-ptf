#!/bin/bash

#Submit this script with: sbatch thefilename

#SBATCH --time=01:00:00   # walltime
#SBATCH --ntasks=16   # number of processor cores (i.e. tasks)
#SBATCH --partition=haswell
##SBATCH --partition=sandy,west
#SBATCH --mem-per-cpu=300M   # memory per CPU core
#SBATCH -J "add"   # job name
#SBATCH -A p_readex

SCOREP_ENABLE_PROFILING=true
SCOREP_ENABLE_TRACING=false
# Set the max number of threads to use for programs using OpenMP. Should be <= ppn. Does nothing if the program doesn't use OpenMP.
export OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE
#OUTFILE="out.txt"
psc_frontend --apprun=../add.exe --mpinumprocs=1 --tune=mpicap --phase="mainRegion" --force-localhost
#psc_frontend --apprun=../add.gdb --mpinumprocs=1 --tune=mpicap --phase="mainRegion" --force-localhost
#psc_frontend --apprun=../add.valgrind --mpinumprocs=1 --tune=mpicap --phase="mainRegion" --force-localhost

exit 0
