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

    gsl_matrix *clust_stats = gsl_matrix_alloc(trials, rows);
    gsl_matrix *centroids = gsl_matrix_alloc(n_clusters, cols);
    gsl_matrix *old_centroids = gsl_matrix_alloc(n_clusters, cols);
    gsl_matrix_set_zero(old_centroids);
    gsl_matrix_set_zero(centroids);

    for (int trial = 0; trial < trials; ++trial)
    {
        // Initialize centroids as a random value from data
        for (int i = 0, r = 0; i < n_clusters; ++i)
        {
            r = (int)pcg32_boundedrand_r(rng, (int)rows);
            gsl_vector_view data_row = gsl_matrix_row(data, r);
            gsl_vector_view cent_row = gsl_matrix_row(centroids, i);
            gsl_vector_view cent_row2 = gsl_matrix_row(old_centroids, i);
            gsl_vector_memcpy(&cent_row.vector, &data_row.vector);
            gsl_vector_memcpy(&cent_row2.vector, &cent_row.vector);
        }

        // Execute LLoyd's algorithm until convergance
        for (int run = 0; run < 10000; ++run)
        {
            memset(counts, 0, n_clusters * sizeof(int));

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
                        gsl_matrix_set(clust_stats, trial, i, k);
                    }
                }

                // Assign the data to the cluster with the minimum norm
                gsl_vector_view clust_row = gsl_matrix_row(clusters[k], counts[k]);
                gsl_vector_memcpy(&clust_row.vector, &data_row.vector);
                counts[k] += 1;
                gsl_vector_free(sub);
            }

            // Calculate the new centroids
            calc_centroids(centroids, data, n_clusters, counts, clusters);

            // If centroids are the same then clustering has converged
            if (gsl_matrix_equal(centroids, old_centroids))
            {
                printf("CENTROIDS ARE SAME, ALGORITHM HAS CONVERGED!\n");
                break;
            }
            gsl_matrix_memcpy(old_centroids, centroids);
        }
    }

    if (DEBUG == DEBUG_CLUSTER)
    {
        printf(YELLOW "[SLAVE %2d] CLUSTERING TRIALS RESULTS\n" RESET, SLAVE);
        for (int i = 0; i < trials; ++i)
        {
            for (uint32_t j = 0; j < rows; ++j)
            {
                if (j == 0)
                    printf(YELLOW "trial[%d] = %d " RESET, i, (int)gsl_matrix_get(clust_stats, i, j));
                else
                    printf(YELLOW "%d " RESET, (int)gsl_matrix_get(clust_stats, i, j));
            }
            printf("\n");
        }
    }

    // TODO Assign the final clusters based on the stats from the trials
    /*memset(counts, 0, n_clusters * sizeof(int));
    for (int i = 0, k = 0; i < rows; ++i)
    {
        gsl_vector_view data_row = gsl_matrix_row(data, i);
        gsl_vector_view clust_stats_col = gsl_matrix_column(clust_stats, i);
        k = (int)round(gsl_stats_mean(clust_stats_col.vector.data, 1, trials));
        gsl_vector_view clust_row = gsl_matrix_row(clusters[k], counts[k]);
        gsl_vector_memcpy(&clust_row.vector, &data_row.vector);
        counts[k] += 1;
    }*/

    if (DEBUG == DEBUG_CLUSTER)
    {
        printf(YELLOW "[SLAVE %2d] FINAL CLUSTERING RESULTS\n" RESET, SLAVE);
        for (int i = 0; i < n_clusters; ++i)
        {
            printf(YELLOW "CLUSTER: %d\n" RESET, i);
            for (uint32_t j = 0; j < counts[i]; ++j)
            {
                for (uint32_t k = 0; k < cols; ++k)
                {
                    printf(YELLOW "%10.6f " RESET, j, gsl_matrix_get(clusters[i], j, k));
                }
                printf("\n");
            }
        }
    }

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
        // Empty cluster
        if (counts[i] < 1)
            continue;

        // Get the submatrix containing the clustered values
        gsl_matrix_view clust_mat = gsl_matrix_submatrix(clusters[i], 0, 0, counts[i], cols);

        for (int j = 0; j < cols; ++j)
        {
            gsl_vector_view clust_col = gsl_matrix_column(&clust_mat.matrix, j);
            gsl_matrix_set(centroids, i, j, gsl_stats_mean(clust_col.vector.data, 1, counts[i]));
        }
    }

    return SUCCESS;
}
