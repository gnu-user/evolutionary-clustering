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
#include <stdbool.h>
#include <confuse.h>

// Define process 0 as MASTER
#define MASTER 0

int DEBUG, VERBOSE, SLAVE;

// Define the configuration parameters
int64_t k_min = 2,
        k_max = 50,
        trials = 50,
        max_iter = 10000;
double  mutation = 0.01,
        crossover = 0.70;
char    *data_file = NULL,
        *results_file = NULL;

// The configuration file parsing mappings
cfg_opt_t opts[] = {
    CFG_SIMPLE_INT("k_min", &k_min),
    CFG_SIMPLE_INT("k_max", &k_max),
    CFG_SIMPLE_INT("trials", &trials),
    CFG_SIMPLE_INT("max_iter", &max_iter),
    CFG_SIMPLE_FLOAT("mutation", &mutation),
    CFG_SIMPLE_FLOAT("crossover", &crossover),
    CFG_SIMPLE_INT("max_iter", &max_iter),
    CFG_SIMPLE_STR("data_file", &data_file),
    CFG_SIMPLE_STR("results_file", &results_file),
    CFG_END()
};
cfg_t *cfg;


int main(int argc, char *argv[])
{
    int status = SUCCESS;
    char conf_file[100] = "./conf/ga.conf";

    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, RED "[ MASTER ]  Incorrect parameters!\n" RESET);
        fprintf(stderr, RED "[ MASTER ]  Correct usage:\n" RESET);
        fprintf(stderr, RED "[ MASTER ]  %s <DEBUG> (1=DEBUG 0=NODEBUG) <VERBOSE> (1=YES 0=NO) <CONFIG> (DEFAULT emeans.conf)\n\n" RESET, argv[0]);
        status = ERROR;
        goto free;
    }
    DEBUG = atoi(argv[1]);
    VERBOSE = atoi(argv[2]);

    if (argc > 3)
    {
        strcpy(conf_file, "./conf/");
        strcat(conf_file, argv[3]);
    }

    if (access(conf_file, F_OK) != -1)
    {
        cfg = cfg_init(opts, 0);
        cfg_parse(cfg, conf_file);
    }
    else
    {
        fprintf(stderr, RED "[ MASTER ]  Unable to open file %s\n" RESET, conf_file);
        status = ERROR;
        goto free;
    }

// Free memory and exit
free:
    cfg_free(cfg);
    free(data_file);
    free(results_file);
    
    exit(status);
}