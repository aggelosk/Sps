#include "spot.h"

typedef struct coord{
	unsigned r;
	unsigned c;
	struct coord * next;
}coord;

typedef struct segment{
	unsigned start;
	unsigned end;
	coord * coords;
	struct segment * next;
}segment;

struct ancestry *anc;

typedef struct preschild{
	int pid;
	segment * segments;
	struct preschild * next;
	struct preschild * prev;
}preschild;

typedef struct speciment{
	person * prs;
	//struct speciment * children;
	struct preschild * present_children;			/* the people of the present generation that descent from this ancestor ~ used for the mutation step */
	struct speciment * next;
	struct speciment * prev;								/* required for creating the queue */
}speciment;

typedef struct affected{
	unsigned num;
	segment * segments;
}affected;

typedef struct ancestry{
  int presentChildren;
  unsigned id;
  affected * affChildren;
}ancestry;

typedef struct rec_free{
	preschild * pres;
	struct unifree * next;
}rec_free;

unsigned min(unsigned x, unsigned y);

unsigned max(unsigned x, unsigned y);

segment * merge_segments(segment * first, segment * second);

speciment * merge(speciment * s1, speciment * s2);

void insert_queue(speciment * s);

void recombine(speciment * s);


void choose_parent(speciment* s);

void create_parent(speciment * s);

void add_present(speciment * s);


void go_back();

void further_back();

void print_children(speciment * s);

/* --- free --- */

void free_present(preschild * p);

void free_seg(segment * seg);

void tree_destruction(speciment * s);


/* --- print --- */

void print_present_children(speciment * s);

void print_segment_representation(speciment *s);

//void print_sampleList();

//void print_mutated(person * p);

void print_segments(segment * tmp);

void rewind_time();
