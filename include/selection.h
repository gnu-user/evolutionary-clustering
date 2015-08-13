/*
 * Genetic Algorithm Optimization
 *
 * Contains functionality for the related selection methods executed
 * when deciding chromosomes to select for each new population.
 * 
 * Author: Jonathan Gillett
 *  
 * Copyright (C) 2015, Makeplain
 * All rights reserved.
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
