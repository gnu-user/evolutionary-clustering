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
#ifndef IO_H_
#define IO_H_

#include <gsl/gsl_matrix.h>


/**
 * Loads the data from as CSV file into a matrix,
 *
 * @param input Path to the data file
 * @param data  Pointer to the GSL matrix to be populated
 * 
 * @return      The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int load_data(char *input, gsl_matrix *data);


/**
 * Save the chromosome and fitness value if they are better than previous.
 *
 * @param output     Path to save the optimal fitness value
 * @param output2    Path to save the optimal fitness centroids
 * @param output3    Path to save the optimal cluster results
 * @param size       Size of the populations
 * @param fitness    Pointer to array of fitness values for the population
 * @param population Population of all chromosomes
 * @param n_clusters The number of clusters
 * @param clusters   The clusters for each chromosome in the population
 * 
 * @return           The status code, 0 for SUCCESS, 1 for ERROR
 */
extern int save_results(char *output, char *output2, char *output3, int size, double fitness[size], 
                        gsl_matrix **population, int n_clusters, gsl_matrix ***clusters);


#endif /* IO_H_ */