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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "pcg_basic.h"
#include "utility.h"
#include "selection.h"

void gen_probability(int size, double fitness[size], double probability[size])
{
    double total = 0;
 
    if (VERBOSE == 1)
        printf(GREEN "Generating probabilities for population\n" RESET);
        
    // Update the total, including the weighting for negative values
    for (int i = 0; i < size; ++i)
    {
        total += fitness[i];
    }

    // Create probability numberline of weighted fitness values from 0 to 1
    for (int i = 0; i < size; ++i)
    {
        if (i == 0)
        {
            probability[i] = fitness[i] / total;
        }
        else
        {
            probability[i] = probability[i-1] + (fitness[i] / total);
        }
    }

    if (DEBUG == DEBUG_PROBABILITY)
    {
        printf(YELLOW "PROBABILITY RESULTS\n" RESET);
        for (int i = 0; i < size; ++i)
        {
            printf(YELLOW "FITNESS[%d]: %10.6f, PROB[%d]: %10.6f\n" RESET, 
                   i, fitness[i], i, probability[i]);
        }
    }
}


int select_parent(int size, double probability[size], pcg32_random_t *rng)
{
    int idx = 0;
    double r = pcg32_random_r(rng) / (double)UINT32_MAX;

    for (int i = 0; i < size; ++i)
    {
        if (i == 0)
        {
            if (r < probability[i])
            {
                idx = i;
                break;
            }
        }
        else if (r >= probability[i-1] && r < probability[i])
        {
            idx = i;
            break;
        }
    }
    return idx;
}
