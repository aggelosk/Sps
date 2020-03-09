#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "rewind_time.h"

#include <gsl/gsl_permutation.h>

void area_sampling(unsigned sam, unsigned srow, unsigned erow, unsigned scol, unsigned ecol, unsigned gen);

void sampling(unsigned samples);

void print_boxes();
