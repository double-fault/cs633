/*
 * halo.cpp
 * Group Prllz
 *
 * April 2025
 */

#include "defs.h"

template <typename T>
Halo<T>::Halo(Block<T> _data, std::vector<int> _neighbours,
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

template <typename T>
void Halo<T>::recv() {
        for (int i = 0; i < 6; i++) {
                if (neighbours[i] != MPI_PROC_NULL) {
                        MPI_Irecv(&halo_recv[i].block.data[0], halo_recv[i].block_sz * steps, 
                                         MPI_FLOAT,
                                       neighbours[i], my_rank + MAGIC,
                                      MPI_COMM_WORLD, &requests[i]);
                }
        } 
}

template <typename T>
void Halo<T>::wait() {
        MPI_Waitall(6, requests, MPI_STATUSES_IGNORE);
}

template <typename T>
void Halo<T>::free()
{
        MPI_Type_free(&halo_xy);
        MPI_Type_free(&halo_yz);
        MPI_Type_free(&halo_zx);
}

        
        
