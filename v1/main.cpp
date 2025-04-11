/*
 * main.cpp
 * Group Prllz
 * March 2025
 *
 */

#include "defs.h"

int main(int argc, char **argv) {
        MPI_Init(&argc, &argv);
        MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

        double start_time = MPI_Wtime(); 

        int mpi_rank, mpi_sz;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &mpi_sz);

        config_t config { }; 
        if (argc != 10) {
                fprintf(stderr, "Usage: 9 args are required.\n");
                return 0;
        }

        config.input_file = argv[1];
        config.px = atoi(argv[2]);
        config.py = atoi(argv[3]);
        config.pz = atoi(argv[4]);
        config.nx = atoi(argv[5]);
        config.ny = atoi(argv[6]);
        config.nz = atoi(argv[7]);
        config.nstep = atoi(argv[8]);
        config.output_file = argv[9];

        assert(mpi_sz == config.px * config.py * config.pz);
        assert(config.nx % config.px == 0);
        assert(config.ny % config.py == 0);
        assert(config.nz % config.pz == 0);

        // all sub-domains have equal sizes. bound stores the size
        Point bound { config.nx / config.px, config.ny / config.py,
               config.nz / config.pz }; 

        auto is_corner {
                [&bound, &config](int x, int y, int z) -> bool {
                        int ret = x == (bound[0] - 1);
                        ret &= y == (bound[1] - 1);
                        ret &= z == (bound[2] - 1);
                        return ret;
                }
        };

        Block<float> data(bound, config.nstep); // this rank's sub-domain

        std::vector rank_assgn(config.px, std::vector(config.py, std::vector<int>(config.pz)));
        {
                int rnk = 0;
                for (int z = 0; z < config.pz; z++) for (int y = 0; y < config.py; y++)
                       for (int x = 0; x < config.px; x++) {
                              rank_assgn[x][y][z] = rnk;
                              rnk = (rnk + 1) % mpi_sz;
                       }
        }

        /*
         * Data I/O
         */
        if (mpi_rank == 0) {
                // sender
                FILE *fptr = fopen(config.input_file, "r");
                assert(fptr != NULL);

                std::vector<Block<float>> data_to_send(mpi_sz, Block<float>(bound, config.nstep));

                MPI_Request requests[mpi_sz];
                for (int i = 0; i < mpi_sz; i++) requests[i] = MPI_REQUEST_NULL;
                
                // TODO: can probably be optimized a lot
                for (int z = 0; z < config.nz; z++) for (int y = 0; y < config.ny; y++)
                        for (int x = 0; x < config.nx; x++) {
                                int _x = x % bound[0];
                                int _y = y % bound[1];
                                int _z = z % bound[2];

                                int rnk = rank_assgn[x / bound[0]][y / bound[1]][z / bound[2]];
                                for (int t = 0; t < config.nstep; t++) {
                                        float val; [[maybe_unused]] int _tmp; 
                                        _tmp = fscanf(fptr, "%f", &val);
                                        passert(_tmp == 1);

                                        if (rnk)
                                                data_to_send[rnk](t, _x, _y, _z) = val;
                                        else
                                                data(t, _x, _y, _z) = val;
                                }

                                if (is_corner(_x, _y, _z)) {
                                        if (rnk != 0) {
                                                MPI_Wait(&requests[rnk], MPI_STATUS_IGNORE);
                                                MPI_Isend(&data_to_send[rnk].data[0],
                                                                data_to_send[rnk].block_sz * config.nstep,
                                                                MPI_FLOAT,
                                                                rnk,
                                                                rnk,
                                                                MPI_COMM_WORLD,
                                                                &requests[rnk]);
                                        } 
                                }
                        }

                MPI_Waitall(mpi_sz - 1, &requests[1], MPI_STATUSES_IGNORE); 
        } else {
               // receive the data 
               MPI_Recv(&data.data[0], data.block_sz * config.nstep,
                               MPI_FLOAT, 0, mpi_rank, MPI_COMM_WORLD, 
                               MPI_STATUS_IGNORE);
        }

        // convention: x -1, y -1, z -1, x +1, y +1, z +1
        std::vector<int> neighbours(6, MPI_PROC_NULL);
        for (int z = 0; z < config.pz; z++) {
                bool _tmp = false;
                for (int y = 0; y < config.py; y++) {
                for (int x = 0; x < config.px; x++) {
                        if (rank_assgn[x][y][z] != mpi_rank) continue;

                        if (x) neighbours[0] = rank_assgn[x - 1][y][z];
                        if (y) neighbours[1] = rank_assgn[x][y - 1][z];
                        if (z) neighbours[2] = rank_assgn[x][y][z - 1];
                        if (x < config.px - 1) neighbours[3] = rank_assgn[x + 1][y][z];
                        if (y < config.py - 1) neighbours[4] = rank_assgn[x][y + 1][z];
                        if (z < config.pz - 1) neighbours[5] = rank_assgn[x][y][z + 1];

                        _tmp = true;
                        break;
                }
                if (_tmp) break;
                }
                if (_tmp) break;
        }

        double read_time = MPI_Wtime();

        Halo<float> halo { data, neighbours, mpi_rank, bound, config.nstep };
        halo.recv();

        // we perform computations on our local sub-domain while the recv's
        // proceed asynchronously
        answer_t<float> ans(config.nstep);

        auto gen_neighs {
                [](int x, int y, int z) -> auto {
                        std::vector<std::array<int, 3>> neighs = {
                                {x - 1, y, z},
                                {x + 1, y, z},
                                {x, y - 1, z},
                                {x, y + 1, z},
                                {x, y, z - 1},
                                {x, y, z + 1}};
                        // do we need move semantics here?
                        // ig the copy might be elided, idk
                        // update: yup, we don't need move semantics
                        // according to the compiler warning it indeed stops
                        // copy elision
                        //return std::move(neighs);
                        return neighs;
                }
        };


        for (int x = 1; x < bound[0] - 1; x++) for (int y = 1; y < bound[1] - 1; y++)
                for (int z = 1; z < bound[2] - 1; z++) for (int t = 0; t < config.nstep; t++) {
                        float val = data(t, x, y, z);
                        ans.gmin[t] = std::min(ans.gmin[t], val);
                        ans.gmax[t] = std::max(ans.gmax[t], val);

                        std::vector<std::array<int, 3>> neighs { gen_neighs(x, y, z) };

                        bool lmin = true, lmax = true;
                        for (auto &ng: neighs) {
                                float v = data(t, ng[0], ng[1], ng[2]);
                                //assert(fabs(v - val) > 0.001);
                                if (v > val - EPS) lmax = false;
                                if (v < val + EPS) lmin = false;
                        }

                        ans.cnt_min[t] += static_cast<int>(lmin);
                        ans.cnt_max[t] += static_cast<int>(lmax);
                }

        halo.wait();

        static const Point origin { 0, 0, 0 };

        auto halo_process { 
                [&bound, &data, &halo, &ans, &gen_neighs, &neighbours]
                        (int t, int x, int y, int z) -> void {
                        float val = data(t, x, y, z);
                        ans.gmin[t] = std::min(ans.gmin[t], val);
                        ans.gmax[t] = std::max(ans.gmax[t], val);

                        std::vector<std::array<int, 3>> neighs { gen_neighs(x, y, z) };

                        std::vector<std::array<int, 3>> to_erase;
                        for (int i = 0; i < 6; i++) {
                                if (neighbours[i] == MPI_PROC_NULL) {
                                        if (i == 0 && x == 0) to_erase.push_back({x - 1, y, z});
                                        if (i == 1 && y == 0) to_erase.push_back({x, y - 1, z});
                                        if (i == 2 && z == 0) to_erase.push_back({x, y, z - 1});
                                        if (i == 3 && x == bound[0] - 1) to_erase.push_back({x + 1, y, z});
                                        if (i == 4 && y == bound[1] - 1) to_erase.push_back({x, y + 1, z});
                                        if (i == 5 && z == bound[2] - 1) to_erase.push_back({x, y, z + 1});
                                }
                        }

                        for (auto &te: to_erase) 
                               neighs.erase(std::find(neighs.begin(), neighs.end(), te)); 

                        bool lmin = true, lmax = true;
                        for (auto &ng: neighs) {
                                float v;
                                Point p_ng { ng[0], ng[1], ng[2] };

                                if (p_ng < bound && p_ng >= origin) 
                                        v = data(t, ng[0], ng[1], ng[2]);
                                else
                                        v = halo(t, ng[0], ng[1], ng[2]);

                                if (v > val - EPS) lmax = false;
                                if (v < val + EPS) lmin = false;
                        }
                        
                        ans.cnt_min[t] += static_cast<int>(lmin);
                        ans.cnt_max[t] += static_cast<int>(lmax);
                }
        };

        // x = 0, x = bound[0] - 1
        for (int y = 0; y < bound[1]; y++) for (int z = 0; z < bound[2]; z++) 
                for (int t = 0; t < config.nstep; t++) {
                        halo_process(t, 0, y, z);
                        if (bound[0] - 1)
                                halo_process(t, bound[0] - 1, y, z);
                }

        // y = 0, y = bound[1] - 1
        for (int x = 1; x < bound[0] - 1; x++) for (int z = 0; z < bound[2]; z++)
                for (int t = 0; t < config.nstep; t++) {
                        halo_process(t, x, 0, z);
                        if (bound[1] - 1)
                                halo_process(t, x, bound[1] - 1, z);
                }

        // z = 0, z = bound[2] - 1
        for (int x = 1; x < bound[0] - 1; x++) for (int y = 1; y < bound[1] - 1; y++)
                for (int t = 0; t < config.nstep; t++) {
                        halo_process(t, x, y, 0);
                        if (bound[2] - 1)
                                halo_process(t, x, y, bound[2] - 1);
                }

        double out_time = MPI_Wtime();

        ans.times[0] = read_time - start_time;
        ans.times[1] = out_time - read_time;
        ans.times[2] = out_time - start_time;

        answer_t<float> reduced_ans { config.nstep };

        MPI_Reduce(&ans.cnt_min[0], &reduced_ans.cnt_min[0], config.nstep, MPI_INT,
                        MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&ans.cnt_max[0], &reduced_ans.cnt_max[0], config.nstep, MPI_INT,
                        MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&ans.gmin[0], &reduced_ans.gmin[0], config.nstep, MPI_FLOAT,
                       MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&ans.gmax[0], &reduced_ans.gmax[0], config.nstep, MPI_FLOAT,
                        MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&ans.times, &reduced_ans.times[0], 3, MPI_DOUBLE,
                        MPI_MAX, 0, MPI_COMM_WORLD);

        halo.free();

        // TODO: mpi free?
        if (mpi_rank == 0) {
                FILE *fptr = fopen(config.output_file, "w");

                for (int t = 0; t < config.nstep; t++) 
                        fprintf(fptr, "(%d, %d) ", reduced_ans.cnt_min[t], reduced_ans.cnt_max[t]);
                fprintf(fptr, "\n");
                
                for (int t = 0; t < config.nstep; t++)
                        fprintf(fptr, "(%f, %f) ", reduced_ans.gmin[t], reduced_ans.gmax[t]);
                fprintf(fptr, "\n");

                fprintf(fptr, "%lf %lf %lf\n", reduced_ans.times[0], reduced_ans.times[1],
                                reduced_ans.times[2]);

                fclose(fptr);
        }

        MPI_Finalize();

        return 0;
}



