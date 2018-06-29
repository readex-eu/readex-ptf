#include <scorep/SCOREP_User.inc>

program add
include "mpif.h"

double precision a(10000), s, s1
integer, volatile :: i,k,l,mm
integer myrank, np, ierr
volatile myrank
integer status(MPI_STATUS_SIZE)

SCOREP_USER_REGION_DEFINE( mainRegion )

call mpi_init(ierr)
call mpi_comm_rank(MPI_COMM_WORLD, myrank, ierr)
call mpi_comm_size(MPI_COMM_WORLD, np, ierr)

if (myrank==0) write (*,*) "add.f90 started with ", np, " processes"

mm=1
if (myrank==0) mm=10000

do k=1,30

SCOREP_USER_OA_PHASE_BEGIN( mainRegion,"mainRegion",SCOREP_USER_REGION_TYPE_COMMON )



do i=1,mm
   a(i)=500*i
enddo

do i=1,mm
	s=s+a(i)
enddo


call mpi_barrier(MPI_COMM_WORLD, ierr)
SCOREP_USER_OA_PHASE_END( mainRegion )
enddo



if (myrank==0) write (*,*) "hallo", a(100), s1
call mpi_finalize(ierr)
end
