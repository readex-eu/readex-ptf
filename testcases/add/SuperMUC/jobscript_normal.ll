#@ wall_clock_limit = 00:30:00
#@ job_name = mytest
#@ job_type = MPICH
#@ class = test
#@ island_count = 1
#@ node = 1
#@ total_tasks = 1
#@ node_usage = not_shared
#@ initialdir = .
#@ output = out.txt
#@ error = out.txt
#@ notification = never
#@ queue
. /etc/profile
. /etc/profile.d/modules.sh

export OMP_NUM_THREADS=1
mpiexec -n 1 ../add.exe

export OMP_NUM_THREADS=2
mpiexec -n 1 ./add.exe

export OMP_NUM_THREADS=4
mpiexec -n 1 ./add.exe

export OMP_NUM_THREADS=8
mpiexec -n 1 ./add.exe

export OMP_NUM_THREADS=16
mpiexec -n 1 ./add.exe
