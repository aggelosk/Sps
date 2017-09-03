#include "create_map.h"

time_t t;

char input = 0; /* flags whether an input file has been given */

int xaxis = -1;
int yaxis = -1;

extern spot **** A;
extern unsigned steps;
extern unsigned rows;
extern unsigned columns;

extern unsigned long int total_people;

extern void print_massmig(char * filename, unsigned ev);
extern void parser();

void show_the_model(){
	unsigned i, j;
	FILE * f1 = fopen("kappa.txt", "w");
	for (i = 0; i < rows; i++){
		for (j = 0; j < columns; j++)
			fprintf(f1, "[%lf %lf] ", A[0][i][j] -> migrate, A[0][i][j] -> friction);
		fprintf(f1, "\n");
	}
	fclose(f1);
}


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
			for (s = 0; s < steps; s++){
				set_spot(s, i, j, latitude, longtitude, altitude, type);
			}
		}
	}
	if (input) /* if the migration and friction rates of the map are given */
		parser();
	show_the_model();
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
	//A[0][i][j] -> fitness = population;
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
	if (xaxis == -1){ /* not set by user thus randomly chosen */
		xaxis = rand() % (rows - 1);
		yaxis = rand() % (columns - 1);
	}
	int population = 5;//(rand() % 80) + 20;
	init_population(xaxis , yaxis, population);
	printf("\n MAP CREATED \n");
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The first major part of the project. As the name suggests it creates the whole map, by giving each individual area it's respective aspects. After that, it initializes the - one or many - populations
	to certain areas of the map thus creating the generation 0.
*/