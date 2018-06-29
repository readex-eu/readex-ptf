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
psc_frontend --apprun=../add.exe --mpinumprocs=1 --strategy=Importance --phase="mainRegion" --force-localhost 
#psc_frontend --apprun=../add.gdb --mpinumprocs=1 --strategy=Importance --phase="mainRegion" --force-localhost 
#psc_frontend --apprun=../add.valgrind --mpinumprocs=1 --strategy=Importance --phase="mainRegion" --force-localhost 
