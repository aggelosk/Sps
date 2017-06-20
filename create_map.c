#include "create_map.h"

time_t t;

extern spot **** A;
extern unsigned steps;
extern unsigned rows;
extern unsigned columns;

extern unsigned long int total_people;

void set_dimentions(unsigned r, unsigned c){
	rows = r;
	columns = c;
}

void set_terrain(unsigned r, unsigned c){
	set_dimentions(r, c);
	int i, j, s, chance;
	double latitude, longtitude, altitude;
	terrain_t type;
	for ( i = 0; i < r; i++){
		for (j = 0; j < c; j++){
			latitude = rand() % 500000;
			latitude = latitude / 1000;
			longtitude = rand() % 500000;
			longtitude = longtitude / 1000;
			chance = rand() % 5;
			if (chance){
				type = land;
				altitude = rand() % 8848;
			} else {
				type = sea;
				altitude = 0;
			}
			for (s = 0; s < steps; s++)
				set_spot(s, i, j, latitude, longtitude, altitude, type);
		}
	}
}

void init_population(unsigned i, unsigned j, unsigned population){ /* a single population in a single area */
	A[0][i][j] -> population = population;
	A[0][i][j] -> pop_num = 0;
	person * tmp;
	int counter;
	for (counter = 0; counter < population; counter++){
		tmp = malloc(sizeof (person));
		tmp -> pid = total_people; /* first person takes the id 0 etc */
		tmp -> flag = 0;
		tmp -> fitness = 0;
		tmp -> parent1 = NULL;
		tmp -> parent2 = NULL;
		tmp -> next = A[0][i][j] -> people;
		A[0][i][j] -> people = tmp;
		total_people++;
	}

}

void allocate(){ /* dynamically allocates the array */
	int i, j;
	A = malloc( sizeof(spot**) * steps );
	for (i = 0; i < steps; i++){
		A[i] = malloc( sizeof(spot*) * rows );
		for (j = 0; j < rows; j++)
			A[i][j] = malloc( sizeof(spot) * columns );
	}
}

void create_map(){
	allocate();
	set_terrain(rows, columns);
	int xaxis = rand() % (rows - 1);
	int yaxis = rand() % (columns - 1);
	int population = 15;//(rand() % 80) + 20;
	init_population(xaxis , yaxis, population);
	printf("\n MAP CREATED \n");
	/* in order to allow multiple populations a pop_type(string not enum) parameter is required */
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The first major part of the project. As the name suggests it creates the whole map, by giving eac individual area it's respective aspects. After that, it initializes the - one or many - populations
	to certain areas of the map thus creating the generation 0.
*/