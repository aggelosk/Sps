/* Allows the user to define certain parameters of the program. If a parameter is not set, we stick with the default values */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* from spot.c */
extern unsigned seed;

/* from create_map.c */
extern char input;
extern char * filename;
extern char capac;
extern unsigned capacity;
extern int xaxis;
extern int yaxis;



/* from forward_time.c */
extern char single_parent; 
extern char flock;

extern unsigned steps;
extern unsigned rows;
extern unsigned columns;
extern double fitness;
extern double ben_chance;
extern char beneficial;


/* from rewind_time.c */
extern unsigned samples;

/* parameters for Poisson and Gaussian */
extern double sigma;
extern double theta;

void cmd_params(int argc, char** argv){
	int i;
	for (i = 1; i < argv[i]; i++){
		
		if ( (!strcmp(argv[i], "-flok" ) ) ){
			flock = 1;
			continue;
		}
		
		if ( (!strcmp(argv[i], "-sngp" ) ) ){
			single_parent = atoi(argv[++i]);
			continue;
		}
		
		/* map start */
		if ( (!strcmp(argv[i], "-gens" ) ) ){
			steps = atoi(argv[++i]);
			continue;
		}
		if ( (!strcmp(argv[i], "-rows" ) ) ){
			rows = atoi(argv[++i]);
			continue;
		}
		if ( (!strcmp(argv[i], "-cols" ) ) ){
			columns = atoi(argv[++i]);
			continue;
		}
		
		/* area for initial population */
		if( (!strcmp(argv[i], "-strt" ) ) ){
			xaxis = atoi(argv[++i]);
			yaxis = atoi(argv[++i]);
			assert(xaxis < rows && yaxis < columns);
			continue;
		}

		if ( (!strcmp(argv[i], "-inpt" ) ) ){ /* checks whether the user wishes to define the capacity of each deme */
			input = 1;
			filename = malloc(sizeof(100));
			filename = strdup(argv[++i]);
			continue;
		}
		
		if ( (!strcmp(argv[i], "-cpct" ) ) ){
			capac = 1;
			capacity = atoi(argv[++i]);
			continue;
		}	
		/* map end */
		
		if ( (!strcmp(argv[i], "-bene" ) ) ){
			beneficial = 1;
			ben_chance = atoi(argv[++i]);
			fitness = atof(argv[++i]);
		}
		
		/* number of samples */
		if ( (!strcmp(argv[i], "-smps" ) ) ){
			samples = atoi(argv[++i]);
			continue;
		}
		
		/* seed for random */
		if ( (!strcmp(argv[i], "-seed" ) ) ){
			seed = atoi(argv[++i]);
			continue;
		}
		
		/* gaussian parameter */
		if ( (!strcmp(argv[i], "-gsnd" ) ) ){
			sigma = atof(argv[++i]);
			continue;
		}
		/* poisson parameter */
		if ( (!strcmp(argv[i], "-psnd" ) )){
			theta = atof(argv[++i]);
			continue;
		}
		
		/* number of samples for the rewind step */
		if ( (!strcmp(argv[i], "-smps" ) ) ){
			samples = atoi(argv[++i]);
			continue;
		}
		

		
		fprintf(stderr, "Argument %s is invalid\n\n\n", argv[i]);	
	}	
}