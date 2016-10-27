#include "spot.h" 

spot **** A; /* initializing the 3D array */

time_t t;

extern void create_map();
extern void forward_time();
extern void rewind_time();
extern void mutate();

spot * set_spot(double latitude, double longtitude, double altitude, terrain_t type){
	spot * tmp;
	tmp = malloc(sizeof(spot));
	tmp -> latitude = latitude;
	tmp -> longtitude = longtitude;
	tmp -> altitude = altitude;
	tmp -> type = type;
	tmp -> capacity = 100;
	tmp -> growth_rate = 1;						
	tmp -> impact = 1;								
	tmp -> migrate = 0.5;
	tmp -> friction = 10;
	tmp -> extra_weight = 0;
	tmp -> population = 0;
	tmp -> people = NULL;
	tmp -> incoming = 0;
	tmp -> immigrants = NULL;
	return tmp;
}

int main(){
	srand((unsigned) time(&t));
	create_map();
	forward_time();
	rewind_time();
	mutate();
	printf ("\n ALL DONE \n");
	return 0;
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The main file of the whole project! The ".h" contains the structures required for each area of the map as well as the structure for each person. The ".c" does nothing much by itself, besides 
	setting the parameters for each area. It calls, in the right order, each of the 4 main parts of the project.
*/