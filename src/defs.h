/*
 * defs.h
 * Group Prllz
 *
 * March 2025
 */

#ifndef _DEFS_H
#define _DEFS_H

#include <cassert>
#include <cmath>
#include <vector>
#include <limits>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "mpi.h"

// comment this for timing runs
// will disable passert calls
#define DEBUG

// config parameters

const int MAX_MSG_SIZE = 1024 * 1024 * 4; // 4 MB 

#define MAGIC 333

#define EPS 0.0001

// predicated assert
#ifdef DEBUG
#define passert(x) assert(x)
#else
#define passert(x)
#endif

// representation of a generic point in 3 dimensions with bounds check
template<typename T>
struct _Point {
        T data[3];

        __attribute__((always_inline)) T& operator[] (const T idx) { 
                passert(idx >= 0 && idx < 3);
                passert(data[idx] <= 1024);

                return data[idx]; 
        }

        __attribute__((always_inline)) const T& operator[] (const T idx) const {
                passert(idx >= 0 && idx < 3);
                passert(data[idx] <= 1024);

                return data[idx];
        }

        // a quickie to return the product of the 3 dimensions
        // careful about overflow, if T is int
        T operator! () const {
                return data[0] * data[1] * data[2];
        }

        friend bool operator< (const struct _Point &a, const struct _Point &b) {
               return a[0] < b[0] && a[1] < b[1] && a[2] < b[2];
        }

        friend bool operator>= (const struct _Point &a, const struct _Point &b) {
                return a[0] >= b[0] && a[1] >= b[1] && a[2] >= b[2];
        }
};

using Point = struct _Point<int>;

/*
 * A "block" of data. Represented by two corner points - the lower one is (0, 0, 0),
 * and the higher one is "bound".
 */
template<typename T>
class Block final {
private:
        Point bound;
        int steps; // no. of time steps
public:
        const int block_sz;
        std::vector<T> data;

        Block(Point _bound, int _steps) : bound { _bound },
                steps { _steps },
                block_sz { !_bound },
                data ( (!_bound) * _steps , 0)
        {
                // TODO: add a passert if !p * _steps is overflowing an int
        }

        // Note: the operator is (x, y, z) unlike your usual array subscripting [z][y][x]
        __attribute__((always_inline)) T& operator() (int t, int x, int y, int z) {
                passert(x < bound[0] && y < bound[1] && z < bound[2]);
                return data[z * bound[0] * bound[1] * steps
                        + y * bound[0] * steps + x * steps + t];
        }

        __attribute__((always_inline)) T operator() (int t, int x, int y, int z) const {
                passert(x < bound[0] && y < bound[1] && z < bound[2]);
                return data[z * bound[0] * bound[1] * steps
                        + y * bound[0] * steps + x * steps + t];
        }
};

template<typename T>
class Block2D final {
private:
        int sx, sy, steps;
public:
        Block<T> block { };
        const int block_sz;

        Block2D(int _x, int _y, int _steps) : sx { _x },
                sy { _y },
                steps { _steps },
                block { Point { _x, _y, 1}, _steps },
                block_sz { _x * _y }
        {
        }

        __attribute__((always_inline)) T& operator() (int t, int x, int y) {
                return block(t, x, y, 0);
        }

        __attribute__((always_inline)) T operator() (int t, int x, int y) const {
                return block(t, x, y, 0);
        }
};

template<typename T>
class Halo final {
private:
        Block<float> &data;
        
        MPI_Datatype halo_xy, halo_yz, halo_zx;

        std::vector<int> neighbours;
        Point bound;
        int steps;
        MPI_Request requests[6];
        int my_rank;
public:
        // does making halo_recv public make it easier for the compiler to inline
        // the operators?
        // idts but yeah who knows
        std::vector<Block2D<T>> halo_recv;

        Halo(Block<T> _data, std::vector<int> _neighbours,
                        int _rank, Point _bound, int _steps) : 
                data { _data },
                neighbours { _neighbours },
                bound { _bound },
                steps { _steps },
                my_rank { _rank }
        {
                // halo exchange
                // first we perform non-blocking sends on the data
                // xy, yz, zx refers to the planes we are going to send
                MPI_Type_vector(bound[1] * bound[2], steps,
                                bound[0] * steps, MPI_FLOAT, &halo_yz);
                MPI_Type_vector(bound[0] * bound[1], steps,
                                steps, MPI_FLOAT, &halo_xy);
                MPI_Type_vector(bound[2], steps * bound[0],
                                bound[1] * bound[0] * steps, MPI_FLOAT, &halo_zx);
                MPI_Type_commit(&halo_xy);
                MPI_Type_commit(&halo_yz);
                MPI_Type_commit(&halo_zx);

                MPI_Request _rst;
                MPI_Isend(&data(0, 0, 0, 0), 1, halo_yz, neighbours[0],
                                neighbours[0] + MAGIC, MPI_COMM_WORLD, &_rst); 
                MPI_Isend(&data(0, 0, 0, 0), 1, halo_zx, neighbours[1],
                                neighbours[1] + MAGIC, MPI_COMM_WORLD, &_rst);
                MPI_Isend(&data(0, 0, 0, 0), 1, halo_xy, neighbours[2],
                                neighbours[2] + MAGIC, MPI_COMM_WORLD, &_rst); 

                MPI_Isend(&data(0, bound[0] - 1, 0, 0), 1, halo_yz, neighbours[3],
                                neighbours[3] + MAGIC, MPI_COMM_WORLD, &_rst);
                MPI_Isend(&data(0, 0, bound[1] - 1, 0), 1, halo_zx, neighbours[4],
                                neighbours[4] + MAGIC, MPI_COMM_WORLD, &_rst);
                MPI_Isend(&data(0, 0, 0, bound[2] - 1), 1, halo_xy, neighbours[5],
                                neighbours[5] + MAGIC, MPI_COMM_WORLD, &_rst);

                // convention: x -1, y -1, z -1, x +1, y +1, z +1
                halo_recv.push_back(std::move(Block2D<T>(bound[1], bound[2], steps)));
                halo_recv.push_back(std::move(Block2D<T>(bound[0], bound[2], steps)));
                halo_recv.push_back(std::move(Block2D<T>(bound[0], bound[1], steps)));
                halo_recv.push_back(std::move(Block2D<T>(bound[1], bound[2], steps)));
                halo_recv.push_back(std::move(Block2D<T>(bound[0], bound[2], steps)));
                halo_recv.push_back(std::move(Block2D<T>(bound[0], bound[1], steps)));

                for (int i = 0; i < 6; i++) requests[i] = MPI_REQUEST_NULL; 
        }

        void recv() {
                for (int i = 0; i < 6; i++) {
                        if (neighbours[i] != MPI_PROC_NULL) {
                                MPI_Irecv(&halo_recv[i].block.data[0], halo_recv[i].block_sz * steps, 
                                                 MPI_FLOAT,
                                               neighbours[i], my_rank + MAGIC,
                                              MPI_COMM_WORLD, &requests[i]);
                        }
                } 
        }

        void wait() {
                // wait till the halo exchange data has been received
                MPI_Waitall(6, requests, MPI_STATUSES_IGNORE);
        }

        __attribute__((always_inline)) T& operator() (int t, int x, int y, int z) {
                if (x < 0) {
                        return halo_recv[0](t, y, z);
                } else if (y < 0) {
                        return halo_recv[1](t, x, z);
                } else if (z < 0) {
                        return halo_recv[2](t, x, y);
                } else if (x == bound[0]) {
                        return halo_recv[3](t, y, z);
                } else if (y == bound[1]) {
                        return halo_recv[4](t, x, z);
                } 
                passert(z == bound[2]);
                return halo_recv[5](t, x, y);
        }

        void free() 
        {
                MPI_Type_free(&halo_xy);
                MPI_Type_free(&halo_yz);
                MPI_Type_free(&halo_zx);
        }

        // instead of going through the headache of redefining the operator for const
        // objects, we can simply delete it since const Halo objects shouldn't exist
        T operator() (int t, int x, int y, int z) const = delete;
};

typedef struct _config_t {
        int px, py, pz;
        int nx, ny, nz;
        int nstep; // no. of time steps
        
        const char* input_file;
        const char* output_file;
} config_t;

template<typename T>
struct answer_t {
        std::vector<int> cnt_min, cnt_max;
        std::vector<T> gmin, gmax;

        std::array<double, 3> times;

        answer_t(int nsteps) :
                cnt_min(nsteps, 0), cnt_max(nsteps, 0),
                gmin(nsteps, std::numeric_limits<T>::max()), 
                gmax(nsteps, std::numeric_limits<T>::min())
        {
        }
};

#endif // _DEFS_H
