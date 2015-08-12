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
#include <stdint.h>
#include "utility.h"
#include "io.h"


int load_data(char *input, gsl_matrix *data)
{
    uint32_t rows = data->size1,
             cols = data->size2;
    double val = 0;
    FILE *ifp;

    printf(CYAN "Loading: %s\n" RESET, input);
    if ((ifp = fopen(input, "r")) == NULL) 
    {
        fprintf(stderr, RED "Can't open input file %s!\n" RESET, input);
        return ERROR;
    }

    for (uint32_t i = 0; i < rows; ++i)
    {
        for (uint32_t j = 0; j < cols; ++j)
        {
            fscanf(ifp, "%lf,", &val);
            gsl_matrix_set(data, i, j, val);
        }
        fscanf(ifp, "\n");
    }
    fclose(ifp);

    if (DEBUG == DEBUG_DATA)
    {
        printf(YELLOW "DATA LOADED\n");
        for (uint32_t i = 0; i < rows; ++i) 
        {
            for (uint32_t j = 0; j < cols; ++j)
            {
                if (j == cols-1)
                    printf(YELLOW "%10.6f\n" RESET, gsl_matrix_get(data, i, j));
                else
                    printf(YELLOW "%10.6f " RESET, gsl_matrix_get(data, i, j));
            }
        }
    }
    return SUCCESS;
}