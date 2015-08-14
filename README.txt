Evolutionary K-means clustering (E-means) using Genetic Algorithms
==============================================================================

An enhanced highly parallel K-means clustering algorithm using evolutionary 
strategies to perform metaheuristic optimization.


Notice
----------------------------------------

All of the instructions and code for this project have been developed and 
tested on Ubuntu 14.04 LTS x64. The code has been written in portable C99
standard and should compile without issue on any UNIX based machine.


Install Dependencies
----------------------------------------

In order to install all of the necessary dependencies on Ubuntu 14.04,
simply execute the following command with root priviledges using sudo.

    sudo apt-get install build-essential libblas-dev liblapack-dev \
                         libgsl0-dev libconfuse-dev


Build Executable
----------------------------------------

To build the executable, simply navigate to the main directory containing
the "Makefile" and execute the make command as follows.

    make

If you wish to build a release version of the executable, which will have
better performance then use the make release command as follows.

    make release


Running the Executable
----------------------------------------

By default the executable will use the default configuration file, in the 
local path ./conf/emeans.conf, this executable will run the E-means algorithm
using the test iris flower dataset. If you wish to customize the configurations
copy the default configuration file, emeans.conf, into the conf/ directory
and modify the configurations as you see fit.

In order to run the executable, you must provide it with 2 or 3 parameters,
where the first parameter is the DEBUG flag (to show debugging output),
the second is VERBOSE flag (to show more logging output), and the last
flag (optional) CONFIG, is the name of the config file in the conf/ directory
if none is provided the default config file ./conf/emeans.conf will be used.
For the list of DEBUG flags, you can view the source file in include/utility.h.

To execute the E-means algorithm with verbose output and no debugging output
using the default cofiguration file with the iris flower data you can execute
it as follows.

    ./emeans.exe 0 1

The results from the execution will be printed to the screen as it is
optimizing the clustering, the final results will be saved in the results/
directory.
