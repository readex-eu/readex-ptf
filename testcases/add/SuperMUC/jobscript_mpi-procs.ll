#@ wall_clock_limit = 00:30:00
#@ job_name = mytest
#@ job_type = MPICH
#@ class = test
#@ island_count = 1
#@ node = 4
#@ tasks_per_node = 16
#@ node_usage = not_shared
#@ initialdir = .
#@ output = out.txt
#@ error = out.txt
#@ notification = never
#@ queue
. /etc/profile
. /etc/profile.d/modules.sh

module unload mpi.ibm
module load mpi.intel

psc_frontend --apprun=../add.exe --force-localhost --mpinumprocs=16 --ompnumthreads=1 --tune=mpicap --phase="mainRegion"
#psc_frontend --apprun=../add.gdb --force-localhost --mpinumprocs=16 --ompnumthreads=1 --tune=mpicap --phase="mainRegion"
#psc_frontend --apprun=../add.valgrind --force-localhost --mpinumprocs=16 --ompnumthreads=1 --tune=mpicap --phase="mainRegion"
