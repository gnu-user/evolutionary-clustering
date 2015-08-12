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

/**
 * Calculates the Dunn Index, a metric for evaluating the clustering results.
 *
 * @param centroids  Pointer to matrix containing the centroids
 * @param n_clusters The number of clusters
 * @param clusters   Pointer to array of matrices containing data in clusters
 * 
 * @return           The Dunn Index 
 */
extern double dunn_index(gsl_matrix *centroids, int n_clusters, gsl_matrix **clusters);


#endif /* CLUSTER_H_ */