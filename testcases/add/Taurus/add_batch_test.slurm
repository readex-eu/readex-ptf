#!/bin/bash

#Submit this script with: sbatch thefilename

#SBATCH --time=01:00:00   # walltime
#SBATCH --ntasks=16   # number of processor cores (i.e. tasks)
#SBATCH --partition=haswell
##SBATCH --partition=sandy,west
#SBATCH --mem-per-cpu=300M   # memory per CPU core
#SBATCH -J "add"   # job name
#SBATCH -A p_readex


# Set the max number of threads to use for programs using OpenMP. Should be <= ppn. Does nothing if the program doesn't use OpenMP.
export OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE
mpiexec -n 16 ../add

exit 0
