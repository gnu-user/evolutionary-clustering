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
#include <float.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_statistics.h>
#include "utility.h"
#include "pcg_basic.h"
#include "cluster.h"

int lloyd_random(int trials, gsl_matrix *data, int n_clusters,
                 gsl_matrix **clusters, pcg32_random_t *rng)
{
    uint32_t rows = data->size1,
             cols = data->size2;
    int counts[n_clusters];
    memset(counts, 0, n_clusters * sizeof(int));

    gsl_matrix *centroids = gsl_matrix_alloc(n_clusters, cols);
    gsl_matrix *old_centroids = gsl_matrix_alloc(n_clusters, cols);
    gsl_matrix_set_zero(old_centroids);
    gsl_matrix_set_zero(centroids);

    // Initialize random centroid as a random value from data
    for (int i = 0, r = 0, r2 = 0; i < n_clusters; ++i)
    {
        r = (int)pcg32_boundedrand_r(rng, (int)rows);
        r2 = (int)pcg32_boundedrand_r(rng, (int)rows);
        gsl_vector_view data_row = gsl_matrix_row(data, r);
        gsl_vector_view cent_row = gsl_matrix_row(centroids, i);
        gsl_vector_memcpy(&cent_row.vector, &data_row.vector);
        gsl_vector_view data_row2 = gsl_matrix_row(data, r2);
        gsl_vector_view cent_row2 = gsl_matrix_row(old_centroids, i);
        gsl_vector_memcpy(&cent_row2.vector, &data_row2.vector);
    }

    if (DEBUG == DEBUG_CLUSTER)
    {
        printf(YELLOW "[SLAVE %2d] RANDOM CENTROIDS\n" RESET, SLAVE);
        for (int i = 0; i < n_clusters; ++i) 
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

    // Determine the initial clustering assignment for each
    for (uint32_t i = 0, k = 0; i < rows; ++i)
    {
        double min_norm = DBL_MAX, 
               norm = DBL_MAX;

        gsl_vector *sub = gsl_vector_alloc(cols);
        gsl_vector_view data_row = gsl_matrix_row(data, i);

        for (int j = 0; j < n_clusters; ++j)
        {
            gsl_vector_view cent_row = gsl_matrix_row(centroids, j);
            gsl_vector_memcpy(sub, &data_row.vector);
            gsl_vector_sub(sub, &cent_row.vector);
            norm = gsl_blas_dnrm2(sub);
            
            // Assign to the cluster if norm is less than in all previous clusters
            if (norm <= min_norm)
            {
                min_norm = norm;
                k = j;
            }
        }

        // Assign the data to the cluster with the minimum norm
        gsl_vector_view clust_row = gsl_matrix_row(clusters[k], counts[k]);
        gsl_vector_memcpy(&clust_row.vector, &data_row.vector);
        counts[k] += 1;
    }

    // Calculate the new centroids
    calc_centroids(centroids, data, n_clusters, counts, clusters);

    return SUCCESS;
}


static int calc_centroids(gsl_matrix *centroids, gsl_matrix *data, int n_clusters, 
                          int counts[n_clusters], gsl_matrix **clusters)
{
    uint32_t rows = data->size1,
             cols = data->size2;

    // Calculate the centroid for each cluster
    for (int i = 0; i < n_clusters; ++i)
    {
        // Get the submatrix containing the clustered values
        gsl_matrix_view clust_mat = gsl_matrix_submatrix(clusters[i], 0, 0, counts[i], cols);

        for (int j = 0; j < cols; ++j)
        {
            gsl_vector_view clust_col = gsl_matrix_column(&clust_mat.matrix, j);
            gsl_matrix_set(centroids, i, j, gsl_stats_mean(clust_col.vector.data, 1, counts[i]));
        }
    }

    if (DEBUG == DEBUG_CLUSTER)
    {
        printf(YELLOW "[SLAVE %2d] NEW CENTROIDS\n" RESET, SLAVE);
        for (int i = 0; i < n_clusters; ++i) 
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
