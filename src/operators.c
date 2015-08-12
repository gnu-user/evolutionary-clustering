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
#include <math.h>
#include <float.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_statistics.h>
#include "utility.h"
#include "fitness.h"
#include "operators.h"

void crossover(gsl_matrix *parent1, gsl_matrix *parent2, pcg32_random_t *rng)
{
    uint32_t rows = parent1->size1,
             cols = parent1->size2;
    uint32_t cut = (uint32_t)pcg32_boundedrand_r(rng, rows-1) + 1;

    if (DEBUG == DEBUG_CROSSOVER)
    {
        printf(YELLOW "CROSSOVER CENTROIDS BEFORE\n" RESET);
        printf(YELLOW "CHROMSOME CUT POINT: %d\n", cut);
        printf(YELLOW "PARENT[1]:\n" RESET);
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                printf(YELLOW "%10.6f " RESET, gsl_matrix_get(parent1, i, j));
            }
            printf("\n");
        }
        printf(YELLOW "PARENT[2]:\n" RESET);
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                printf(YELLOW "%10.6f " RESET, gsl_matrix_get(parent2, i, j));
            }
            printf("\n");
        }
    }

    // Copy bottom half from parent1 to parent2
    for (uint32_t i = 0; i < cut; ++i)
    {
        gsl_vector_view parent1_row = gsl_matrix_row(parent1, i);
        gsl_vector_view parent2_row = gsl_matrix_row(parent2, i);
        gsl_vector_memcpy(&parent2_row.vector, &parent1_row.vector);
    }

    // Copy top half from parent2 to parent1
    for (uint32_t i = cut; i < rows; ++i)
    {
        gsl_vector_view parent1_row = gsl_matrix_row(parent1, i);
        gsl_vector_view parent2_row = gsl_matrix_row(parent2, i);
        gsl_vector_memcpy(&parent1_row.vector, &parent2_row.vector);
    }

    if (DEBUG == DEBUG_CROSSOVER)
    {
        printf(YELLOW "CROSSOVER CENTROIDS AFTER\n" RESET);
        printf(YELLOW "CHROMSOME CUT POINT: %d\n", cut);
        printf(YELLOW "PARENT[1]:\n" RESET);
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                printf(YELLOW "%10.6f " RESET, gsl_matrix_get(parent1, i, j));
            }
            printf("\n");
        }
        printf(YELLOW "PARENT[2]:\n" RESET);
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                printf(YELLOW "%10.6f " RESET, gsl_matrix_get(parent2, i, j));
            }
            printf("\n");
        }
    }
}


void mutate(gsl_matrix *chromosome, gsl_matrix *bounds, pcg32_random_t *rng)
{
    uint32_t rows = chromosome->size1,
             cols = chromosome->size2,
             row, 
             col;
    double r, min, max;

    // Select a random row and column
    row = (uint32_t)pcg32_boundedrand_r(rng, rows);
    col = (uint32_t)pcg32_boundedrand_r(rng, cols);

    // Set to a random value within the bounds
    min = gsl_matrix_get(bounds, col, 0);
    max = gsl_matrix_get(bounds, col, 1);
    r = (ldexp(pcg32_random_r(rng), -32) * (max - min)) + min;
    gsl_matrix_set(chromosome, row, col, r);


    if (DEBUG == DEBUG_MUTATE)
    {
        printf(YELLOW "MUTATED CHROMSOME\n" RESET);
        printf(YELLOW "ROW: %d, COL: %d, VAL: %10.6f\n" RESET, row, col, r);
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                printf(YELLOW "%10.6f " RESET, gsl_matrix_get(chromosome, i, j));
            }
            printf("\n");
        }
    }
}
