    SUBROUTINE x_solve(rho_i,qs,square,u,rhs,nx,nxmax,ny,nz)

!---------------------------------------------------------------------
!---------------------------------------------------------------------

!  header.h

!---------------------------------------------------------------------
!---------------------------------------------------------------------

      IMPLICIT NONE

!---------------------------------------------------------------------
! The following include file is generated automatically by the
! "setparams" utility. It defines
!      problem_size:  maximum overall grid size
!      dt_default:    default time step for this problem size if no
!                     config file
!      niter_default: default number of iterations for this problem size
!---------------------------------------------------------------------

! NPROCS = 4 CLASS = A


!  This file is generated automatically by the setparams utility.
!  It sets the number of processors and the class of the NPB
!  in this directory. Do not modify it by hand.

      CHARACTER :: class
      PARAMETER (class='A')
      INTEGER :: num_procs, num_procs2
      PARAMETER (num_procs=4,num_procs2=4)
      INTEGER :: x_zones, y_zones
      PARAMETER (x_zones=4,y_zones=4)
      INTEGER :: gx_size, gy_size, gz_size, niter_default
      PARAMETER (gx_size=128,gy_size=128,gz_size=16)
      PARAMETER (niter_default=200)
      INTEGER :: problem_size
      PARAMETER (problem_size=58)
      INTEGER :: max_xysize, max_xybcsize
      INTEGER :: proc_max_size, proc_max_size5, proc_max_bcsize
      PARAMETER (max_xysize=4261)
      PARAMETER (max_xybcsize=3020)
      PARAMETER (proc_max_size=max_xysize*gz_size)
      PARAMETER (proc_max_size5=proc_max_size*5)
      PARAMETER (proc_max_bcsize=max_xybcsize*(gz_size-2))
      INTEGER :: max_numzones
      PARAMETER (max_numzones=5)
      DOUBLE PRECISION :: dt_default, ratio
      PARAMETER (dt_default=0.0008D0,ratio=4.5D0)
      INTEGER :: start1, start5, qstart_west, qstart_east
      INTEGER :: qstart_south, qstart_north, qoffset
      INTEGER :: qcomm_size, qstart2_west, qstart2_east
      INTEGER :: qstart2_south, qstart2_north
      LOGICAL :: convertdouble
      PARAMETER (convertdouble=.FALSE.)
      CHARACTER :: compiletime*(11)
      PARAMETER (compiletime='05 May 2012')
      CHARACTER :: npbversion*(3)
      PARAMETER (npbversion='3.2')
      CHARACTER :: cs1*(46)
      PARAMETER (cs1='psc_instrument -i -v -d -s ../bin/bt-mz.A.4...')
      CHARACTER :: cs2*(6)
      PARAMETER (cs2='$(F77)')
      CHARACTER :: cs3*(6)
      PARAMETER (cs3='(none)')
      CHARACTER :: cs4*(6)
      PARAMETER (cs4='(none)')
      CHARACTER :: cs5*(11)
      PARAMETER (cs5='-O3 -openmp')
      CHARACTER :: cs6*(7)
      PARAMETER (cs6='-openmp')
      CHARACTER :: cs7*(6)

      PARAMETER (cs7='randi8')
      INTEGER :: aa, bb, cc, block_size
      PARAMETER (aa=1,bb=2,cc=3,block_size=5)

      INTEGER :: npb_verbose
      DOUBLE PRECISION :: elapsed_time
      LOGICAL :: timeron
      COMMON /global/elapsed_time, npb_verbose, timeron

      DOUBLE PRECISION :: tx1, tx2, tx3, ty1, ty2, ty3, tz1, tz2, tz3, dx1, &
        dx2, dx3, dx4, dx5, dy1, dy2, dy3, dy4, dy5, dz1, dz2, dz3, dz4, dz5, &
        dssp, dt, ce(5,13), dxmax, dymax, dzmax, xxcon1, xxcon2, xxcon3, &
        xxcon4, xxcon5, dx1tx1, dx2tx1, dx3tx1, dx4tx1, dx5tx1, yycon1, &
        yycon2, yycon3, yycon4, yycon5, dy1ty1, dy2ty1, dy3ty1, dy4ty1, &
        dy5ty1, zzcon1, zzcon2, zzcon3, zzcon4, zzcon5, dz1tz1, dz2tz1, &
        dz3tz1, dz4tz1, dz5tz1, dnxm1, dnym1, dnzm1, c1c2, c1c5, c3c4, c1345, &
        conz1, c1, c2, c3, c4, c5, c4dssp, c5dssp, dtdssp, dttx1, dttx2, &
        dtty1, dtty2, dttz1, dttz2, c2dttx1, c2dtty1, c2dttz1, comz1, comz4, &
        comz5, comz6, c3c4tx3, c3c4ty3, c3c4tz3, c2iv, con43, con16

      COMMON /constants/tx1, tx2, tx3, ty1, ty2, ty3, tz1, tz2, tz3, dx1, dx2, &
        dx3, dx4, dx5, dy1, dy2, dy3, dy4, dy5, dz1, dz2, dz3, dz4, dz5, dssp, &
        dt, ce, dxmax, dymax, dzmax, xxcon1, xxcon2, xxcon3, xxcon4, xxcon5, &
        dx1tx1, dx2tx1, dx3tx1, dx4tx1, dx5tx1, yycon1, yycon2, yycon3, &
        yycon4, yycon5, dy1ty1, dy2ty1, dy3ty1, dy4ty1, dy5ty1, zzcon1, &
        zzcon2, zzcon3, zzcon4, zzcon5, dz1tz1, dz2tz1, dz3tz1, dz4tz1, &
        dz5tz1, dnxm1, dnym1, dnzm1, c1c2, c1c5, c3c4, c1345, conz1, c1, c2, &
        c3, c4, c5, c4dssp, c5dssp, dtdssp, dttx1, dttx2, dtty1, dtty2, dttz1, &
        dttz2, c2dttx1, c2dtty1, c2dttz1, comz1, comz4, comz5, comz6, c3c4tx3, &
        c3c4ty3, c3c4tz3, c2iv, con43, con16

      DOUBLE PRECISION :: cuf(0:problem_size), q(0:problem_size), &
        ue(0:problem_size,5), buf(0:problem_size,5)
      COMMON /work_1d/cuf, q, ue, buf

      !$OMP THREADPRIVATE (/work_1d/)
      DOUBLE PRECISION :: fjac(5,5,0:problem_size), njac(5,5,0:problem_size), &
        lhs(5,5,3,0:problem_size), rtmp(5,0:problem_size), tmp1, tmp2, tmp3
      COMMON /work_lhs/fjac, njac, lhs, rtmp, tmp1, tmp2, tmp3

      !$OMP THREADPRIVATE (/work_lhs/)
      INTEGER :: x_start(x_zones), x_end(x_zones), x_size(x_zones), &
        y_start(y_zones), y_end(y_zones), y_size(y_zones)
      COMMON /zones/x_start, x_end, x_size, y_start, y_end, y_size


!-----------------------------------------------------------------------
!   Timer constants
!-----------------------------------------------------------------------
      INTEGER :: t_rhsx, t_rhsy, t_rhsz, t_xsolve, t_ysolve, t_zsolve, &
        t_rdis1, t_rdis2, t_add, t_rhs, t_last, t_total
      PARAMETER (t_total=1)
      PARAMETER (t_rhsx=2)
      PARAMETER (t_rhsy=3)
      PARAMETER (t_rhsz=4)
      PARAMETER (t_rhs=5)
      PARAMETER (t_xsolve=6)
      PARAMETER (t_ysolve=7)
      PARAMETER (t_zsolve=8)
      PARAMETER (t_rdis1=9)
      PARAMETER (t_rdis2=10)
      PARAMETER (t_add=11)

      PARAMETER (t_last=11)
      INTEGER :: nx, nxmax, ny, nz
      DOUBLE PRECISION :: rho_i(0:nxmax-1,0:ny-1,0:nz-1), &
        qs(0:nxmax-1,0:ny-1,0:nz-1), square(0:nxmax-1,0:ny-1,0:nz-1), &
        u(5,0:nxmax-1,0:ny-1,0:nz-1), rhs(5,0:nxmax-1,0:ny-1,0:nz-1)

      INTEGER :: i, j, k, m, n, isize
      INTEGER :: psc_old_task_id
      INTEGER, EXTERNAL :: psc_get_task_id

!---------------------------------------------------------------------
!---------------------------------------------------------------------

      IF (timeron) CALL timer_start(t_xsolve)

!---------------------------------------------------------------------
!---------------------------------------------------------------------

!---------------------------------------------------------------------
!     This function computes the left hand side in the xi-direction
!---------------------------------------------------------------------

      CALL start_region(10,42,153,0,-1)
      !$OMP PARALLEL DEFAULT(SHARED), PRIVATE(n,m,i,j,k,isize), &
        !$OMP SHARED(dx5,dx4,dx3,dx2,dx1,tx2,tx1,dt,c1345,c3c4,con43,c1,c2,nx, &
        !$OMP ny,nz), FIRSTPRIVATE(psc_old_task_id)
      CALL start_region(11,42,153,0,-1)
      isize = nx - 1

!---------------------------------------------------------------------
!     determine a (labeled f) and n jacobians
!---------------------------------------------------------------------
      CALL start_region(12,42,34,0,-1)
      !$OMP DO 
      DO k = 1, nz - 2
        DO j = 1, ny - 2
          DO i = 0, isize

            tmp1 = rho_i(i,j,k)
            tmp2 = tmp1*tmp1
            tmp3 = tmp1*tmp2
          END DO
!---------------------------------------------------------------------
!     now jacobians set, so form left hand side in x direction
!---------------------------------------------------------------------
          CALL lhsinit(lhs,isize)


        END DO
      END DO
      !$OMP END DO NOWAIT
      CALL end_region(12,42,34,0,-1)
      CALL start_region(26,42,153,0,-1)
      psc_old_task_id = psc_get_task_id()
      !$OMP BARRIER 
      CALL psc_set_task_id(psc_old_task_id)
      CALL end_region(26,42,153,0,-1)
      CALL end_region(11,42,153,0,-1)
      !$OMP END PARALLEL 
      CALL end_region(10,42,153,0,-1)
      IF (timeron) CALL timer_stop(t_xsolve)

      RETURN
    END SUBROUTINE x_solve
