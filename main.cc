/*
MIT License

Copyright (c) 2022 Oscar Riveros

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <mpi.h>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <numeric>
#include <random>
#include <algorithm>

bool logs = false;
typedef std::size_t integer;
std::map<integer, bool> global_db;
std::map<integer, bool> local_db;

uint64_t hashing(const int &i, const int &j, const std::vector<int> &sequence) {
    uint64_t hash = 0;
    hash ^= i;
    for (auto &k: sequence) {
        hash ^= k + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    hash ^= j;
    return hash;
}

void invert(int i, int j, std::vector<int> &sequence) {
    while (i < j) {
        std::swap(sequence[i], sequence[j]);
        i++;
        j--;
    }
}

bool next_orbit(std::vector<int> &sequence) {
    integer key;
    for (auto i{0}; i < sequence.size(); i++) {
        for (auto j{0}; j < sequence.size(); j++) {
            key = hashing(i, j, sequence);
            if (!global_db[key]) {
                global_db[key] = true;
                goto finally;
            } else {
                invert(std::min(i, j), std::max(i, j), sequence);
            }
        }
    }
    return false;
    finally:
    return true;
}

double oracle(std::vector<int> &sequence, const std::vector<std::vector<double>> &data, const double &global) {
    int n = static_cast<int>(sequence.size());
    double local{0};
    for (auto i{0}; i < n; i++) {
        local += std::sqrt(std::pow(data[sequence[(i + 1) % n]][0] - data[sequence[i]][0], 2) + std::pow(data[sequence[(i + 1) % n]][1] - data[sequence[i]][1], 2));
        if (local > global) {
            return local;
        }
    }
    return local;
}

std::vector<int> hess(const std::vector<std::vector<double>> &data, const int &world_rank, const int &world_size) {
    double local, global{std::numeric_limits<double>::max()};
    int n = static_cast<int>(data.size()), flag, finished;
    std::random_device device;
    std::default_random_engine engine(device());
    std::vector<int> sequence(n);
    std::iota(sequence.begin(), sequence.end(), 0);
    std::shuffle(sequence.begin(), sequence.end(), engine);
    std::vector<int> optimal(n);
    MPI_Status status;
    MPI_Request request[world_size - 1];
    std::cout.precision(std::numeric_limits<double>::max_digits10 + 1);
    if (world_rank == 0) {
        finished = 0;
        for (;;) {
            flag = 0;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                MPI_Recv(sequence.data(), sequence.size(), MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                if (sequence.front() == sequence.back()) {
                    finished++;
                    if (finished == world_size - 1) {
                        return optimal;
                    }
                } else {
                    local = oracle(sequence, data, global);
                    if (local < global) {
                        global = local;
                        if (logs) {
                            std::cout << "c " << global << std::endl;
                        }
                        optimal.assign(sequence.begin(), sequence.end());
                        for (int rank{1}, idx = 0; rank < world_size; rank++) {
                            MPI_Isend(sequence.data(), sequence.size(), MPI_INT, rank, 1, MPI_COMM_WORLD, &request[idx++]);
                        }
                    }
                }
            }
        }
    } else {
        for (;;) {
            auto done = true;
            for (auto i{0}; i < n - 1; i++) {
                for (auto j{i + 1}; j < n; j++) {
                    flag = 0;
                    MPI_Iprobe(0, 1, MPI_COMM_WORLD, &flag, &status);
                    if (flag) {
                        MPI_Recv(sequence.data(), sequence.size(), MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                    }
                    auto key = hashing(i, j, sequence);
                    if (local_db[key]) {
                        continue;
                    } else {
                        local_db[key] = true;
                    }
                    invert(std::min(i, j), std::max(i, j), sequence);
                    local = oracle(sequence, data, global);
                    if (local < global) {
                        global = local;
                        MPI_Send(sequence.data(), sequence.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
                        done = false;
                    } else if (local > global) {
                        invert(std::min(i, j), std::max(i, j), sequence);
                    } else {
                        local_db.clear();
                    }
                }
            }
            if (done && next_orbit(sequence)) {
                break;
            }
        }
    }
    std::fill(sequence.begin(), sequence.end(), 0);
    MPI_Send(sequence.data(), sequence.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
    return sequence;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int n{0};
    double x{0}, y{0};
    std::string dummy;
    std::vector<std::vector<double>> data;

    std::ifstream file(argv[1]);
    file >> n;
    data.resize(n);
    for (auto i{0}; i < n; i++) {
        file >> dummy;
        file >> x;
        file >> y;
        data[i].emplace_back(x);
        data[i].emplace_back(y);
    }
    file.close();

    logs = std::atoi(argv[2]) == 1;

    auto optimal = hess(data, world_rank, world_size);

    if (world_rank == 0) {
        std::cout << "s " << "OPTIMAL" << "\n" << "v " << oracle(optimal, data, std::numeric_limits<double>::max()) << std::endl;

        std::ofstream solution(std::string(argv[1]) + ".sol");
        for (auto i{0}; i < n; i++) {
            solution << data[optimal[i]][0] << " " << data[optimal[i]][1] << std::endl;
        }
        solution.close();
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
