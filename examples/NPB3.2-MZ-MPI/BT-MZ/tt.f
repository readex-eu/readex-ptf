      subroutine x_solve(rho_i, qs, square, u, rhs, nx, nxmax, ny, nz)

      include 'header.h'

      integer nx, nxmax, ny, nz
      double precision rho_i (  0:nxmax-1,0:ny-1,0:nz-1), 
     $                 qs    (  0:nxmax-1,0:ny-1,0:nz-1), 
     $                 square(  0:nxmax-1,0:ny-1,0:nz-1), 
     $                 u     (5,0:nxmax-1,0:ny-1,0:nz-1),
     $                 rhs   (5,0:nxmax-1,0:ny-1,0:nz-1)

      integer i,j,k,m,n,isize

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      if (timeron) call timer_start(t_xsolve)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     This function computes the left hand side in the xi-direction
c---------------------------------------------------------------------

!$OMP PARALLEL DEFAULT(SHARED) PRIVATE(n,m,i,j,k,isize)
!$OMP&  SHARED(dx5,dx4,dx3,dx2,dx1,tx2,tx1,dt,c1345,c3c4,con43,c1,c2,
!$OMP&         nx,ny,nz)
      isize = nx-1

c---------------------------------------------------------------------
c     determine a (labeled f) and n jacobians
c---------------------------------------------------------------------
!$OMP DO
      do k = 1, nz-2
         do j = 1, ny-2
            do i = 0, isize

               tmp1 = rho_i(i,j,k)
               tmp2 = tmp1 * tmp1
               tmp3 = tmp1 * tmp2
            enddo
c---------------------------------------------------------------------
c     now jacobians set, so form left hand side in x direction
c---------------------------------------------------------------------
            call lhsinit(lhs, isize)


         enddo
      enddo
!$OMP END DO nowait
!$OMP END PARALLEL
      if (timeron) call timer_stop(t_xsolve)

      return
      end
      
