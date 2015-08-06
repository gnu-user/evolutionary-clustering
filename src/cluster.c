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
#include <gsl/gsl_matrix.h>
#include "utility.h"
#include "pcg_basic.h"
#include "cluster.h"

int lloyd_random(int trials, gsl_matrix *data, gsl_matrix *cluster, 
                 pcg32_random_t *rng)
{
    uint32_t rows = data->size1,
             cols = data->size2,
             clusters = cluster->size1;

    gsl_matrix *centroids = gsl_matrix_alloc(clusters, cols);
    gsl_matrix_set_zero(centroids);

    // Initialize random centroid as a random value from data
    for (int j = 0, r = 0; j < (int)clusters; ++j)
    {
        r = (int)pcg32_boundedrand_r(rng, (int)rows);
        gsl_vector_view data_row = gsl_matrix_row(data, r);
        gsl_vector_view cent_row = gsl_matrix_row(centroids, j);
        gsl_vector_memcpy(&cent_row.vector, &data_row.vector);
    }

    if (DEBUG == DEBUG_CLUSTER)
    {
        printf(YELLOW "[SLAVE %2d] RANDOM CENTROIDS\n", SLAVE);
        for (uint32_t i = 0; i < clusters; ++i) 
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                if (j == 0)
                    printf(YELLOW "cluster[%d] = %10.6f " RESET, i, gsl_matrix_get(centroids, i, j));
                else
                    printf(YELLOW "%10.6f " RESET, gsl_matrix_get(centroids, i, j));
            }
            printf("\n");
        }
    }
    return SUCCESS;
}
