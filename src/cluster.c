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
#include "pcg_basic.h"
#include "cluster.h"

int lloyd_random(int trials, gsl_matrix *data, int n_clusters,
                 gsl_matrix **clusters, pcg32_random_t *rng)
{
    uint32_t rows = data->size1,
             cols = data->size2;
    uint32_t counts[n_clusters];
    memset(counts, 0, n_clusters * sizeof(int));

    gsl_vector *sub = gsl_vector_alloc(cols);
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
            // Determine the initial clustering assignment for each
            for (uint32_t i = 0, k = 0; i < rows; ++i)
            {
                double min_norm = DBL_MAX, 
                       norm = DBL_MAX;
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
                counts[k] += 1;
            }

            // Assign the data to the cluster with the minimum norm
            for (int i = 0; i < n_clusters; ++i)
            {
                if (counts[i] < 1)
                {
                    clusters[i] = NULL;
                }
                else
                {
                    clusters[i] = gsl_matrix_alloc(counts[i], cols);
                    gsl_matrix_set_zero(clusters[i]);
                }
            }
            memset(counts, 0, n_clusters * sizeof(int));

            for (uint32_t i = 0, k = 0; i < rows; ++i)
            {
                k = gsl_matrix_get(clust_stats, trial, i);

                // Skip clusters that are empty
                if (clusters[k] == NULL)
                {
                    continue;
                }
                gsl_vector_view data_row = gsl_matrix_row(data, i);
                gsl_vector_view clust_row = gsl_matrix_row(clusters[k], counts[k]);
                gsl_vector_memcpy(&clust_row.vector, &data_row.vector);
                counts[k] += 1;
            }

            // Calculate the new centroids
            calc_centroids(centroids, data, n_clusters, clusters);

            // If centroids are the same then clustering has converged
            if (gsl_matrix_equal(centroids, old_centroids))
            {
                break;
            }
            gsl_matrix_memcpy(old_centroids, centroids);
            
            for (int i = 0; i < n_clusters; ++i)
            {
                gsl_matrix_free(clusters[i]);
            }
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
                    printf(YELLOW "%10.6f " RESET, gsl_matrix_get(clusters[i], j, k));
                }
                printf("\n");
            }
        }
    }
    gsl_vector_free(sub);

    return SUCCESS;
}


int lloyd_defined(int trials, gsl_matrix *centroids, gsl_matrix *data, 
                  int n_clusters, gsl_matrix **clusters)
{
    uint32_t rows = data->size1,
             cols = data->size2;
    uint32_t counts[n_clusters];
    memset(counts, 0, n_clusters * sizeof(int));

    gsl_vector *sub = gsl_vector_alloc(cols);
    gsl_matrix *clust_stats = gsl_matrix_alloc(1, rows);
    gsl_matrix *old_centroids = gsl_matrix_alloc(n_clusters, cols);
    gsl_matrix_memcpy(old_centroids, centroids);

    // Execute LLoyd's algorithm until convergance
    for (int run = 0; run < 10000; ++run)
    {
        // Determine the initial clustering assignment for each
        for (uint32_t i = 0, k = 0; i < rows; ++i)
        {
            double min_norm = DBL_MAX, 
                   norm = DBL_MAX;
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
                    gsl_matrix_set(clust_stats, 0, i, k);
                }
            }
            counts[k] += 1;
        }

        // Assign the data to the cluster with the minimum norm
        for (int i = 0; i < n_clusters; ++i)
        {
            if (counts[i] < 1)
            {
                clusters[i] = NULL;
            }
            else
            {
                clusters[i] = gsl_matrix_alloc(counts[i], cols);
                gsl_matrix_set_zero(clusters[i]);
            }
        }
        memset(counts, 0, n_clusters * sizeof(int));

        for (uint32_t i = 0, k = 0; i < rows; ++i)
        {
            k = gsl_matrix_get(clust_stats, 0, i);

            // Skip clusters that are empty
            if (clusters[k] == NULL)
            {
                continue;
            }
            gsl_vector_view data_row = gsl_matrix_row(data, i);
            gsl_vector_view clust_row = gsl_matrix_row(clusters[k], counts[k]);
            gsl_vector_memcpy(&clust_row.vector, &data_row.vector);
            counts[k] += 1;
        }

        // Calculate the new centroids
        calc_centroids(centroids, data, n_clusters, clusters);

        // If centroids are the same then clustering has converged
        if (gsl_matrix_equal(centroids, old_centroids))
        {
            break;
        }
        gsl_matrix_memcpy(old_centroids, centroids);
        
        for (int i = 0; i < n_clusters; ++i)
        {
            gsl_matrix_free(clusters[i]);
        }
    }

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
                    printf(YELLOW "%10.6f " RESET, gsl_matrix_get(clusters[i], j, k));
                }
                printf("\n");
            }
        }
    }
    gsl_vector_free(sub);

    return SUCCESS;
}


int calc_centroids(gsl_matrix *centroids, gsl_matrix *data, int n_clusters, 
                   gsl_matrix **clusters)
{
    uint32_t rows = 0,
             cols = data->size2;

    // Calculate the centroid for each cluster
    for (int i = 0; i < n_clusters; ++i)
    {
        // Empty cluster
        if (clusters[i] == NULL)
        {
            continue;
        }
        rows = clusters[i]->size1;

        // Get the submatrix containing the clustered values
        gsl_matrix_view clust_mat = gsl_matrix_submatrix(clusters[i], 0, 0, rows, cols);

        for (uint32_t j = 0; j < cols; ++j)
        {
            gsl_vector_view clust_col = gsl_matrix_column(&clust_mat.matrix, j);
            gsl_matrix_set(centroids, i, j, gsl_stats_mean(clust_col.vector.data, 1, rows));
        }
    }

    return SUCCESS;
}


int calc_bounds(gsl_matrix *data, gsl_matrix *bounds)
{
    uint32_t cols = data->size2;
    double min, max;

    // Calculate the minimum and maximum bounds for each dimension
    for (uint32_t i = 0; i < cols; ++i)
    {
        gsl_vector_view data_col = gsl_matrix_column(data, i);
        gsl_vector_minmax(&data_col.vector, &min, &max);
        gsl_matrix_set(bounds, i, 0, min);
        gsl_matrix_set(bounds, i, 1, max);
    }

    if (DEBUG == DEBUG_BOUNDS)
    {
        printf(YELLOW "[ MASTER ]  MIN/MAX BOUNDS\n" RESET);
        for (uint32_t i = 0; i < cols; ++i)
        {
            printf(YELLOW "%10.6f %10.6f\n" RESET, gsl_matrix_get(bounds, i, 0),
                   gsl_matrix_get(bounds, i, 1));
        }
    }

    return SUCCESS;
}


int random_centroids(gsl_matrix *centroids, gsl_matrix *bounds, pcg32_random_t *rng)
{
    uint32_t rows = centroids->size1,
             cols = centroids->size2;
    double r, min, max;

    for (uint32_t i = 0; i < rows; ++i)
    {
        for (uint32_t j = 0; j < cols; ++j)
        {
            min = gsl_matrix_get(bounds, j, 0);
            max = gsl_matrix_get(bounds, j, 1);
            r = (ldexp(pcg32_random_r(rng), -32) * (max - min)) + min;
            gsl_matrix_set(centroids, i, j, r);
        }
    }

    if (DEBUG == DEBUG_CENTROIDS)
    {
        printf(YELLOW "[ MASTER ]  RANDOM CENTROIDS\n" RESET);
        for (uint32_t i = 0; i < rows; ++i)
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                printf(YELLOW "%10.6f " RESET, gsl_matrix_get(centroids, i, j));
            }
            printf("\n");
        }
    }

    return SUCCESS;
}