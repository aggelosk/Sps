#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>


#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>


struct ancestry *anc;



typedef struct topology{
    unsigned gen; /* first generation this topology applies */
    char * filename;
    struct topology * next;
}topology;

typedef struct ram{
	unsigned cram;
	struct ram * next;
}ram;


typedef struct boxed{
	unsigned samples;
	unsigned srow;
	unsigned erow;
	unsigned scol;
	unsigned ecol;
	unsigned gen;
	struct box * next;
}boxed;


typedef struct leader{
	unsigned lead;
	int xaxis;
	int yaxis;
}leader;

typedef struct person{
	unsigned pid;
	float fitness;
	/* just for the spatial origin version */
	unsigned row;
	unsigned col;
	struct person * parent1;
	struct person * parent2;
	//struct person * next; /* functions as a way to initialize the list of people in an area */
  char flag; /* boolean that shows if the person reproduced or not */
}person;

typedef struct init_pop{
	unsigned lat;
	unsigned lon;
	unsigned ppl;
	struct init_pop * next;
}init_pop;

typedef struct spot{
	unsigned capacity;  /* max population possible */
	int pop_num; /* shows which population this belongs to */
  unsigned fit_people;

	float growth_rate;
	float migrate; /* probability to leave the area ~ so 0 - 1 values */
	float friction;	 /* probability to enter the area ~ sort of an area quality check */
	float total_fric; /* sum of friction from neighbouring areas ~ used for migration */

	unsigned long int population; /* number of people in the area */
	person * people; /* a list of every person in the area */
	unsigned long int incoming; /* people about to migrate to this area */
	person * immigrants;
}spot;

void parser_migr();
void parser_fric();
void parser_grth();
void parser_cpct();

void area_config(unsigned i, unsigned j);

void set_spot(unsigned ev, unsigned i, unsigned j);
