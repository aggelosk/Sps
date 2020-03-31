#include <gsl/gsl_math.h>
#include <gsl/gsl_permutation.h>

#include "rewind_time.h"

void print_table_sample();

void allocate_mutation_table(unsigned rows, unsigned columns);

short isSubsegment(segment * seg, unsigned mutation);

void mutate(unsigned nmut);
