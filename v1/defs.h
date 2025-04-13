/*
 * defs.h
 * Group Prllz
 *
 * March 2025
 */

#ifndef _DEFS_H
#define _DEFS_H

#include <array>
#include <algorithm>
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
//#define DEBUG

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
                        int _rank, Point _bound, int _steps); 

        void recv();
        void wait();
        void free(); 

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
