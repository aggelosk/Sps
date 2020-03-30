#include "create_map.h"

time_t t;

char input = 0; /* flags whether any input file has been given */

int xaxis = 0;

init_pop * pop_h;

topology * head_m = NULL;	/* for getting migration rate from file(s) */
topology * head_f = NULL;	/* for getting friction from file(s) */
topology * head_g = NULL;	/* for getting growth rate from file(s) */
topology * head_c = NULL;	/* for getting capacity from file(s) */


extern spot **** A;
extern unsigned steps;
extern unsigned rows;
extern unsigned columns;

extern unsigned long int total_people;

extern void print_massmig(char * filename, unsigned ev);
extern void parser();

double total_fric(unsigned i, unsigned j, unsigned gen){
	double total_fric = 0;
	int x, y;
	for (x = -1; x < 2; ++x){
		if (  i + x >= 0 && i + x < rows ){ /* caution check so that we don't wander outside the array borders */
			for (y = -1; y < 2; ++y){
				if (  j + y >= 0 && j + y < columns && (x != 0 || y !=0 ) )
					total_fric += A[gen][i + x][j + y] -> friction;
			}
		}
	}
	return total_fric;
}

void show_the_model(unsigned gen){
	unsigned i, j;
	FILE * f1 = fopen("kappa.txt", "a");
	for (i = 0; i < rows; ++i){
		for (j = 0; j < columns; ++j)
			fprintf(f1, "[%lf %u %lf - %lf] " , A[gen][i][j] -> migrate, A[gen][i][j] -> capacity,  A[gen][i][j] -> friction, A[gen][i][j] -> total_fric);
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
	unsigned i, j, s;
	#pragma omp parallel for
	for ( i = 0; i < r; ++i){
		#pragma omp parallel for
		for (j = 0; j < c; ++j){
			for (s = 0; s < steps; ++s)
				set_spot(s, i, j);
		}
	}
	if (input){ /* if the migration, friction and capacity rates of the map are given */
		if (head_m != NULL)
			parser_migr();
		if (head_f != NULL)
			parser_fric();
		if (head_g != NULL)
			parser_grth();
		if (head_c != NULL)
			parser_cpct();

	}
	double tf = 0.0;
	for ( i = 0; i < rows; ++i){
		for (j = 0; j < columns; ++j){
			for (s = 0; s < steps; ++s)
				A[s][i][j] -> total_fric = total_fric(i, j, s);
		}
	}
}

void init_population(unsigned i, unsigned j, unsigned population){ /* a single population in a single area */
	A[0][i][j] -> population = population;
	A[0][i][j] -> pop_num = 0;
	A[0][i][j] -> people = malloc(population * sizeof(person));
	int counter;
	for (counter = 0; counter < population; ++counter){
		A[0][i][j] -> people[counter].pid = ++total_people; /* first person takes the id 0 etc */
		A[0][i][j] -> people[counter].flag = 0;
		A[0][i][j] -> people[counter].fitness = 1 + counter;
		A[0][i][j] -> people[counter].parent1 = NULL;
		A[0][i][j] -> people[counter].parent2 = NULL;
		A[0][i][j] -> people[counter].row = i;
    A[0][i][j] -> people[counter].col = j;
	}
}

void allocate(){ /* dynamically allocates the array */
	int i, j;
	A = malloc( sizeof(spot**) * steps );
	for (i = 0; i < steps; ++i){
		A[i] = malloc( sizeof(spot*) * rows );
		for (j = 0; j < rows; ++j)
			A[i][j] = malloc( sizeof(spot) * columns );
	}
}



void create_map(){
	allocate();
	set_terrain(rows, columns);
	init_pop * tmp = NULL;
	if (!xaxis) /* not set by user thus randomly chosen */
		init_population(rand() % (rows - 1), rand() % (columns - 1), 5);
	else{
		while ( pop_h != NULL){
			init_population(pop_h -> lat, pop_h -> lon, pop_h -> ppl);
			tmp = pop_h;
			pop_h = pop_h -> next;
			free(tmp);
		}

	}
	//printf("\n MAP CREATED \n");
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	The first major part of the project. As the name suggests it creates the whole map, by giving each individual area it's respective aspects. After that, it initializes the - one or many - populations
	to certain areas of the map thus creating the generation 0.
*/
