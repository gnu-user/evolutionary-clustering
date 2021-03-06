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
#include <unistd.h>
#include <time.h>
#include <confuse.h>
#include <unistd.h>
#include <gsl/gsl_matrix.h>
#include "utility.h"
#include "pcg_basic.h"
#include "io.h"
#include "cluster.h"
#include "fitness.h"
#include "operators.h"
#include "selection.h"


int DEBUG, VERBOSE;

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
        *centroids_file = NULL,
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
    CFG_SIMPLE_STR("centroids_file", &centroids_file),
    CFG_SIMPLE_STR("fitness_file", &fitness_file),
    CFG_SIMPLE_STR("cluster_file", &cluster_file),
    CFG_END()
};
cfg_t *cfg;


/**
 * The E-means algorithm, uses a genetic algorithm to optimize the parameters 
 * for the K-means implemetation of clustering based Lloyds clustering algorithm.
 *
 * @return        Status code, 0 for SUCCESS, 1 for ERROR
 */
int emeans(void)
{
    gsl_matrix *data = NULL,
               *bounds = NULL,
               *parent1 = NULL,
               *parent2 = NULL,
               **population = NULL,
               **new_population = NULL,
               ***clusters = NULL;
    int status = SUCCESS;
    double fitness[size],
           probability[size];

    // Initialize the PRNG
    pcg32_random_t rng;
    int rounds = 5;
    pcg32_srandom_r(&rng, time(NULL) ^ (intptr_t)&printf, (intptr_t)&rounds);

    // Allocate memory and load the data
    data = gsl_matrix_alloc(data_rows, data_cols);
    bounds = gsl_matrix_alloc(data_cols, 2);
    parent1 = gsl_matrix_alloc(n_clusters, data_cols);
    parent2 = gsl_matrix_alloc(n_clusters, data_cols);
    population = (gsl_matrix **)calloc(size, sizeof(gsl_matrix **));
    new_population = (gsl_matrix **)calloc(size, sizeof(gsl_matrix **));
    clusters = (gsl_matrix ***)calloc(size, sizeof(gsl_matrix ***));

    for (int i = 0; i < (int)size; ++i)
    {
        clusters[i] = (gsl_matrix **)calloc(n_clusters, sizeof(gsl_matrix **));
        population[i] = gsl_matrix_alloc(n_clusters, data_cols);
        new_population[i] = gsl_matrix_alloc(n_clusters, data_cols);
    }
    if ((status = load_data(data_file, data)) != SUCCESS)
    {   
        fprintf(stderr, RED "Unable to load data!\n" RESET);
        status = ERROR;
        goto free;
    }

    // Calculate the bounds of the data
    calc_bounds(data, bounds);

    // Generate the initial population
    printf(CYAN "Generating initial population...\n" RESET);
    for (int i = 0; i < (int)size; ++i)
    {
        random_centroids(population[i], bounds, &rng);
    }

    // Perform the Genetic Algorithm
    for (int iter = 0; iter < max_iter; ++iter)
    {
        // Compute the fitness of each chromosome
        for (int i = 0; i < (int)size; ++i)
        {
            lloyd_defined(trials, population[i], data, n_clusters, clusters[i]);
            fitness[i] = dunn_index(population[i], n_clusters, clusters[i]);
            if (VERBOSE == 1)
                printf(CYAN "chromsome[%d], fitness: %10.6f\n" RESET, i, fitness[i]);
        }

        // Generate the probabilities for roulette wheel selection
        gen_probability(size, fitness, probability);

        // Save the results if there is a new best solution
        save_results(fitness_file, centroids_file, cluster_file, size, fitness, 
                     population, n_clusters, clusters);

        // Perform roulette wheel selection and GA operators
        if (VERBOSE == 1)
            printf(CYAN "Executing roulette wheel selection...\n" RESET);

        // Select parents from the population and perform genetic operators
        for (int i = 0; i < size; i += 2)
        {
            // Select parents
            for (int j = 0, idx = 0; j < 2; ++j)
            {
                idx = select_parent(size, probability, &rng);
                if (VERBOSE == 1)
                    printf(CYAN "Parent %d selected as chromsome %d from population\n" RESET, j, idx);

                if (j == 0)
                    gsl_matrix_memcpy(parent1, population[idx]);
                else
                    gsl_matrix_memcpy(parent2, population[idx]);
            }

            // Perform crossover and mutation with specified probabilities
            if (VERBOSE == 1)
                printf(CYAN "Performing crossover...\n" RESET);
            if (pcg32_random_r(&rng) / (double)UINT32_MAX <= c_rate)
            {
                crossover(parent1, parent2, &rng);
            }

            if (VERBOSE == 1)
                printf(CYAN "Performing background mutation...\n" RESET);
            for (int j = 0; j < 2; ++j)
            {
                if (pcg32_random_r(&rng) / (double)UINT32_MAX <= m_rate)
                {
                    if (j == 0)
                        mutate(parent1, bounds, &rng);
                    else
                        mutate(parent2, bounds, &rng);
                }
            }

            // Copy each parent to the new population
            if (VERBOSE == 1)
                printf(CYAN "Copying parents to new population...\n" RESET);
            gsl_matrix_memcpy(new_population[i], parent1);
            gsl_matrix_memcpy(new_population[i+1], parent2);
        }

        // Copy the new population to the next population
        if (VERBOSE == 1)
            printf(CYAN "Copying current population as new population\n" RESET);

        for (int i = 0; i < size; ++i)
        {
            gsl_matrix_memcpy(population[i], new_population[i]);
        }

        // Check if stop signal, terminate if present
        if (access("./stop", F_OK) != -1)
        {
            printf(YELLOW "Stop signal received, shutting down!\n" RESET);
            remove("./stop");
            break;
        }

    }
    printf(GREEN "Finished executing E-means, shutting down!\n" RESET);

free:
    for (int i = 0; i < (int)size; ++i)
    {
        for (int j = 0; j < n_clusters; ++j)
        {
            gsl_matrix_free(clusters[i][j]);
        }
        free(clusters[i]);
    }
    free(clusters);
    for (int i = 0; i < (int)size; ++i)
    {
        gsl_matrix_free(population[i]);
        gsl_matrix_free(new_population[i]);
    }
    free(population);
    free(new_population);
    gsl_matrix_free(data);
    gsl_matrix_free(bounds);
    return status;
}


int main(int argc, char *argv[])
{
    int status = SUCCESS;
    char conf_file[100] = "./conf/emeans.conf";

    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, RED "Incorrect parameters!\n" RESET);
        fprintf(stderr, RED "Correct usage:\n" RESET);
        fprintf(stderr, RED "%s <DEBUG> (1..N=DEBUG 0=NODEBUG) <VERBOSE> (1=YES 0=NO) <CONFIG> (DEFAULT ./conf/emeans.conf)\n\n" RESET, argv[0]);
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
        fprintf(stderr, RED "Unable to open file %s\n" RESET, conf_file);
        status = ERROR;
        goto free;
    }

    if (DEBUG == DEBUG_CONFIG)
    {
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
        printf(YELLOW " CENTROIDS FILE: %s\n" RESET, centroids_file);
        printf(YELLOW "   FITNESS FILE: %s\n" RESET, fitness_file);
        printf(YELLOW "   CLUSTER FILE: %s\n" RESET, cluster_file);
        goto free;
    }

    // Execute the E-means algorithm
    status = emeans();

// Free memory and exit
free:
    cfg_free(cfg);
    free(data_file);
    free(centroids_file);
    free(fitness_file);
    free(cluster_file);

    exit(status);
}