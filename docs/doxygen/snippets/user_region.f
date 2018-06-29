       do  step = 1, niter
!$MON user region
         if (mod(step, 20) .eq. 0 .or. step .eq. 1) then
            if (myid .eq. root) write(*, 200) step
 200        format(' Time step ', i4)
         endif

         call exch_qbc(u, qbc_ou, qbc_in, nx, nxmax, ny, nz, 
     $                 0)

         do iz = 1, proc_num_zones
           zone = proc_zone_id(iz)
           call adi(rho_i(start1(iz)), us(start1(iz)), 
     $              vs(start1(iz)), ws(start1(iz)), 
     $              qs(start1(iz)), square(start1(iz)), 
     $              rhs(start5(iz)), forcing(start5(iz)), 
     $              u(start5(iz)), 
     $              nx(zone), nxmax(zone), ny(zone), nz(zone))
         end do
!$MON end user region         
       end do
