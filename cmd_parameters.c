/* Allows the user to define certain parameters of the program. If a parameter is not set, we stick with the default values */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern unsigned seed;

extern char single_parent; 
extern char flock;

extern unsigned steps;
extern unsigned rows;
extern unsigned columns;
extern double fitness;
extern unsigned ben_gen;
extern  char beneficial;
extern unsigned init_ben;

extern unsigned samples;

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
		
		/* map size */
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
		
		if ( (!strcmp(argv[i], "-bene" ) ) ){
			beneficial = 1;
			ben_gen = atoi(argv[++i]);
			init_ben = atoi(argv[++i]);
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