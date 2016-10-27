#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

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

typedef struct person{
	unsigned pid;
	int xaxis;
	int yaxis;
	int weight; /* default 0 ~ If the child is beneficial, we add the extra weight into consideration */
	struct person * parent1;
	struct person * parent2;
	/* the number of parents can be easily changed */
	/* in rewind step we are gonna need more parameters here */
	struct person * next; /* functions as a way to initialize the list of people in an area */
}person;

typedef struct spot{
	double latitude; 		/* y axis */
	double longtitude;	/* x axis */
	double altitude;		/* might be under sea level so negative values also work */
	terrain_t type;
	
	unsigned long int capacity;  /* max population possible */
	double growth_rate;
	double impact; /* after which point does the growth_rate skyrocket */

	//species * sp; /* header to a list with all the species in an area ~ init NULL ~ only used for predator - prey cases */
	
	double migrate; /* probability to leave the area ~ so 0 - 1 values */
	double friction;	 /* probability to enter the area ~ sort of an area quality check */
	
	unsigned long int population; /* number of people in the area */
	unsigned long int extra_weight; /* each "beneficial" person in an area adds an extra weight that needs to be concidered in reproduction */
	person * people; /* a list of every person in the area */
	unsigned long int incoming; /* people about to migrate to this area */
	person * immigrants;
}spot;

spot * set_spot(double latitude, double longtitude, double altitude, terrain_t type);