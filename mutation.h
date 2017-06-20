#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <gsl/gsl_math.h>

#include "rewind_time.h"

double power(double x, unsigned y);

unsigned factorial(unsigned n);

void poisson_mutation(unsigned k, double mu);

speciment * pop_spec();

void push_spec(speciment * s);

void empty_specs();

speciment * occurance();

short isSubsegment(segment * seg, unsigned mutation);

void create_table_row(speciment * s, unsigned row);

void print_table_mutation();

void print_table_sample();

void allocate_mutation_table(unsigned rows, unsigned columns);

void mutate();