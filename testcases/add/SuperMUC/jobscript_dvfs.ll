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


psc_frontend --apprun=../add.exe --ompnumthreads=16 --tune=dvfs --phase="mainRegion" --force-localhost
#psc_frontend --apprun=../add.gdb --ompnumthreads=16 --tune=dvfs --phase="mainRegion" --force-localhost
#psc_frontend --apprun=../add.valgrind --ompnumthreads=16 --tune=dvfs --phase="mainRegion" --force-localhost
