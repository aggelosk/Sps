#include "forward_time.h"

#define current	A[event_number][i][j] 
#define nxtstep	A[event_number +1][i][j]

time_t t;

extern spot **** A;

unsigned event_number = 0; /* number of current event - shows the number of entries in the event table */
unsigned long int total_people = 0;

unsigned steps = 51;
unsigned rows = 100; 
unsigned columns = 100;
unsigned default_weight = 1;
/* ----- will be changed later on, for now they serve as testing "flexibilities" ----- */


unsigned * People; /* holds the population for each generation */

/* -------------------------------------------------------------------------------------------- */


/* -------------------------- Different scenario activators ------------------------------------- */
char single_parent = 1; 			/* default is to allows single parented children */
char multi_populations = 0;	/* default is not to allow such a thing */
char predator_prey = 0; 		/* default is not to allow such a thing ~ requires multiple populations */
char settlements = 0;				/* default is not to allow such a thing ~ requires humans as species */
char beneficial = 0;				/* default is not to allow such a thing */
/* ------------------------------------------------------------------------------------------------------ */



unsigned logistic_function(unsigned i, unsigned j){
	/* determines the reproduction of the population in the area */
	double x = 1 - ((double)current -> population / current -> capacity);
	double population = current -> growth_rate * current -> population  * x + current -> population;
	return (unsigned)population; /* cast into unsigned as a round - up measure */
}

void migration(unsigned i, unsigned j, person * child){
	/* will move the entity to the new area */
	unsigned total_fric = 0;
	unsigned curr_inc;
	int x, y;
	for (x = -1; x< 2; x++){ 	
		if (  i + x >= 0 && i + x < rows ){ /* caution check so that we don't wander outside the array borders */
			for (y = -1; y < 2; y++){
				if (  j + y >= 0 && j + y < columns && (x != 0 || y !=0 ) )
					total_fric = total_fric + A[event_number][i + x][j + y] -> friction;
			}
		}
	}
	int migrate = rand() % total_fric;
	unsigned counter = 0;
	for (x = -1; x< 2; x++){
		if ( i + x >= 0 && i + x < rows){ 
			for (y = -1; y < 2; y++){
				if ( j + y >= 0 && j + y < columns && (x != 0 || y !=0 ) ){
					if ( migrate - counter <= A[event_number][i + x][j + y] -> friction ){
						A[(event_number + 1)][i + x][j + y] -> immigrants = child;
						A[(event_number + 1)][i + x][j + y] -> incoming++;
						return;
					}
					else
						counter = counter + A[event_number][i + x][j + y] -> friction;
				}
			}
		}
	}
	assert(0); /* we should never arrive at this point of the code */
}

person * birth(unsigned i, unsigned j){
	person * child;
	child = malloc(sizeof (person));
	child -> pid = total_people; /* first person of each generation takes the id 0 etc */
	child -> xaxis = j;
	child -> yaxis = i;
	int p1 = rand() % (current -> population );
	int p2 = rand() % (current -> population );
	if (!single_parent){ /* in this case we can't allow single parents */
		while (p1 == p2)
			p2 = rand() % (current -> population);
	}
	unsigned ps = 0;
	person * tmp = current -> people;
	if ( p1 < p2){
		while (ps < p1 -1&& tmp -> next){
			tmp = tmp -> next;
			ps++;
		}
		child -> parent1 = tmp;
		while (ps < p2 - 1&& tmp -> next){
			tmp = tmp -> next;
			ps++;
		}
		child -> parent2 = tmp;
	}
	else if (p2 < p1){
		while (ps < p2 && tmp ->next){
			tmp = tmp -> next;
			ps++;
		}
		child -> parent2 = tmp;
		while (ps < p1 && tmp ->next){
			tmp = tmp -> next;
			ps++;
		}
		child -> parent1 = tmp;
	}
	else if (single_parent == 1){ /* only if we accept single-parented childer */
		while (ps < p2 && tmp ->next){
			tmp = tmp -> next;
			ps++;
		}
		child -> parent1 = tmp;
		child -> parent2 = tmp;
	}
	else
		assert(0); /* we should never arrive in this case at any point */
	/* we need to fixate the extra weight - beneficiality - of the newborn child If we allow such a thing */
	if (beneficial){
		unsigned weig;
		if (child -> parent1 -> weight ) {
			weig = rand() % 100;
			if ( child -> parent1 -> weight == 2 || weig > 5)
				child -> weight++;
		}
		if (child -> parent2 -> weight ) {
			weig = rand() % 100;
			if ( child -> parent2 -> weight == 2 || weig > 5)
				child -> weight++;
		}
	}
	assert(child); /* for safely purposes,should never be triggered*/
	return child;
}

unsigned long int reproduce(unsigned i, unsigned j){
	unsigned long int new_pop = 0;
	if (!predator_prey)	
		new_pop = logistic_function(i,j);
	unsigned mig_pop = 0;
	unsigned remaining = 0;
	/* we now need to decide the parents of each kid and the place of birth*/
	unsigned counter = 0;
	float migrate = 0;
	person * child;
	while (counter < new_pop ){
		child = birth(i,j);
		/* now we need to choose the destination of the newborn child */
		migrate = rand() / (float)RAND_MAX;
		if (migrate < current -> migrate){
			migration(i, j, child);
			mig_pop++;
		}
		else{
			child -> next = nxtstep -> people;
			nxtstep -> people = child;
		}
		counter++;
		total_people++;
	}
	return new_pop - mig_pop; /* the population of the area after births/migrations */
}

void intergrate_migration(unsigned i, unsigned j){
	current -> population += current -> incoming; /* it is done in this step of the process to avoid the  extra parse of the table */
	current -> incoming = 0;	/* already merged to the native population */
	if (current -> people){
		person * tmp;
		tmp = current -> people;
		while (tmp -> next != NULL)
			tmp = tmp -> next;
		tmp -> next = current -> immigrants;
	}
	else
		current -> people = current -> immigrants;
	current -> immigrants = NULL;
}

void event(){
	printf("Event %d\n", event_number);
	unsigned i, j;
	total_people = 0; /* each generation obviously starts with 0 people */
	for (i = 0; i < rows; i++){
		for (j = 0; j < columns; j++){
			if (current -> incoming > 0)	 /* if there are immigrants coming to this area */
				intergrate_migration(i,j);
			if (current -> population > 0) /* inhabitated areas only */
				nxtstep -> population = reproduce(i,j);
		}
	}
	People[++event_number] = total_people;
}

void print_people(){
	int i;
	for (i = 0; i <= event_number; i++)
		printf("event %d has %d people \n", i, People[i]);
}



/* sort of a main for this section */
void forward_time(){
	People = malloc(sizeof(unsigned) * steps);
	People[0] = total_people;
	unsigned i = 0;
	printf("%d\n", People[0]);
	while (i < steps - 1){
		if (People[event_number] == 0){
			printf(" \nThere are no people left so there is no point in continuing this simulation\n");
			break;
		}
		event();
		i++;
	}
	unsigned x, y;
	if (People[event_number] > 0){
		for (x = 0; x< rows; x++){
			for (y = 0; y < columns; y++){
				if (A[event_number][x][y] -> incoming > 0)
					intergrate_migration(x, y);
			}
		}
	}
	print_people();
	printf("\n FORWARD COMPLETE \n");
}

/* time complexity is O(steps * rows * columns) */

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	In this section of the project, the second one called, we receive an initialized instance of a map, along with the populations inhabiting some of its areas, and we begin going a number of steps forward in time, where each step equals to a new generation. For each of these steps, the population of each area reproduces and each of the children either inhabitates the same area as the parent, or one of the neighbouring areas.
*/