#include "forward_time.h"

#define current	A[event_number][i][j]
#define nxtstep	A[event_number +1][i][j]

time_t t;

extern spot **** A;

unsigned event_number = 0; 				 /* number of current event - shows the number of entries in the event table */
unsigned long int total_people = 0;			/* total number of people encountered in the forward in time step */
unsigned long int unique_id = 0;

unsigned steps = 101;
unsigned rows = 5;
unsigned columns = 5;
/* default values */

double ben_chance = 0.001;
unsigned init_ben = 10;
float fitness = 0.1;

unsigned long int no_children = 0;
unsigned pop_counter = 0;

unsigned cram = -1; 					/* at a certain generation the user may choose to prune the tree to reduce the total ram of the code */
ram * ram_h;

unsigned * People; 					/* holds the population for each generation */
leader * Leaders; 					/* for the mass - migration model. Shows which population tends to follow which */

extern char ancs; 					/* determines whether we use ancestral sampling */

extern void show_the_model(unsigned gen);

/* -------------------------------------------------------------------------------------------- */


/* -------------------------- Different scenario activators ------------------------------------- */
char single_parent = 1;							/* default is to allows single parented children */
char beneficial = 0;								/* default is to not allow such a thing */
char flock = 0;										/* default is not to allow such a thing */
/* ------------------------------------------------------------------------------------------------------ */

unsigned logistic_function(unsigned i, unsigned j){
	/* determines the reproduction of the population in the area */
	double x = 1.0 -  ( (double)current -> population / (double)current -> capacity ) ;
	if (current -> population > current -> capacity)
		return current -> population;
	double population = (current -> growth_rate * (double)current -> population  * x);
	population += (double)current -> population + 0.5;
	return (unsigned)population; /* cast into unsigned as a round - up measure (since we added the + 0.5 above) */
}

/* ------------------------------- BEFORE HERE WE ARE DONE ------------------------------- */

void migration(unsigned i, unsigned j, person * child){	/* will move the entity to the new area */
	assert(child != NULL);
	double fr = ((float)rand()/(float)(RAND_MAX)) * nxtstep -> total_fric;
	double counter = 0.0;
	int x,y;
	for (x = -1; x < 2; ++x){
		if ( i + x >= 0 && i + x < rows){
			for (y = -1; y < 2; ++y){
				if ( j + y >= 0 && j + y < columns && (x != 0 || y !=0 ) ){
					if ( fr == nxtstep -> total_fric || fr - counter <= A[event_number + 1][i + x][j + y] -> friction ){ /* we found the destination */
						i += x;
						j += y;
						child -> row = i;
						child -> col = j;
						/* the person migrates to the new area */
						if ( nxtstep -> immigrants == NULL){
							nxtstep -> immigrants = malloc(sizeof(person));
							memcpy(nxtstep -> immigrants, child, sizeof(person));
						}
						else{
							 nxtstep -> immigrants = realloc( nxtstep -> immigrants, (nxtstep -> incoming + 1) * sizeof(person) );
							 memcpy(&nxtstep -> immigrants[nxtstep -> incoming], child, sizeof(person));
						}
						++nxtstep -> incoming;
						return;
					}
					else
						counter = counter + A[event_number + 1][i + x][j + y] -> friction;
				}
			}
		}
	}
	printf("res: %lf\n", fr - counter);
	printf("busted in event %d from: [%d %d] going for: [%d %d] \n", event_number, i,j, x, y);
	assert(0); /* we should never arrive at this point of the code */
}

person * birth(unsigned i, unsigned j, person * child){
	//#pragma atomic
	child -> pid = ++unique_id; /* first person of each generation takes the id 0 etc */
	child -> flag = '0';
	child -> fitness = 0;
	child -> parent1 = NULL;
	child -> parent2 = NULL;
	child -> row = i;
	child -> col = j;

	unsigned p1 = rand() % current -> population;
	unsigned p2 = rand() % current -> population;
	while (single_parent == 0 && p2 == p1)
		p2 = rand() % current -> population;
	child -> parent1 = &current -> people[p1];
	child -> parent2 = &current -> people[p1];
	//}
	unsigned ff = p2;
	if ( (float)rand() / RAND_MAX < 0.5)
		ff = p1;
	child -> fitness = current -> people[ff].fitness;
	if (ff != 0)
		child -> fitness -= current -> people[ff - 1].fitness;

	assert(child); /* just a safety clause, should never be triggered */
	return child;
}


/* --------------------- mass migration end ------------------- */

unsigned long int reproduce(unsigned i, unsigned j){
	assert(current -> population != 0);
	unsigned long int new_pop = 0;
	new_pop = logistic_function(i,j);
	if (new_pop > 0)
		nxtstep -> people = malloc(new_pop * sizeof(person));
	else
		assert(0);
/*	if (event_number >= 1000 && j != 0)
		fprintf(stderr, "old %u, new pop [%u %u %u] %lu \n", current->population, event_number, i, j, new_pop); */
	unsigned mig_pop = 0;
	/* we now need to decide the parents of each kid and the place of birth*/
	unsigned pos = 0;
	float migrate = 0.0;
	unsigned counter;
	for (counter = 0; counter < new_pop; ++counter ){
		nxtstep -> people[pos] = *birth(i, j, &nxtstep -> people[pos]);
		/* cumulative fitness allows for easier parental selection in the next generation */
		if (pos != 0)
			nxtstep -> people[pos].fitness += nxtstep -> people[pos - 1].fitness;
		/* now we need to choose the destination of the newborn child */
		migrate = rand() / (float)RAND_MAX;
		if ( ( migrate < current -> migrate ) && ( nxtstep -> total_fric > 0) ){
			migration(i, j, &nxtstep -> people[pos]);
			++mig_pop;
		}
		else
			++pos;
	}
	total_people += new_pop;
	if (mig_pop > 0)
		nxtstep -> people = realloc(nxtstep -> people, sizeof(person) * (new_pop - mig_pop) );
	return new_pop - mig_pop; /* the population of the area after births/migrations */
}

void intergrate_migration(unsigned i, unsigned j){
	/* we merge the two populations */
	if (current -> population == 0){
		current -> people = malloc(sizeof(person) * current -> incoming);
		memcpy(current -> people, current -> immigrants,  current -> incoming * sizeof(person));
	}
	else{
		current -> people = realloc(current -> people, sizeof(person) * (current -> population + current -> incoming) );
		memcpy(&current -> people[current -> population], current -> immigrants,  (current -> incoming) * sizeof(person));
	}

	/* fix the fitness of the migrants */
	unsigned z;
	for (z = current -> population; z < current -> population + current -> incoming; ++z){
		if ( z != 0)
			current -> people[z].fitness += current -> people[z-1].fitness;
	}

	/* fix the indexes */
	current -> population += current -> incoming; /* it is done in this step of the process to avoid the  extra parse of the table */
	current -> incoming = 0;	/* already merged to the native population */

	/* no more need for this as the people are not considered immigrants anymore */
	free(current -> immigrants);
}

/* --------------------------------- AFTER HERE WE ARE DONE --------------------------------- */

void event(){
//	if (event_number  % 500 == 0) /* just to track the process of the simulation */
	unsigned i, j;
	total_people = 0; /* each generation obviously starts with 0 people */
	//#pragma omp parallel for
	for (i = 0; i < rows; ++i){
		//#pragma omp parallel for
		for (j = 0; j < columns; ++j){
			if (current -> incoming > 0)	 /* if there are immigrants coming to this area */
				intergrate_migration(i,j);
			if (current -> population > 0 && nxtstep -> capacity > 0){ /* inhabitated areas only */
				// if (flock)
				// 	nxtstep -> population = mass_migration(i,j);
				// else
					nxtstep -> population = reproduce(i,j);
			}
/*			else if (flock && current -> pop_num != -1)
				nxtstep -> pop_num = current -> pop_num; */
		}
	}
	People[++event_number] = total_people;
}

void print_people(){
	unsigned i;
	unsigned total = 0;
	for (i = 0; i <= event_number; ++i){
		total += People[i];
		printf("event %d has %d people \n", i, People[i]);
	}
	printf ("Total People: %d\n", total);
}

void print_population(){ /* prints the people in each area */
	FILE * f1 = fopen("population.txt", "a");
	unsigned i;
	unsigned j = 0;
	unsigned x = 0;
	fprintf(f1, "Event: %d\n", event_number);
	for (i = 0; i < rows; ++i){
		for (j = 0; j < columns; ++j){
			for (x = 0; x < current -> population; ++x)
				fprintf(f1, "%u %u ", current -> people[x].parent1 -> pid, current -> people[x].parent2 -> pid);
		}
		fprintf(f1, "\n");
	}
	fprintf(f1, "-----\n");
	fclose(f1);
}

void destruction(){ /* frees all the allocated memory */
	unsigned e, i, j;
	person * tmp = NULL;
	for ( e = 0; e <= event_number; ++e){
	  for (i = 0; i < rows; ++i){
	    for (j = 0; j < columns; ++j){
	      	free(A[e][i][j] -> people);
      		free(A[e][i][j]);
			}
	    free(A[e][i]);
	  }
	  free(A[e]);
	}
	free(A);
	free(People);
}

/* sort of a main for this section */
void forward_time(){
	People = malloc(sizeof(unsigned) * steps);
	People[0] = total_people;
	unsigned i;
	/* Leaders = malloc(  (rows * columns) * sizeof(leader) );
	for (i = 0; i < rows * columns; ++i){
		Leaders[i].lead = -1;
		Leaders[i].xaxis = 0;
		Leaders[i].yaxis = 0;
	}
	*/

	for (i = 0; i < steps - 1; ++i){
		if (People[event_number] == 0){
			printf(" \nThere are no people left so there is no point in continuing this simulation\n");
			break;
		}
		event();
	//	print_population();
	}

	unsigned x, y;
	if (People[event_number] > 0){
		for (x = 0; x< rows; ++x){
			for (y = 0; y < columns; ++y){
				if (A[event_number][x][y] -> incoming > 0)
					intergrate_migration(x, y);
			}
		}
	}
}
