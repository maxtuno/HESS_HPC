#!/usr/bin/env bash
mpirun -q -np ${2} -hostfile hostfile --allow-run-as-root ./hess_hpc ${1} ${3}
pkill hpc_hpc
