#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "spot.h"

unsigned logistic_function(unsigned i, unsigned j);

void migration(unsigned i, unsigned j, person * child);

person * birth(unsigned i, unsigned j);

unsigned long int reproduce(unsigned i, unsigned j);

void intergrate_migration(unsigned i, unsigned j);

void darwinism(unsigned i, unsigned j);

void event();

void print_massmig(char * filename, unsigned ev);

void print_people();

void destruction();

void forward_time();