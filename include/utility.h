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
#ifndef UTILITY_H_
#define UTILITY_H_

// Define terminal colors
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

// Declare the globals and CLI flags DEBUG, VERBOSE
extern int DEBUG, VERBOSE;

/**
 * @enum debug_code
 * @brief Enumeration of the DEBUG codes 
 */
typedef enum
{
    DEBUG_CONFIG        = 1,    /**< Print all the values parsed from the config file */
    DEBUG_DATA          = 2,    /**< Print the contents of the data file */
    DEBUG_CLUSTER       = 3,    /**< Debug the clutering process using lloyd's */
    DEBUG_BOUNDS        = 4,    /**< Debug the min/max bounds for each dimension */
    DEBUG_CENTROIDS     = 5,    /**< Debug the randomly generated initial centroids */
    DEBUG_DUNN          = 6,    /**< Debug the Dunn Index calculations */
    DEBUG_CROSSOVER     = 7,    /**< Debug the crossover operator */
    DEBUG_MUTATE        = 8,    /**< Debug the mutation operator */
    DEBUG_PROBABILITY   = 9    /**< Debug output for the probability generation */
} debug_code;

/**
 * @enum error_code
 * @brief Error codes
 */
typedef enum
{
    SUCCESS         = 0,    /**< Successful execution */
    ERROR           = 1     /**< Generic error code */
} error_code;


#endif /* UTILITY_H_ */
