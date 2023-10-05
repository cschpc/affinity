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
#include <numaif.h>

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

char* get_mempolicy_str(char *str)
{
  unsigned long nodemask;
  const unsigned long maxnode = 8;
  int mode;
  get_mempolicy(&mode, &nodemask, maxnode, NULL, 0);
  if ( MPOL_DEFAULT == mode) {
    sprintf(str, "MPOL_DEFAULT");
    return str;
  }

  int entry_made = 0;
  for (int i=0; i < maxnode; i++) {
    if (nodemask & (1 << i)) {
      int run = 0;
      entry_made = 1;
      for (int j = i + 1; j < maxnode; j++) {
        if (nodemask & (1 << j)) run++;
        else break;
      }
      if (!run)
        sprintf(str, "%d,", i);
      else if (run == 1) {
        sprintf(str, "%d,%d,", i, i + 1);
        i++;
      } else {
        sprintf(str, "%d-%d,", i, i + run);
        i += run;
      }
      while (*str != 0) str++;
    }
  }
  str -= entry_made;
  *str = '\0';
  return(str);
}

double get_current_cpu_freq()
{
  double freq;
  char str1[80], str2[80], str3[80];
  int hwthread  = sched_getcpu();
  char cmd[] = "awk '/cpu MHz/ {print $4}' /proc/cpuinfo";

  FILE *file = popen(cmd, "r");
  if (file == NULL) return 0.0;
  for (int i=0; i < hwthread; i++) {
    //int n = fscanf(file, "%s %s %s %lf", str1, str2, str2, &freq);
    int n = fscanf(file, "%lf", &freq);
    if (n != 1) return 0.0;
  }
  pclose(file);
  return freq;
} 
