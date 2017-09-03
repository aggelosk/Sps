#include "spot.h" 

spot **** A; /* initializing the 3D array */

time_t t;
gsl_rng * r;

double sigma = 1.0;
unsigned seed = 0;
extern unsigned steps;
extern unsigned rows;
extern unsigned columns;

extern char input;

char capac = 0;
unsigned capacity = 20;

char * filename = " ";

extern void cmd_params(int argc, char** argv);
extern void create_map();
extern void forward_time();
extern void rewind_time();
extern void mutate();

void parser(){
	printf("K\nA\nP\nP\nA\n");
	FILE * f1 = fopen(filename, "r");
	FILE * f2 = fopen("yolo.txt", "w");
	assert( f1 != NULL); /* just a safety clause should never be triggered */
	unsigned i, j, s;
	for (i = 0; i < rows; i++){
		for (j = 0; j < columns; j++){
			 fscanf(f1, "%lf", &A[0][i][j] -> migrate);
			 fprintf(f2, "[%lf ", A[0][i][j] -> migrate);
			 fscanf(f1, "%lf", &A[0][i][j] -> friction);
			 fprintf(f2, "%lf] ", A[0][i][j] -> friction);
			for (s = 1; s < steps; s++){
				A[s][i][j] -> migrate = A[0][i][j] -> migrate;
				A[s][i][j] -> friction = A[0][i][j] -> friction;
			}
		}
		fprintf(f2, "\n");
	}
	fclose(f2);
	fclose(f1);
}

void area_config(unsigned i, unsigned j){
	double counter = 0;
	double cap = 0;
	double mig = 0;
	double frc = 0;
	double grt = 0;
	//if (i == 0 && j == 0){ /* top left corner of the map */
		A[0][i][j] -> capacity = 50;
		A[0][i][j] -> growth_rate = 0.5;		
			A[0][i][j] -> friction = 0.5;										
			A[0][i][j] -> migrate = 0.5;
		return;
	//}

	if (i > 0){ /* meaning we ain't at the first row, so we can check above */
			if (j > 0){ /* ain't in the first column either, so we can check left */
			  cap = A[0][i - 1][j - 1] -> capacity; 
				frc = A[0][i - 1][j - 1] -> friction;
				grt = A[0][i - 1][j - 1] -> growth_rate;
				mig = A[0][i - 1][j - 1] -> migrate;
				counter++;
			}
			cap += A[0][i - 1][j] -> capacity;
			frc += A[0][i - 1][j] -> friction;
			grt += A[0][i - 1][j] -> growth_rate;
			mig += A[0][i - 1][j] -> migrate;
			counter++;
			if (j < rows  - 1 ){ /* ain't in the last column, so we can check right */
				cap += A[0][i - 1][j + 1] -> capacity;
				frc += A[0][i - 1][j + 1] -> friction;
				grt += A[0][i - 1][j + 1] -> growth_rate;
				mig += A[0][i - 1][j + 1] -> migrate;
				counter++;
			}
	}

	if (j > 0){
		cap += A[0][i ][j - 1] -> capacity;
		frc += A[0][i][j - 1] -> friction;
		grt += A[0][i][j - 1] -> growth_rate;
		mig += A[0][i][j - 1] -> migrate;
		counter++;
	}
	cap = cap/counter;
	frc = frc/counter;
	grt = grt/counter;
	mig = mig/counter;
	
	double tmp = -1.0; 	/* the minimum capacity is 1, to allow potential migrants to go through a cell */
	while(tmp < 1 ) 
	  tmp = gsl_ran_gaussian(r, sigma) + cap + 0.5;		/* the 0.5 added is in order for proper rounding during the unsigned typecast to happen */
	A[0][i][j] -> capacity = (unsigned)tmp;	
	tmp = -1.0;
	while (tmp <= 0 && tmp > 1)
		tmp = gsl_ran_gaussian(r, sigma) + frc + 0.5;
	A[0][i][j] -> friction = (unsigned)tmp;
	tmp = -1.0;
	while (tmp < 0 || tmp > 0.3 )
		tmp = gsl_ran_gaussian(r, sigma) + grt;
	A[0][i][j] -> growth_rate = tmp;				
	tmp = -1.0;
	while (tmp < 0 || tmp > 1)
		tmp = gsl_ran_gaussian(r, sigma) + mig;
	A[0][i][j] -> migrate = tmp;
	
}

void set_spot(unsigned ev, unsigned i, unsigned j, double latitude, double longtitude, double altitude, terrain_t type){
	A[ev][i][j] = malloc(sizeof(spot));
	A[ev][i][j] -> latitude = latitude;
	A[ev][i][j] -> longtitude = longtitude;
	A[ev][i][j] -> altitude = altitude;
	A[ev][i][j] -> type = type;
	if (!ev){
		area_config(i,j);
		if (capac)
			A[ev][i][j] -> capacity = capacity;
	}
	else{
		A[ev][i][j] -> capacity = A[0][i][j] -> capacity;
		A[ev][i][j] -> growth_rate = A[0][i][j] -> growth_rate;
		A[ev][i][j] -> migrate = A[0][i][j] -> migrate;
		A[ev][i][j] -> friction = A[0][i][j] -> friction;
	}
	A[ev][i][j] -> pop_num = -1;
	A[ev][i][j] -> population = 0;
	A[ev][i][j] -> fit_people = 0;
	A[ev][i][j] -> people = NULL;
	A[ev][i][j] -> incoming = 0;
	A[ev][i][j] -> immigrants = NULL;
}

int main(int argc, char** argv){  
    seed = (unsigned) time(&t);
	cmd_params(argc, argv);

	clock_t start = clock();
	srand(seed);
	const gsl_rng_type * T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc (T);
	gsl_rng_set(r, seed);

	create_map();
	forward_time();
	rewind_time();
	mutate(500);
	
	gsl_rng_free (r);
	clock_t end = clock();
	FILE * f1 = fopen("seedtimesps.txt", "a");
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	
	fprintf(f1,"----------------------\n seed: %d in %f seconds \n",seed, seconds);
	fclose(f1);
	printf ("\n ALL DONE in %f \n", seconds);
	return 0;
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The main file of the whole project! The ".h" contains the structures required for each area of the map as well as the structure for each person. The ".c" does nothing much by itself, besides 
	setting the parameters for each area. It calls, in the right order, each of the 4 main parts of the project.
*/
