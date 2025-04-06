/*
 * defs.h
 * Group Prllz
 *
 * March 2025
 */

#ifndef _DEFS_H
#define _DEFS_H

#include <cassert>
#include <iostream>

#include "mpi.h"

// comment this for timing runs
// will disable passert calls
#define DEBUG

// config parameters

const int MAX_MSG_SIZE = 1024 * 1024 * 4; // 4 MB 

// predicated assert
#ifdef DEBUG
#define passert(x) assert(x)
#else
#define passert(x)
#endif

// representation of a generic point in 3 dimensions with bounds check
template<typename T>
struct Point {
        T data[3];

        T& operator[] (const T idx) { 
                passert(idx >= 0 && idx < 3);
                passert(data[idx] <= 1024);

                return data[idx]; 
        }

        const T& operator[] (const T idx) const {
                passert(idx >= 0 && idx < 3);
                passert(data[idx] <= 1024);

                return data[idx];
        }

        // a quickie to return the product of the 3 dimensions
        // careful about overflow, if T is int
        T operator! () const {
                return data[0] * data[1] * data[2];
        }
};

using Point = struct Point<int>;

/*
 * A "block" of data. Represented by two corner points - the lower one is (0, 0, 0),
 * and the higher one is "bound".
 */
template<typename T>
class Block final {
private:
        Point bound;
        int steps; // no. of time steps
        const int block_sz;
public:
        std::unique_ptr<T[]> data;

        Block(Point _bound, int _steps) : bound { p },
                steps { _steps },
                block_sz { !p },
                data { std::make_unique<T[]>((!p) * _steps) } 
        {
                // TODO: add a passert if !p * _steps is overflowing an int
        }

        // Note: the operator is (x, y, z) unlike your usual array subscripting [z][y][x]
        __attribute__((always_inline)) T& operator() (int t, int x, int y, int z) {
                passert(x < bound[0] && y < bound[1] && z < bound[2]);
                return data[t * block_sz + 
                        z * bound[0] * bound[1] + y * bound[0] + x];
        }

        __attribute__((always_inline)) T operator() (int t, int x, int y, int z) const {
                passert(x < bound[0] && y < bound[1] && z < bound[2]);
                return data[t * block_sz + 
                        z * bound[0] * bound[1] + y * bound[0] + x];
        }
};

typedef struct _config_t {
        int px, py, pz;
        int nx, ny, nz;
        int nstep; // no. of time steps
        
        const char* input_file, output_file;
} config_t;

#endif // _DEFS_H
