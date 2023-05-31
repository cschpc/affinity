   ! Print outs to which core MPI rank and OpenMP thread is bind to.
   ! Performs also dummy calculation, increase in execution time indicates
   ! that cores are oversubscribed. Comparing times with single MPI task,
   ! single thread vs. full node gives also hints about effects of CPU
   ! frequency scaling.

   ! Copyright (C) 2023 CSC - IT Center for Science

   ! This file may be redistributed under the terms of the
   ! GNU General Public License. See accompanying LICENSE for details

program affinity
  use, intrinsic :: iso_c_binding
  use iso_fortran_env, only : REAL64
  use omp_lib
  use mpi
  implicit none

  interface 

    subroutine affinity_string(str) bind(C, name='get_affinity_str')
      use, intrinsic :: iso_c_binding

      character(kind=c_char) :: str(*)
    end subroutine

  end interface

  integer :: rank, thread_id, provided, name_len, i, ierr
  integer, parameter :: niter =  80000000
  real(REAL64) :: x, y, z, t
  
  character(len=7*128) :: cpu_mask 
  character(len=MPI_MAX_PROCESSOR_NAME) :: hostname

  call mpi_init_thread(MPI_THREAD_FUNNELED, provided, ierr)
  call mpi_comm_rank(MPI_COMM_WORLD, rank, ierr)
  hostname = ''
  call mpi_get_processor_name(hostname, name_len, ierr)

! Reduction should ensure that compiler does not optimize "calculation" away
!$omp parallel private(thread_id, cpu_mask, t) reduction(+:z)
  cpu_mask = ''
  thread_id = omp_get_thread_num()

  z = 0.0
  t = MPI_Wtime()
  do i=1, niter
    x = cos(i*0.1)*exp(i*0.04)
    y = sin(i*0.1)*exp(i*0.04)
    z = z + ((x*x)+(y*y))
  end do
  t = MPI_Wtime() - t

  call affinity_string(cpu_mask)

!$omp critical
  write(*,'(A,I3.3,A,I2.2,5A,F6.4,A)') 'Rank ', rank, ' thread ', thread_id, ' on ', & 
    &    trim(hostname), ' core = ', trim(cpu_mask), ' (', t, ' seconds)'
!$omp end critical
!$omp end parallel

  ! Print to avoid compiler optimizations
  if (z < 0.0) then
    write(*,*) 'Should not happen'
  end if

  call mpi_finalize(ierr)

end program
  

  

