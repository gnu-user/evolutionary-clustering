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
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_statistics.h>
#include "utility.h"
#include "fitness.h"

double dunn_index(gsl_matrix *centroids, int n_clusters, gsl_matrix **clusters)
{
    uint32_t rows = 0,
             cols = centroids->size2;
    double dunn = 0;
    gsl_vector *sub = gsl_vector_alloc(cols),
               *mean_dist = gsl_vector_alloc(n_clusters),
               *interclus = gsl_vector_alloc(n_clusters * (n_clusters-1));

    gsl_vector_set_zero(mean_dist);

    // Calculate the mean distance between all pairs in each cluster
    for (int n = 0; n < n_clusters; ++n)
    {
        if (clusters[n] == NULL)
        {
            continue;
        }
        rows = clusters[n]->size1;
        gsl_vector *dist = gsl_vector_alloc(rows * (rows-1));

        for (uint32_t i = 0, k = 0; i < rows; ++i)
        {
            gsl_vector_view row = gsl_matrix_row(clusters[n], i);
            
            for (uint32_t j = (i+1) % rows; i != j; j = (j+1) % rows, ++k)
            {
                gsl_vector_view row2 = gsl_matrix_row(clusters[n], j);
                gsl_vector_memcpy(sub, &row.vector);
                gsl_vector_sub(sub, &row2.vector);
                gsl_vector_set(dist, k, gsl_blas_dnrm2(sub));
            }
        }
        gsl_vector_set(mean_dist, n, gsl_stats_mean(dist->data, 1, rows * (rows-1)));
        gsl_vector_free(dist);
    }
    
    // Calculate the intercluster distance metric
    for (int i = 0, k = 0; i < n_clusters; ++i)
    {
        gsl_vector_view row = gsl_matrix_row(centroids, i);

        for (int j = (i+1) % n_clusters; i != j; j = (j+1) % n_clusters, ++k)
        {
            gsl_vector_view row2 = gsl_matrix_row(centroids, j);
            gsl_vector_memcpy(sub, &row.vector);
            gsl_vector_sub(sub, &row2.vector);
            gsl_vector_set(interclus, k, gsl_blas_dnrm2(sub));
        }
    }

    // Calcualte the Dunn Index
    dunn = gsl_vector_min(interclus) / gsl_vector_max(mean_dist);

    if (DEBUG == DEBUG_DUNN)
    {
        printf(YELLOW "[SLAVE %2d] DUNN INDEX CALCULATIONS\n" RESET, SLAVE);
        for (int i = 0; i < n_clusters; ++i)
        {
            if (i == 0)
                printf(YELLOW "mean_dist = %10.6f " RESET, gsl_vector_get(mean_dist, i));
            else
                printf(YELLOW "%10.6f " RESET, gsl_vector_get(mean_dist, i));
        }
        printf("\n");

        for (int i = 0; i < n_clusters * (n_clusters-1); ++i)
        {
            if (i == 0)
                printf(YELLOW "interclus = %10.6f " RESET, gsl_vector_get(interclus, i));
            else
                printf(YELLOW "%10.6f " RESET, gsl_vector_get(interclus, i));
        }
        printf("\n");

        printf(YELLOW "DUNN INDEX: %10.6f\n" RESET, dunn);
    }

    return dunn;
}

