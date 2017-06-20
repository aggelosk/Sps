#include "spot.h" 

spot **** A; /* initializing the 3D array */

time_t t;
gsl_rng * r;

double sigma = 1.0;
unsigned seed = 0;
extern unsigned rows;
unsigned spec_num = 1;

extern void cmd_params(int argc, char** argv);
extern void create_map();
extern void forward_time();
extern void rewind_time();
extern void mutate();

void area_config(unsigned i, unsigned j){
	double counter = 0;
	double cap = 0;
	double mig = 0;
	double frc = 0;
	double grt = 0;
	if (i == 0 && j == 0){ /* top left corner of the map */
		A[0][i][j] -> capacity = 20;
		A[0][i][j] -> friction = 1;
		A[0][i][j] -> growth_rate = 0.5;												
		A[0][i][j] -> migrate = 0.5;
		return;
	}
	if (i > 0){ /* meaning we ain't at the first row, so we can check above */
			if (j > 0){ /* ain't in the first column either, so we can check left */
				cap += A[0][i - 1][j - 1] -> capacity;
				frc += A[0][i - 1][j - 1] -> friction;
				grt += A[0][i - 1][j - 1] -> growth_rate;
				mig += A[0][i - 1][j - 1] -> migrate;
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
	A[0][i][j] -> capacity = 20;//(unsigned)gsl_ran_gaussian_pdf(cap, sigma);
	A[0][i][j] -> friction = 1;//gsl_ran_gaussian_pdf(fri, sigma);
	A[0][i][j] -> growth_rate = 0.5;//gsl_ran_gaussian_pdf(grt, sigma);											
	A[0][i][j] -> migrate = 0.5;//gsl_ran_gaussian_pdf(mig, sigma);
}

void set_spot(unsigned ev, unsigned i, unsigned j, double latitude, double longtitude, double altitude, terrain_t type){
	A[ev][i][j] = malloc(sizeof(spot));
	A[ev][i][j] -> latitude = latitude;
	A[ev][i][j] -> longtitude = longtitude;
	A[ev][i][j] -> altitude = altitude;
	A[ev][i][j] -> type = type;
	if (!ev)
		area_config(i,j);
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
	printf("seed: %d\n",seed) ;

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
	mutate();
	
	gsl_rng_free (r);
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	printf ("\n ALL DONE in %f \n", seconds);
	return 0;
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The main file of the whole project! The ".h" contains the structures required for each area of the map as well as the structure for each person. The ".c" does nothing much by itself, besides 
	setting the parameters for each area. It calls, in the right order, each of the 4 main parts of the project.
*/
