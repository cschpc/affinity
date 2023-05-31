# Affinity testing

This simply code prints outs to which core MPI rank and OpenMP thread is bind to.
Code performs also dummy calculation, increase in execution time shows how 
oversubscription of cores affects performance. 

With modern CPUs, clock frequency is typically adjusted dynamically based on the
number of cores in use, and comparing timings with single MPI task, single thread 
vs. full node gives also hints how frequency scaling affects performance

In order to test possibly different OpenMP runtime behaviour, there is both C and Fortran
version.

## Usage

Requires MPI and OpenMP.

Compile and run C version:
```
mpicc -o cpu_affinity cpu_affinity.c utilities.c -fopenmp -lm
export OMP_NUM_THREADS=<threads>
mpiexec -np <tasks> ./cpu_affinity
```

Compile and run Fortran version:
```
gcc -c utilities.c
mpif90 -o cpu_affinity cpu_affinity.f90 utilities.o -fopenmp 
export OMP_NUM_THREADS=<threads>
mpiexec -np <tasks> ./cpu_affinity
```


