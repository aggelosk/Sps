#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

struct ancestry *anc;  // why in the header file???

typedef enum terrain_t{
	sea, 	/* first so we can default to it */
	land,
	grass,
	forest,
	mountain,
	savvanah,
	tropical,
	tundra
}terrain_t;

typedef struct leader{
	unsigned lead;
	int xaxis;
	int yaxis;
}leader;

typedef struct person{
	int pid;
	char flag; /* boolean that shows if the person reproduced or not */
	float fitness; 
	struct person * parent1;
	struct person * parent2;
	struct person * next; /* functions as a way to initialize the list of people in an area */
}person;

typedef struct spot{
	double latitude; 		/* y axis */
	double longtitude;		/* x axis */
	double altitude;		/* might be under sea level so negative values also work */
	terrain_t type;
	
	unsigned capacity;  /* max population possible */
	double growth_rate;
	double migrate; /* probability to leave the area ~ so 0 - 1 values */
	double friction;	 /* probability to enter the area ~ sort of an area quality check */
	
	int pop_num; /* shows which population this belongs to */
	unsigned long int population; /* number of people in the area */
	float fit_people;
	person * people; /* a list of every person in the area */
	unsigned long int incoming; /* people about to migrate to this area */
	person * immigrants;
}spot;

void parser();

void area_config(unsigned i, unsigned j);

void set_spot(unsigned ev, unsigned i, unsigned j, double latitude, double longtitude, double altitude, terrain_t type);
