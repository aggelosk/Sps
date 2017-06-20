#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "spot.h"

typedef struct segment{
	unsigned start;
	unsigned end;
	struct segment * next;
}segment;

typedef struct speciment{ 
	person * prs;
	struct speciment * children;
	struct speciment * present_children;			/* the people of the present generation that descent from this ancestor ~ used for the mutation step */
	segment * segments;
	struct speciment * next;	
	struct speciment * prev;			/* required for creating the queue */
}speciment;

typedef struct unifree{
	speciment * spec;
	struct unifree * next;
}unifree;

unsigned min(unsigned x, unsigned y);

unsigned max(unsigned x, unsigned y);

segment * merge_segments(segment * first, segment * second);

speciment * merge(speciment * s1, speciment * s2);

void insert_queue(speciment * s);

void sample_storage(person * p);

void sampling(unsigned samples);

void recombine(speciment * s);

void choose_parent(speciment* s);

void create_parent(speciment * s);

void go_back();

void further_back();

void print_children(speciment * s);

void free_present();

void free_seg(segment * seg);

void tree_destruction(speciment * s);

void print_sampleList();

void print_mutated(person * p);

void rewind_time();