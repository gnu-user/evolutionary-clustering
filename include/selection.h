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
#ifndef SELECTION_H_
#define SELECTION_H_

#include "pcg_basic.h"

/**
 * Perform the roulette wheel probability selection, an array is populated with
 * the index of the chromosome to select with a frequency based on the fitness value.
 *
 * @param size        The size of the population
 * @param fitness     Pointer to an array of fitness values for population
 * @param probability Pointer to the probability array, populated by function
 */
extern void gen_probability(int size, double fitness[size], double probability[size]);

/**
 * Selects a parent from the population at random with a probability of being
 * selected based on the proabilities provided.
 * 
 * @param  size        The size of the population
 * @param  probability Probabilities of each chromosome in population being selected
 * @param  rng         Pointer to the random number generator
 * @return             The index of the parent in the population to select
 */
extern int select_parent(int size, double probability[size], pcg32_random_t *rng);


#endif /* SELECTION_H_ */
