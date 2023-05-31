# Affinity testing

This simply code prints outs to which core MPI rank and OpenMP thread is bind to.
Code performs also dummy calculation, increase in execution time shows how 
oversubscription of cores affects performance. 

With modern CPUs, clock frequency is typically adjusted dynamically based on the
number of cores in use, and comparing timings with single MPI task, single thread 
vs. full node gives also hints how frequency scaling affects performance

## Usage

Requires MPI and OpenMP, compile and run as
```
mpicc -o cpu_affinity cpu_affinity.c -fopenmp -lm
export OMP_NUM_THREADS=<threads>
mpiexec -np <tasks> ./cpu_affinity
```



