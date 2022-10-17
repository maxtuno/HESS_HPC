# HESS_HPC
An HPC implementación of HESS algorithm for TSP.

    usage: sh hess_hpc.sh instance.tsp number_of_nodes loggin=[0|1]
  
# Format

    number of points
    order x y
    ...

Note: order ignored only for easy compatibility with DIMACS format.

### Example 1:
    python3 gen_tsp.py 100 > 100.tsp
    time sh hess_hpc.sh 100.tsp 4 0

    s OPTIMAL
    v 83.3025692743183725

    real    0m0.361s
    user    0m0.129s
    sys     0m0.104s

    python3 plot_tsp.py 100.tsp.sol
  
 ### Example 2:
    python3 gen_tsp.py 1000 > 1000.tsp
    time sh hess_hpc.sh 1000.tsp 4 0

    s OPTIMAL
    v 258.356912058405726

    real    0m16.168s
    user    0m56.844s
    sys     0m0.395s

    python3 plot_tsp.py 1000.tsp.sol
    
[<img src="https://github.com/maxtuno/HESS_HPC/blob/main/bin/1000.tsp.sol_tour.png">](https://twitter.com/maxtuno)
  
