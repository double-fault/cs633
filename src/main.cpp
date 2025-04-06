/*
 * main.cpp
 * Group Prllz
 * March 2025
 *
 */

#include "defs.h"

int main(int argc, char **argv) {
        MPI_Init(&argc, &argv);

        int mpi_rank, mpi_sz;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &mpi_sz);

        config_t config; // TODO: setup constructor
        assert(mpi_sz == config.px * config.py * config.pz);

        // all sub-domains have equal sizes. bound stores the size
        Point bound { config.nx / config.px, config.ny / config.py,
               config.nz / config.pz }; 

        // max msg sizes are bounded by MAX_MSG_SIZE
        // this also bounds our sub-domain size
        const int max_z = (MAX_MSG_SIZE) / (bound[0] * bound[1]);
        bound[2] = std::min(bound[2], max_z);

        auto is_corner {
                [&bound, &config](int x, int y, int z) -> bool {
                        x %= bound[0];
                        y %= bound[1];
                        int ret = x == (bound[0] - 1);
                        ret &= y == (bound[1] - 1);

                        if (z == config.nz - 1) return ret;

                        z %= bound[2];
                        ret &= (z == bound[2] - 1);
                        return ret;
                }
        };

        const int num_chunks_z = (config.nz + bound[2] - 1) / bound[2];
        vector rank_assign(vector<vector<vector>>(num_chunks_z, 
                                vector<vector> 
        vector rank_assgn[config.px][config.py][num_chunks_z];
        {
                int rnk = 1;
                for (int z = 0; z < num_chunks_z; z++) for (int y = 0; y < config.py; y++)
                       for (int x = 0; x < config.px; x++) {
                              rank_assgn[x][y][z] = rnk;
                              rnk++;
                              if (rnk == mpi_sz) rnk = 1;
                       }
        }

        // Data distribution from rank 0
        if (mpi_rank == 0) {
                // sender
                FILE *fptr = fopen(config.input_file, "r");
                assert(fptr != NULL);

                Block<float> data_to_send[mpi_sz];
                int tags[mpi_sz]; memset(tags, 0, sizeof tags);

                MPI_Request requests[mpi_sz];
                for (int i = 0; i < mpi_sz; i++) requests[i] = MPI_REQUEST_NULL;
                
                // TODO: can probably be optimized a lot
                int rnk = 1;
                for (int z = 0; z < config.nz; z++) for (y = 0; y < config.ny; y++)
                        for (x = 0; x < config.nx; x++) {
                                int _x = x % bound[0];
                                int _y = y % bound[1];
                                int _z = z % bound[2];
                                for (t = 0; t < config.nstep; t++) {
                                        float val; fscanf(fptr, "%f", &val);
                                        data_to_send[rnk](t, _x, _y, _z) = val;
                                }

                                if (is_corner(x, y, z)) {
                                        MPI_Wait(&requests[rnk], MPI_STATUS_IGNORE);
                                        MPI_Isend(data_to_send[rnk].data,
                                                        data_to_send[rnk].block_sz,
                                                        MPI_FLOAT,
                                                        rnk,
                                                        tags[rnk]++,
                                                        MPI_COMM_WORLD,
                                                        &requests[rnk]);

                                        rnk++;
                                        if (rnk == mpi_sz) rnk = 1;
                                }
                        }
        } 

        for (int z = 0; z < num_chunks_z; z++)


