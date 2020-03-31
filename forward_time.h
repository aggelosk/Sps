#include "spot.h"

unsigned logistic_function(unsigned i, unsigned j);

void migration(unsigned i, unsigned j, person * child);

person * birth(unsigned i, unsigned j, person * child);

unsigned long int reproduce(unsigned i, unsigned j);

void intergrate_migration(unsigned i, unsigned j);

void prune();

unsigned darwinism(unsigned e, unsigned i, unsigned j);

void event();

void print_massmig(char * filename, unsigned ev);

void print_people();

void destruction();

void forward_time();
