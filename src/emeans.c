/*
 * Evolutionary K-means clustering (E-means) using Genetic Algorithms.
 *
 * Copyright (C) 2015, Jonathan Gillett
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <confuse.h>
#include <unistd.h>
#include <gsl/gsl_matrix.h>
#include "utility.h"
#include "pcg_basic.h"
#include "io.h"
#include "cluster.h"
#include "fitness.h"
#include "operators.h"


// Define process 0 as MASTER
#define MASTER 0

int DEBUG, VERBOSE, SLAVE;

// Define the configuration parameters
int64_t n_clusters = 3,
        trials = 1,
        size = 100;
double  m_rate = 0.01,
        c_rate = 0.70;
int64_t max_iter = 10000,
        data_rows = 0,
        data_cols = 0;
char    *data_file = NULL,
        *chrom_file = NULL,
        *fitness_file = NULL,
        *cluster_file = NULL;

// The configuration file parsing mappings
cfg_opt_t opts[] = {
    CFG_SIMPLE_INT("n_clusters", &n_clusters),
    CFG_SIMPLE_INT("trials", &trials),
    CFG_SIMPLE_INT("size", &size),
    CFG_SIMPLE_FLOAT("m_rate", &m_rate),
    CFG_SIMPLE_FLOAT("c_rate", &c_rate),
    CFG_SIMPLE_INT("max_iter", &max_iter),
    CFG_SIMPLE_INT("data_rows", &data_rows),
    CFG_SIMPLE_INT("data_cols", &data_cols),
    CFG_SIMPLE_STR("data_file", &data_file),
    CFG_SIMPLE_STR("chrom_file", &chrom_file),
    CFG_SIMPLE_STR("fitness_file", &fitness_file),
    CFG_SIMPLE_STR("cluster_file", &cluster_file),
    CFG_END()
};
cfg_t *cfg;


/**
 * The master, initializes the genetic algorithm, performs GA operators.
 * 
 * @param nproc Number of MPI processes
 *
 * @return      Status code, 0 for SUCCESS, 1 for ERROR
 */
int master(int nproc)
{
    int status = SUCCESS;
    MPI_Status mpi_status;

    sleep(5);
    return status;
}


/**
 * The slave, performs Lloyds clustering algorith, computes the fitness,
 * and returns results to master.
 * 
 * @param proc_id Process ID of the slave, used as an identifier
 *
 * @return        Status code, 0 for SUCCESS, 1 for ERROR
 */
int slave(int proc_id)
{
    gsl_matrix *data = NULL,
               *bounds = NULL,
               **clusters = NULL;
    // Set global SLAVE identifier
    SLAVE = proc_id;
    int status = SUCCESS;
    MPI_Status mpi_status;

    // Initialize the PRNG
    pcg32_random_t rng;
    int rounds = 5;
    pcg32_srandom_r(&rng, time(NULL) ^ (intptr_t)&printf, (intptr_t)&rounds);

    // Load the data
    if ((data = gsl_matrix_alloc(data_rows, data_cols)) == NULL)
    {
        fprintf(stderr, RED "[ MASTER ]  Error allocating data matrix!" RESET);
        status = ERROR;
        goto free;
    }
    if ((status = load_data(data_file, data)) != SUCCESS)
    {   
        fprintf(stderr, RED "[ MASTER ]  Unable to load data!\n" RESET);
        status = ERROR;
        goto free;
    }
    if ((clusters = (gsl_matrix **)calloc(n_clusters, sizeof(gsl_matrix **))) == NULL)
    {
        fprintf(stderr, RED "[ MASTER ]  Error allocating clusters!\n" RESET);
        status = ERROR;
        goto free;
    }
    if ((bounds = gsl_matrix_alloc(data_cols, 2)) == NULL)
    {
        fprintf(stderr, RED "[ MASTER ]  Error allocating bounds matrix!" RESET);
        status = ERROR;
        goto free;
    }

    gsl_matrix *centroids = gsl_matrix_alloc(n_clusters, data_cols);
    calc_bounds(data, bounds);
    random_centroids(centroids, bounds, &rng);
    // Perform the first GA step, optimizing
    //lloyd_random(trials, data, 3, clusters, &rng);
    lloyd_defined(trials, centroids, data, n_clusters, clusters);
    dunn_index(centroids, n_clusters, clusters);

    gsl_matrix *parent1 = gsl_matrix_alloc(n_clusters, data_cols),
               *parent2 = gsl_matrix_alloc(n_clusters, data_cols);
    random_centroids(parent1, bounds, &rng);
    random_centroids(parent2, bounds, &rng);
    crossover(parent1, parent2, &rng);
    mutate(parent1, bounds, &rng);
    mutate(parent2, bounds, &rng);

free:
    gsl_matrix_free(data);
    return status;
}


int main(int argc, char *argv[])
{
    int proc_id;      // Process ID number
    int n_proc;       // Number of processes
    int status = SUCCESS;
    char conf_file[100] = "./conf/emeans.conf";

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);

    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, RED "[ MASTER ]  Incorrect parameters!\n" RESET);
        fprintf(stderr, RED "[ MASTER ]  Correct usage:\n" RESET);
        fprintf(stderr, RED "[ MASTER ]  %s <DEBUG> (1..N=DEBUG 0=NODEBUG) <VERBOSE> (1=YES 0=NO) <CONFIG> (DEFAULT ./conf/emeans.conf)\n\n" RESET, argv[0]);
        status = ERROR;
        goto free;
    }
    DEBUG = atoi(argv[1]);
    VERBOSE = atoi(argv[2]);

    if (argc > 3)
    {
        strcpy(conf_file, argv[3]);
    }

    if (access(conf_file, F_OK) != -1)
    {
        cfg = cfg_init(opts, 0);
        cfg_parse(cfg, conf_file);
    }
    else
    {
        fprintf(stderr, RED "[ MASTER ]  Unable to open file %s\n" RESET, conf_file);
        status = ERROR;
        goto free;
    }

    if (DEBUG == DEBUG_CONFIG)
    {
        printf(YELLOW "[ MASTER ]  DISPLAY CONTENTS OF CONFIG FILE" RESET);
        printf(YELLOW "\n============================================================\n" RESET);
        printf(YELLOW "= CONFIG FILE PARAMS\n" RESET);
        printf(YELLOW "============================================================\n" RESET);
        printf(YELLOW "   NUM CLUSTERS: %10ld\n" RESET, n_clusters);
        printf(YELLOW "CENTROID TRIALS: %10ld\n" RESET, trials);
        printf(YELLOW "POPULATION SIZE: %10ld\n" RESET, size);
        printf(YELLOW "  MUTATION RATE: %10.6f\n" RESET, m_rate);
        printf(YELLOW " CROSSOVER RATE: %10.6f\n" RESET, c_rate);
        printf(YELLOW " MAX ITERATIONS: %10ld\n" RESET, max_iter);
        printf(YELLOW "      DATA ROWS: %10ld\n" RESET, data_rows);
        printf(YELLOW "      DATA COLS: %10ld\n" RESET, data_cols);
        printf(YELLOW "      DATA FILE: %s\n" RESET, data_file);
        printf(YELLOW "CHROMOSOME FILE: %s\n" RESET, chrom_file);
        printf(YELLOW "   FITNESS FILE: %s\n" RESET, fitness_file);
        printf(YELLOW "   CLUSTER FILE: %s\n" RESET, cluster_file);
        goto free;
    }

    if (proc_id == MASTER)
    {
        status = master(n_proc);
    }
    else
    {
        status = slave(proc_id);
    }

// Free memory and exit
free:
    cfg_free(cfg);
    free(data_file);
    free(chrom_file);
    free(fitness_file);
    free(cluster_file);
    
    if (status == SUCCESS)
    {
        MPI_Finalize();
    }
    else
    {
        MPI_Abort(MPI_COMM_WORLD, status);
    }
    exit(status);
}