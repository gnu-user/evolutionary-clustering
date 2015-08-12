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
#ifndef CLUSTER_H_
#define CLUSTER_H_

#include <gsl/gsl_matrix.h>
#include "pcg_basic.h"

/**
 * Performs Lloyd's algorithm using random initial centroids.
 *
 * @param trials     Number of trials to perform
 * @param data       Pointer to matrix containing the data
 * @param n_clusters The number of clusters
 * @param clusters   Pointer to array of matrices containing data in clusters
 * @param rng        Pointer to the random number generator
 * 
 * @return      The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int lloyd_random(int trials, gsl_matrix *data, int n_clusters,
                        gsl_matrix **clusters, pcg32_random_t *rng);


/**
 * Performs Lloyd's algorithm using the defined centroids.
 * 
 * @param trials     Number of trials to perform
 * @param centroids  Pointer to matrix containing the centroids
 * @param data       Pointer to matrix containing the data
 * @param n_clusters The number of clusters
 * @param clusters   Pointer to array of matrices containing data in clusters
 * 
 * @return      The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int lloyd_defined(int trials, gsl_matrix *centroids, gsl_matrix *data, 
                         int n_clusters, gsl_matrix **clusters);


/**
 * Calculate the new centroids using the clustering assignment.
 * 
 * @param  centroids  Pointer to matrix containing centroids to be updated
 * @param  data       Point to matrix containing the data
 * @param  n_clusters The number of clusters
 * @param  cluster    Pointer to vector containing cluster assignment
 * 
 * @return            The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int calc_centroids(gsl_matrix *centroids, gsl_matrix *data, int n_clusters, 
                          gsl_matrix **clusters);


/**
 * Calculates the minimum and maximum bounds based on the data.
 *
 * @param  data   Point to matrix containing the data
 * @param  bounds The min/max bounds for each dimensions of the data
 *
 * @return        The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int calc_bounds(gsl_matrix *data, gsl_matrix *bounds);


/**
 * Generates random centroids within the bounds for each dimension.
 *
 * @param  centroids Pointer to matrix containing centroids to be updated
 * @param  bounds    The min/max bounds for each dimensions of the data
 * @param  rng       Pointer to the random number generator
 *
 * @return           The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int random_centroids(gsl_matrix *centroids, gsl_matrix *bounds, 
                            pcg32_random_t *rng);


#endif /* CLUSTER_H_ */