#include "spot.h"

spot **** A; /* initializing the 3D array */

time_t t;
gsl_rng * r;

double sigma = 1.0;
double theta = -1.0;	 			/* input for the Poisson distribution */
unsigned seed = 0;
extern unsigned steps;
extern unsigned rows;
extern unsigned columns;

extern char input;

extern unsigned total;

unsigned capacity = 0;
double migration_rate = 0.1;
double friction = -1;
double growth = -1;

extern topology * head_m;
extern topology * head_f;
extern topology * head_g;
extern topology * head_c;
char * filename = " ";

extern void cmd_params(int argc, char** argv);
extern void create_map();
extern void forward_time();
extern void rewind_time();
extern void mutate();

extern void destruction(); /* check for leaks before the rewind step. TO BE REMOVED */

void parser_migr(){
	FILE * f1 = NULL;
	topology * tmp = NULL;
	while (head_m != NULL){
		f1 = fopen(head_m -> filename, "r");
		assert( f1 != NULL); /* should only be triggered when wrong input file is given */
		unsigned i, j, s;
		for (i = 0; i < rows; ++i){
			for (j = 0; j < columns; ++j){
				//printf("\n[%d,%d, %d] \n", i, j, head_m -> gen);
				fscanf(f1, "%f", &A[head_m -> gen][i][j] -> migrate);
				if (head_m -> next != NULL){ /* meaning after a certain generation a different scenario applies */
					for (s = head_m -> gen + 1; s < head_m -> next -> gen; ++s)
						A[s][i][j] -> migrate = A[head_m -> gen][i][j] -> migrate;
				}
				else{
					for (s = head_m -> gen + 1; s < steps; ++s)
						A[s][i][j] -> migrate = A[head_m -> gen][i][j] -> migrate;
				}

			}
		}
		fclose(f1);
		tmp = head_m;
		head_m = head_m -> next;
		free(tmp -> filename);
		free(tmp);
	}
}

void parser_fric(){
	FILE * f1 = NULL;
	topology * tmp = NULL;
	while (head_f != NULL){
		f1 = fopen(head_f -> filename, "r");
		assert( f1 != NULL); /* should only be triggered when wrong input file is given */
		unsigned i, j, s;
		for (i = 0; i < rows; ++i){
			for (j = 0; j < columns; ++j){
				fscanf(f1, "%f", &A[head_f -> gen][i][j] -> friction);
				if (head_f -> next != NULL){ /* meaning after a certain generation a different scenario applies */
					for (s = head_f -> gen + 1; s < head_f -> next -> gen; ++s)
						A[s][i][j] -> friction = A[head_f -> gen][i][j] -> friction;
				}
				else{
					for (s = head_f -> gen + 1; s < steps; ++s)
						A[s][i][j] -> friction = A[head_f -> gen][i][j] -> friction;
				}

			}
		}
		fclose(f1);
		tmp = head_f;
		head_f = head_f -> next;
		free(tmp -> filename);
		free(tmp);
	}
}

void parser_grth(){
	FILE * f1 = NULL;
	topology * tmp = NULL;
	while (head_g != NULL){
		f1 = fopen(head_g -> filename, "r");
		assert( f1 != NULL); /* should only be triggered when wrong input file is given */
		unsigned i, j, s;
		for (i = 0; i < rows; ++i){
			for (j = 0; j < columns; ++j){
				fscanf(f1, "%lf", &A[head_g -> gen][i][j] -> growth_rate);
				if (head_g -> next != NULL){ /* meaning after a certain generation a different scenario applies */
					for (s = head_g -> gen + 1; s < head_g -> next -> gen; ++s)
						A[s][i][j] -> growth_rate = A[head_g -> gen][i][j] -> growth_rate;
				}
				else{
					for (s = head_g -> gen + 1; s < steps; ++s)
						A[s][i][j] -> growth_rate = A[head_g -> gen][i][j] -> growth_rate;
				}

			}
		}
		fclose(f1);
		tmp = head_g;
		head_g = head_g -> next;
		free(tmp -> filename);
		free(tmp);
	}
}

void parser_cpct(){
	FILE * f1 = NULL;
	topology * tmp = NULL;
	while (head_c != NULL){
		f1 = fopen(head_c -> filename, "r");
		assert( f1 != NULL); /* should only be triggered when wrong input file is given */
		unsigned i, j, s;
		for (i = 0; i < rows; ++i){
			for (j = 0; j < columns; ++j){
				fscanf(f1, "%u", &A[head_c -> gen][i][j] -> capacity);

				if (head_c -> next != NULL){ /* meaning after a certain generation a different scenario applies */
					for (s = head_c -> gen + 1; s < head_c -> next -> gen; ++s)
						A[s][i][j] -> capacity = A[head_c -> gen][i][j] -> capacity;
				}
				else{
					for (s = head_c -> gen + 1; s < steps; ++s)
						A[s][i][j] -> capacity = A[head_c -> gen][i][j] -> capacity;
				}

			}
		}
		fclose(f1);
		tmp = head_c;
		head_c = head_c -> next;
		free(tmp -> filename);
		free(tmp);
	}
}

void area_config(unsigned i, unsigned j){
	double counter = 0;
	double cap = 0;
	double mig = 0;
	double frc = 0;
	double grt = 0;
	A[0][i][j] -> capacity = 50;
	A[0][i][j] -> growth_rate = 0.2;
	A[0][i][j] -> friction = 0.5;
	A[0][i][j] -> migrate = 0.5;
	return;
}

void set_spot(unsigned ev, unsigned i, unsigned j){
	A[ev][i][j] = malloc(sizeof(spot));
	if (!ev){
		area_config(i,j);
		if (capacity > 0) /* capacity user defined */
			A[ev][i][j] -> capacity = capacity;
		if (migration_rate >= 0) /* migration rate user defined */
			A[ev][i][j] -> migrate = migration_rate;
		if (friction >= 0) /* friction user defined */
			A[ev][i][j] -> friction = friction;
		if (growth >= 0)
			A[ev][i][j] -> growth_rate = growth;
	}
	else{
		A[ev][i][j] -> capacity = A[0][i][j] -> capacity;
		A[ev][i][j] -> growth_rate = A[0][i][j] -> growth_rate;
		A[ev][i][j] -> migrate = A[0][i][j] -> migrate;
		A[ev][i][j] -> friction = A[0][i][j] -> friction;
	}
	A[ev][i][j] -> total_fric = 0;
	A[ev][i][j] -> fit_people = 0;
	A[ev][i][j] -> pop_num = -1;
	A[ev][i][j] -> population = 0;
	A[ev][i][j] -> people = NULL;
	A[ev][i][j] -> incoming = 0;
	A[ev][i][j] -> immigrants = NULL;
}

int main(int argc, char** argv){
	seed = (unsigned) time(&t);

	cmd_params(argc, argv);

	// clock_t start = clock();
	srand(seed);
	const gsl_rng_type * T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	gsl_rng_set(r, seed);

	create_map();

	forward_time();

	rewind_time();

	int nmut = -1;
	if (theta != -1)
		nmut = (int)gsl_ran_poisson(r, theta * total);
	mutate(nmut);

	gsl_rng_free(r);

	// clock_t end = clock();
	// FILE * f1 = fopen("seedtimesps.txt", "w");
	// float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	// fprintf(f1,"%d %f\n", seed, seconds);
	// fclose(f1);
	return 0;
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The main file of the whole project! The ".h" contains the structures required for each area of the map as well as the structure for each person. The ".c" does nothing much by itself, besides
	setting the parameters for each area. It calls, in the right order, each of the 4 main parts of the project.
*/
