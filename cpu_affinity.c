/* 
   * Print outs to which core MPI rank and OpenMP thread is bind to.
   * Performs also dummy calculation, increase in execution time indicates
   * that cores are oversubscribed. Comparing times with single MPI task,
   * single thread vs. full node gives also hints about effects of CPU
   * frequency scaling.

   * Copyright (C) 2023 CSC - IT Center for Science

   * This file may be redistributed under the terms of the
   * GNU General Public License. See accompanying LICENSE for details
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

char* get_affinity_str(char *str)
{
  cpu_set_t mask;
  sched_getaffinity(0, sizeof(mask), &mask);

  // Code based on https://github.com/util-linux/util-linux (lib/cpuset.c)
  char *ptr = str;
  int i, j, entry_made = 0;
  for (i = 0; i < CPU_SETSIZE; i++) {
    if (CPU_ISSET(i, &mask)) {
      int run = 0;
      entry_made = 1;
      for (j = i + 1; j < CPU_SETSIZE; j++) {
	if (CPU_ISSET(j, &mask)) run++;
	else break;
      }
      if (!run)
	sprintf(ptr, "%d,", i);
      else if (run == 1) {
	sprintf(ptr, "%d,%d,", i, i + 1);
	i++;
      } else {
	sprintf(ptr, "%d-%d,", i, i + run);
	i += run;
      }
      while (*ptr != 0) ptr++;
    }
  }
  ptr -= entry_made;
  *ptr = '\0';
  return(str);
}


int main(int argc, char *argv[])
{
  int rank, thread_id, provided;
  int niter = 80000000;  // number of iterations per FOR loop
  double z;
  
  char cpu_mask[7 * CPU_SETSIZE], hostname[64]; // core and hostname strings

  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  memset(cpu_mask, 0, sizeof(cpu_mask));
  memset(hostname, 0, sizeof(hostname));
  gethostname(hostname, sizeof(hostname));

// Reduction should ensure that compiler does not optimize "calculation" away
#pragma omp parallel private(thread_id, cpu_mask) reduction(+:z)
  {
    thread_id = omp_get_thread_num();
    double t;

    z = 0.0;
    t = MPI_Wtime();
    for (int i=0; i<niter; i++)
      {
	double x = cos(i*0.1)*exp(i*0.04);
	double y = sin(i*0.1)*exp(i*0.04);
	z += ((x*x)+(y*y));  
      }
    t = MPI_Wtime() - t;

    get_affinity_str(cpu_mask);

#pragma omp critical
    printf("Rank %03d thread %02d on %s core = %s (%f seconds)\n",
	   rank, thread_id, hostname, cpu_mask, t);
  }

// Print to avoid compiler optimizations
  if (z < 0.0) printf("Should not happen\n");

  MPI_Finalize();
  return(0);
}