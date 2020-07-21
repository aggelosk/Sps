#include "forward_time.h"

#define current	A[event_number][i][j]
#define nxtstep	A[event_number +1][i][j]

time_t t;

extern spot **** A;

unsigned event_number = 0; 				 			/* number of current event - shows the number of entries in the event table */
unsigned long int total_people = 0;			/* total number of people encountered in the forward in time step */
unsigned long int unique_id = 0;

unsigned steps = 101;
unsigned rows = 5;
unsigned columns = 5;
/* default values */

double ben_chance = 0.001;
int ben_gen = 10;
char single_bene = '0'; 								/* whether we only have a single beneficial mutation or not ~ Default is not */
int gen_after_fix = -1; 								/* if this value takes a positive value, we keep on running # generation after fixation happens */
double fitness = 1.01;
short fixation = 0; 										/* check whether we have reached fixation */
double rec_rate = 0.01;									/* probability of recombination occuring at any given person */

unsigned long int no_children = 0;
unsigned pop_counter = 0;

unsigned cram = -1; 										/* at a certain generation the user may choose to prune the tree to reduce the total ram of the code */
ram * ram_h;

unsigned * People; 											/* holds the population for each generation */
leader * Leaders; 											/* for the mass - migration model. Shows which population tends to follow which */

extern char ancs; 											/* determines whether we use ancestral sampling */

extern void show_the_model(unsigned gen);

unsigned fit_everywhere = 0;
/* -------------------------------------------------------------------------------------------- */


/* ------------------------------ Different scenario activators ----------------------------------------- */
char single_parent = 1;							/* default is to allows single parented children */
char beneficial = 0;								/* default is to not allow such a thing */
char flock = 0;											/* default is not to allow such a thing */
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
						nxtstep -> incoming++;
						if (child -> fitness > 1)
							nxtstep -> fit_people++;
						return;
					}
					else
						counter = counter + A[event_number + 1][i + x][j + y] -> friction;
				}
			}
		}
	}
	fprintf(stderr, "res: %lf\n", fr - counter);
	fprintf(stderr, "busted in event %d from: [%d %d] going for: [%d %d] \n", event_number, i,j, x, y);
	assert(0); /* we should never arrive at this point of the code */
}

unsigned binary_search(unsigned i, unsigned j, unsigned l, unsigned r, double x){ /* Theta(logn) compexity of parental selection */
  if (r - l > 1) { /* array has at least a size of 3 */
      int mid = l + (r - l) / 2;
      /* If the element is present at the middle itself */
      if (current -> people[mid - 1].fitness < x && current -> people[mid + 1].fitness > x)
          return mid;
      /* If element is smaller than mid, then it can only be present in the left subarray */
      if (current -> people[mid].fitness > x)
          return binary_search(i, j, l, mid - 1, x);
      /* Else the element can only be present in the right subarray */
      return binary_search(i, j, mid + 1, r, x);
  }
	else if (r - l == 1){ /* either left or right is the correct answer so found it */
		if (current -> people[l].fitness >= x )
			return l;
		return r;
	}
	else /* single element so once again got it */
		return r;
}

person * birth(unsigned i, unsigned j, person * child){
	//#pragma atomic
	child -> pid = ++unique_id; /* first person of each generation takes the id 0 etc */
	child -> flag = '0';
	child -> fitness = 1;
	child -> parent1 = NULL;
	child -> parent2 = NULL;
	child -> row = i;
	child -> col = j;
	unsigned p1 = 0;
	unsigned p2 = current -> population + 1; /* invalid for now might be changed later */
	double recombination = ((float)rand()/(float)(RAND_MAX));
	if (current -> fit_people && current -> fit_people != current -> population){
		unsigned max = current-> population -1;
		double x = ((float)rand()/(float)(RAND_MAX)) * current -> people[max].fitness;
		p1 = binary_search(i, j, 0, max, x);
		if (recombination < rec_rate ){
			p2 = binary_search(i, j, 0, max, x);
			while (p2 == p1 && !single_parent)
				p2 = binary_search(i, j, 0, max, x);
			child -> parent2 = &current -> people[p2];
		}
	}
	else if (current -> fit_people == current -> population){
		fixation = 1;
		p1 = rand() % current -> population;
		if (recombination < rec_rate ){
			while (p2 == p1 && !single_parent)
				p2 = rand() % current -> population;
		}
	}
	else{
		p1 = rand() % current -> population;
		if (recombination < rec_rate ){
			while (p2 == p1 && !single_parent)
				p2 = rand() % current -> population;
		}
	}
	child -> parent1 = &current -> people[p1];

	/* now we need to calculate the child's fitness */
	if (recombination < rec_rate && current -> fit_people > 0 && current -> fit_people < current -> population){
		double f1 = 0;
		double f2 = 0;
		if (((float)rand()/(float)(RAND_MAX)) < 0.5){ /* fitness comes from the first parent */
			if (p1)
				f1 = child -> parent1 -> fitness - current -> people[p1-1].fitness; /* since fitness is cumulative */
			else
				f1 = child -> parent1 -> fitness;
			child -> fitness = f1;
		}
		else{
			if (p2)
				f2 = child -> parent2 -> fitness - current -> people[p2-1].fitness; /* since fitness is cumulative */
			else
				f2 = child -> parent2 -> fitness;
			child -> fitness = f2;
		}
		/* case where parent2 carries the beneficial mutation unlike parent1 ~ swap to help us in rewind*/
		if (f2 > f1){
			child -> parent1 = &current -> people[p2];
			child -> parent2 = &current -> people[p1];
		}
	}
	else{
		if (p1 != 0)
			child -> fitness = child -> parent1 -> fitness - current -> people[p1-1].fitness; /* since fitness is cumulative */
		else
			child -> fitness = child -> parent1 -> fitness;
	}

	/* case where a single fit child is supposed to be initialized */
	if (event_number == ben_gen && single_bene == '1'){
		child -> fitness = fitness;
		single_bene = 0;
		ben_gen = steps + 1;
	}
	/* case where a new beneficial mutation appears */
	else if ( event_number >= ben_gen){
		double mutation = ((float)rand()/(float)(RAND_MAX));
		if (mutation <= ben_chance)
			child -> fitness = fitness;
	}

	if (child -> fitness > 1)
		fit_everywhere++;

	assert(child); /* just a safety clause, should never be triggered */
	return child;
}

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
		/* now we need to choose the destination of the newborn child */
		migrate = rand() / (float)RAND_MAX;
		if ( ( migrate < current -> migrate ) && ( nxtstep -> total_fric > 0 ) ){
			migration(i, j, &nxtstep -> people[pos]);
			++mig_pop;
		}
		else{
			/* cumulative fitness allows for easier parental selection in the next generation */
			if ( nxtstep -> people[pos].fitness > 1 )
				nxtstep -> fit_people++;
			if (pos != 0)
				nxtstep -> people[pos].fitness += nxtstep -> people[pos - 1].fitness;
			++pos;
		}
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
	fit_everywhere = 0;
	unsigned i, j;
	total_people = 0; /* each generation obviously starts with 0 people */
	unsigned areas_on_capacity = 0;
	for (i = 0; i < rows; ++i){
		for (j = 0; j < columns; ++j){
			if (current -> incoming > 0)	 /* if there are immigrants coming to this area */
				intergrate_migration(i,j);
			if (current -> population == current -> capacity)
				++areas_on_capacity;
			if (current -> population > 0 && nxtstep -> capacity > 0) /* inhabitated areas only */
				nxtstep -> population = reproduce(i,j);
				// nxtstep -> population = mass_migration(i,j);
		}
	}
	// print_population();

	FILE * f1 = fopen("qifsha.txt", "a");
	float logos = (float)fit_everywhere/ total_people;
	fprintf(f1, "%lf\n", logos );
	fclose(f1);

	if (ben_gen == -1 && areas_on_capacity == (rows * columns)) /* if every area on the map has reached it's capacity */
		ben_gen == steps + 1;

	if (single_bene && ben_gen > steps && fit_everywhere == 0){ /* mutation lost and in this scenario the simulation no longer has a purpose */
		fprintf(stderr, "No fit people remaining in generation %u \n", event_number);
		assert(0);
	}
	else if (logos == 1.0)
		fixation = 1;
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
	for (i = 0; i < rows; ++i){
		for (j = 0; j < columns; ++j)
			fprintf(f1, "%u ", current -> population);
		fprintf(f1, "\n");
	}
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

	int gen_of_fix = -1;
	unsigned i;
	for (i = 0; i < steps - 1; ++i){
		if (People[event_number] == 0){
			fprintf(stderr, " \nThere are no people left so there is no point in continuing this simulation\n");
			break;
		}
		if ( gen_after_fix != -1 && fixation == 1 && gen_of_fix == -1){
				gen_of_fix = event_number;
				fixation = 2; /* just to avoid re-entering this block of code */
		}
		if ( fixation != 0 && (gen_of_fix + gen_after_fix) == event_number ){ /* when this happens we end the simulation */
			FILE * ff = fopen("gen_of_fix.txt", "w");
			fprintf(ff, "%u\n", event_number);
			fclose(ff);
			break;
		}

		event();
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
