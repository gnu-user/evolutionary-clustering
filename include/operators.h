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
#ifndef OPERATORS_H_
#define OPERATORS_H_

#include <gsl/gsl_matrix.h>
#include "pcg_basic.h"

/**
 * Performs chromosome crossover by randomly selecting a crossover point and
 * randomly either swapping the top or the bottom half.
 * 
 * @param  len     The length of the chromosome
 * @param  parent1 The first parent chromosome
 * @param  parent2 The second parent chromosome
 * @param  rng     Pointer to the random number generator
 */
extern void crossover(gsl_matrix *parent1, gsl_matrix *parent2, pcg32_random_t *rng);


/**
 * Performs mutation, selects a random row and column in the chromosome and
 * mutates it to a random value within the min/max bounds.
 * 
 * @param  chromosome The chromosome
 * @param  bounds     The min/max bounds for each dimensions of the data
 * @param  rng        Pointer to the random number generator
 */
extern void mutate(gsl_matrix *chromosome, gsl_matrix *bounds, pcg32_random_t *rng);


#endif /* OPERATORS_H_ */