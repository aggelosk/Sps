#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

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
	unsigned xaxis;
	unsigned yaxis;
}leader;

typedef struct person{
	unsigned pid;
	char flag; /* boolean that shows if the person reproduced or not */
	float fitness; 
	struct person * parent1;
	struct person * parent2;
	struct person * next; /* functions as a way to initialize the list of people in an area */
}person;

typedef enum predator_type{
	predator,
	prey,
	hybrid /* middle of the food chain, preys on some species is preyd upon by others */
}predator_type;

typedef struct predators{
	species * current; /* pointer to the predator */
	double ratio;
	struct predators * next;
}predators;

typedef struct preys{ 
	species * current; /* pointer to the prey */
	double ratio;
	struct preys * next;
}preys;

typedef struct  species{
	char * name;
	predator_type * type;
	predators * dies; 							/* species that kill it*/
	preys *	kill;								   	/* species that it feasts upon */
	/* for preys it is used to determine which preys are competing with it */
	/* --- area related info --- */
	int pop_num;								/* shows which population this belongs to */
	unsigned spec_capacity;				/* unique area_capacity for the current species */
	unsigned long int population; 		/* number of people in the area */
	unsigned long int fit_people;
	person * people;							 /* a list of every person in the area */
	unsigned long int incoming; 		/* people about to migrate to this area */
	person * immigrants;
	
	struct species * next; /* for keeping every species in an area*/
}species;

typedef struct spot{
	double latitude; 		/* y axis */
	double longtitude;	/* x axis */
	double altitude;		/* might be under sea level so negative values also work */
	terrain_t type;
	
	species * spec;			/* species inhabiting the area */
	unsigned capacity;  /* max population possible */
	double growth_rate;
	double migrate; 	/* probability to leave the area ~ so 0 - 1 values */
	double friction;	 	/* probability to enter the area ~ sort of an area quality check */
}spot;

void area_config(unsigned i, unsigned j);

void * set_spot(unsigned ev, unsigned i, unsigned j, double latitude, double longtitude, double altitude, terrain_t type);