#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <gsl/gsl_math.h>

#include "rewind_time.h"

void print_table_sample();

void allocate_mutation_table(unsigned rows, unsigned columns);

short isSubsegment(segment * seg, unsigned mutation);

void mutate(unsigned nmut);
