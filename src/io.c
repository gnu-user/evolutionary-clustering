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
#include <float.h>
#include "utility.h"
#include "io.h"


int save_results(char *output, char *output2, char *output3, int size, double fitness[size], 
                 gsl_matrix **population, int n_clusters, gsl_matrix ***clusters)
{
    uint32_t rows = 0,
             cols = 0;
    int max_idx = 0;
    FILE *ofp, *ofp2, *ofp3;
    double new_fitness = DBL_MIN;
    // Static record across all function calls of the max fitness
    static double max_fitness = DBL_MIN;

    // Determine the chromosome with the highest fitness
    for (int i = 0; i < size; ++i)
    {
        if (fitness[i] > max_fitness)
        {
            new_fitness = fitness[i];
            max_idx = i;
        }
    }
    // Only save the results if fitness is same or better
    if (max_fitness > new_fitness)
    {
        return SUCCESS;
    }
    max_fitness = new_fitness;

    printf(GREEN "Saving results for new best fitness: %10.6f\n" RESET, max_fitness);

    // Append the solution to the results file
    if ((ofp = fopen(output, "a")) == NULL) 
    {
        fprintf(stderr, RED "Can't open output file %s!\n" RESET, output);
        return ERROR;
    }
    if ((ofp2 = fopen(output2, "w")) == NULL) 
    {
        fprintf(stderr, RED "Can't open output file %s!\n" RESET, output2);
        return ERROR;
    }
    if ((ofp3 = fopen(output3, "w")) == NULL) 
    {
        fprintf(stderr, RED "Can't open output file %s!\n" RESET, output3);
        return ERROR;
    }

    // Save the current optimal fitness
    fprintf(ofp, "%10.6f\n", max_fitness);
    fclose(ofp);

    // Save the optimal population centroids
    rows = population[max_idx]->size1;
    cols = population[max_idx]->size2;
    for (uint32_t i = 0; i < rows; ++i)
    {
        for (uint32_t j = 0; j < cols; ++j)
        {
            if (j == 0)
                fprintf(ofp2, "%10.6f", gsl_matrix_get(population[max_idx], i, j));
            else
                fprintf(ofp2, ",%10.6f", gsl_matrix_get(population[max_idx], i, j));
        }
        fprintf(ofp2, "\n");
    }
    fclose(ofp2);

    // Save the optimal clustering
    for (int n = 0; n < n_clusters; ++n)
    {
        // Skip empty clusters
        if (clusters[max_idx][n] == NULL)
            continue;

        rows = clusters[max_idx][n]->size1;
        cols = clusters[max_idx][n]->size2;
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                if (j == 0)
                    fprintf(ofp3, "%10.6f,%10.6f", (double)n, gsl_matrix_get(clusters[max_idx][n], i, j));
                else
                    fprintf(ofp3, ",%10.6f", gsl_matrix_get(clusters[max_idx][n], i, j));
            }
            fprintf(ofp3, "\n");
        }
    }
    fclose(ofp3);
    
    printf("MADE IT!");
    return SUCCESS;
}


int load_data(char *input, gsl_matrix *data)
{
    uint32_t rows = data->size1,
             cols = data->size2;
    double val = 0;
    FILE *ifp;

    printf(CYAN "Loading: %s\n" RESET, input);
    if ((ifp = fopen(input, "r")) == NULL) 
    {
        fprintf(stderr, RED "Can't open input file %s!\n" RESET, input);
        return ERROR;
    }

    for (uint32_t i = 0; i < rows; ++i)
    {
        for (uint32_t j = 0; j < cols; ++j)
        {
            fscanf(ifp, "%lf,", &val);
            gsl_matrix_set(data, i, j, val);
        }
        fscanf(ifp, "\n");
    }
    fclose(ifp);

    if (DEBUG == DEBUG_DATA)
    {
        printf(YELLOW "DATA LOADED\n");
        for (uint32_t i = 0; i < rows; ++i) 
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                if (j == cols-1)
                    printf(YELLOW "%10.6f\n" RESET, gsl_matrix_get(data, i, j));
                else
                    printf(YELLOW "%10.6f " RESET, gsl_matrix_get(data, i, j));
            }
        }
    }
    return SUCCESS;
}