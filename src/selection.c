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
#include <string.h>
#include <stdbool.h>
#include "pcg_basic.h"
#include "utility.h"
#include "selection.h"

void gen_probability(int size, double fitness[size], double probability[size])
{
    double total = 0;
 
    if (VERBOSE == 1)
        printf(CYAN "Generating probabilities for population...\n" RESET);
        
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
