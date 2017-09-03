#include "forward_time.h"

#define current	A[event_number][i][j] 
#define nxtstep	A[event_number +1][i][j]

time_t t;

extern spot **** A;

unsigned event_number = 0; /* number of current event - shows the number of entries in the event table */
unsigned long int total_people = 0;	/* total number of people encountered in the forward in time step */
unsigned long int unique_id = 0;

unsigned steps = 101;
unsigned rows = 50; 
unsigned columns = 50;
/* default values */

double ben_chance = 0.001;
unsigned init_ben = 10;
float fitness = 1.2;

unsigned long int no_children = 0;
unsigned pop_counter = 0;

unsigned * People; 	/* holds the population for each generation */
leader * Leaders; 		/* for the mass - migration model. Shows which population tends to follow which */

/* -------------------------------------------------------------------------------------------- */


/* -------------------------- Different scenario activators ------------------------------------- */
char single_parent = 1; 		/* default is to allows single parented children */
char beneficial = 1;				/* default is to allow such a thing */
char flock = 0;						/* default is to allow such a thing */
/* ------------------------------------------------------------------------------------------------------ */

unsigned logistic_function(unsigned i, unsigned j){
	/* determines the reproduction of the population in the area */
	double x = 1.0 - ((double)current -> population / (double)current -> capacity);
	double population = (current -> growth_rate * (double)current -> population  * x) + (double)current -> population  + 0.5;
	return (unsigned)population; /* cast into unsigned as a round - up measure */
}


void leadershift(unsigned oldleader, unsigned newleader, unsigned xaxis, unsigned yaxis){
	unsigned i = 0;
	Leaders[oldleader].lead = newleader;
	Leaders[oldleader].xaxis = xaxis;
	Leaders[oldleader].yaxis = yaxis;
	while (i < rows * columns && Leaders[i].lead != -1 ) { /* while there are still valid leaders to check */
		if (Leaders[i].lead == oldleader ){
			Leaders[i].lead = newleader;
			Leaders[i].xaxis = xaxis;
			Leaders[i].yaxis = yaxis;
		}
		i++;
	}
}

void migration(unsigned i, unsigned j, person * child){	/* will move the entity to the new area */
	double total_fric = 0;
	int x, y;
	for (x = -1; x< 2; x++){
		if (  i + x >= 0 && i + x < rows ){ /* caution check so that we don't wander outside the array borders */
			for (y = -1; y < 2; y++){
				if (  j + y >= 0 && j + y < columns && (x != 0 || y !=0 ) )
					total_fric = total_fric + A[event_number][i + x][j + y] -> friction;
			}
		}
	}
	double migrate = ((float)rand()/(float)(RAND_MAX)) * total_fric;
	double counter = 0.0;
	for (x = -1; x< 2; x++){
		if ( i + x >= 0 && i + x < rows){ 
			for (y = -1; y < 2; y++){
				if ( j + y >= 0 && j + y < columns && (x != 0 || y !=0 ) ){
					if ( migrate - counter <= A[event_number][i + x][j + y] -> friction ){
						if (flock){
							if  (A[event_number][i + x][j + y] -> pop_num != -1 ) /* previous population inhabiting the area */ 
								A[event_number + 1][i + x][j + y] -> pop_num = A[event_number][i + x][j + y] -> pop_num;
							else if ( A[event_number + 1][i + x][j + y] -> pop_num == -1){ 				/* first in the area */
								A[event_number + 1][i + x][j + y] -> pop_num = ++pop_counter;			/* since no previous population has been in the area, we define a new one */
								leadershift(current -> pop_num, pop_counter, x, y);								/* we set the new leader for the rest of the populations following */
							}
							assert(pop_counter < (rows * columns));									/* just a safely clause should never be triggered */
						}
						child -> next = A[(event_number + 1)][i + x][j + y] -> immigrants;	/* the person migrates to the new area */
						A[event_number + 1][i + x][j + y] -> immigrants = child;	
						A[event_number + 1][i + x][j + y] -> incoming++;
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
	person * child = malloc(sizeof (person));
	child -> pid = ++unique_id;//total_people; /* first person of each generation takes the id 0 etc */
	child -> flag = 0;
	child -> fitness = 1;
	child -> parent1 = NULL;
	child -> parent2 = NULL;
	child -> next = NULL;
	double p1 = (rand() / (double)RAND_MAX)  * (current -> population + (double)current -> fit_people * fitness);
	double p2 = (rand() / (double)RAND_MAX)  * (current -> population + (double)current -> fit_people * fitness);
	if (!single_parent){ /* in this case we can't allow single parents */
		while (p1 == p2)
			p2 = ( rand() / (double)RAND_MAX ) * (current -> population + (double)current -> fit_people * fitness);
	}
	double ps = 0;
	person * tmp = current -> people;
	if ( p1 < p2){
		while (ps < p1 -1 && tmp -> next){
			tmp = tmp -> next;
			ps += tmp -> fitness;
		}
		child -> parent1 = tmp;
		tmp -> flag = 1;
		while (ps < p2 - 1&& tmp -> next){
			tmp = tmp -> next;
			ps += tmp -> fitness;
		}
		child -> parent2 = tmp;
		tmp -> flag = 1;
	}
	else if (p2 < p1){
		while (ps < p2 && tmp ->next){
			tmp = tmp -> next;
			ps += tmp -> fitness;
		}
		child -> parent2 = tmp;
		tmp -> flag = 1;
		while (ps < p1 && tmp ->next){
			tmp = tmp -> next;
			ps += tmp -> fitness;
		}
		child -> parent1 = tmp;
		tmp -> flag = 1;
	}
	else if (single_parent == 1){ /* only if we accept single-parented childer */
		while (ps < p2 && tmp -> next){
			tmp = tmp -> next;
			ps+= tmp -> fitness;
		}
		child -> parent1 = tmp;
		child -> parent2 = tmp;
		tmp -> flag = 1;
	}
	else
		assert(0); /* we should never arrive in this case at any point */
	/* we need to fixate the extra weight - beneficiality - of the newborn child If we allow such a thing */
	if (child -> parent1 -> fitness > 1)
		child -> fitness = child -> parent1 -> fitness;
	else if  (child -> parent2 -> fitness > 1 )
		child -> fitness = child -> parent1 -> fitness;
	else if ( (rand() / (double)RAND_MAX) < ben_chance){
		child -> fitness += fitness;
	}
	assert(child); /* for safely purposes,should never be triggered */
	return child;
}

/* --------------------- mass migration start  ------------------- */

void follow_the_leader(unsigned i, unsigned j, leader  l, person * child){
	int x = i;
	int y = j;
	assert (l.xaxis == -1 || l.xaxis == 0 || l.xaxis == 1);
	assert (l.yaxis == -1 || l.yaxis == 0 || l.yaxis == 1);
	if ( i + l.xaxis >=0 && i + l.xaxis < rows)
		x += l.xaxis;
	if (j + l.yaxis >= 0 && j + l.yaxis < columns)
		y += l.yaxis;
	if (A[event_number][x][y] -> friction == 0){ /* the area is not accesible thus the person remains where he is */
		x = i;	// will require a more realistic solution 
		y = j;
	}
	if (A[event_number][x][y] -> pop_num != -1)
		A[event_number +1][x][y] -> pop_num = A[event_number][x][y] -> pop_num;
	else if (A[event_number +1][x][y] -> pop_num == -1) /* first in the area */
		A[event_number +1][x][y] -> pop_num = l.lead;	/* the population is set to be the same as the Leaders */
	assert (A[event_number +1][x][y] -> pop_num != -1);	/* just a safely clause should never be triggered */
	child -> next = A[event_number +1][x][y] -> immigrants;
	A[event_number +1][x][y] -> immigrants = child;
	A[event_number +1][x][y] -> incoming++;
}

unsigned long int mass_migration(unsigned i, unsigned j){
	nxtstep -> pop_num = current -> pop_num;
	assert(nxtstep -> pop_num != -1);
	unsigned long int new_pop = 0;
	new_pop = logistic_function(i,j);
	unsigned remaing = 0;
	person * child = NULL;
	unsigned counter;
	double random;
	for (counter = 0; counter < new_pop; counter++){
		child = birth(i,j);		
		/* now we need to decide if the person is gonna stay or migrate*/
		random = rand() / (float)RAND_MAX;
		if (random < 0.990){
			if ( Leaders[current -> pop_num].lead != -1 && random < 0.9)
				follow_the_leader(i, j, Leaders[current -> pop_num], child);
			else
				migration(i, j, child);
		}
		else{ /* stays back */
			child -> next = nxtstep -> people;
			nxtstep -> people = child;
			nxtstep -> fit_people += child -> fitness;
			remaing++;
		}
		total_people++;
	}
	return remaing;
}

/* --------------------- mass migration end ------------------- */

unsigned long int reproduce(unsigned i, unsigned j){
	unsigned long int new_pop = 0;
	new_pop = logistic_function(i,j);
	unsigned mig_pop = 0;
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

void darwinism(unsigned i, unsigned j){ /* we kill every person that has no children in order to save memory */
	person * dmp = NULL;
	if (current -> people -> flag == 0){
		dmp = current -> people;
		current -> people = current -> people -> next;
		free(dmp);
		return;
	}
	person * tmp = current -> people;
	person * prev = tmp;
	while (tmp != NULL){
		if ( tmp -> flag == 0){
			prev -> next = tmp -> next;
			dmp = tmp;
			tmp = tmp -> next;
			free(dmp);
			no_children++;
		}
		else{
			prev = tmp;
			tmp = tmp -> next;
		}
	}
}

void event(){
	//if (event_number % 500 == 0) /* just to track the process of the simulation */
		printf("Event %d\n", event_number);
	unsigned i, j;
	total_people = 0; /* each generation obviously starts with 0 people */
	for (i = 0; i < rows; i++){
		for (j = 0; j < columns; j++){
			if (current -> incoming > 0)	 /* if there are immigrants coming to this area */
				intergrate_migration(i,j);
			if (current -> population > 0){ /* inhabitated areas only */
				if (flock)
					nxtstep -> population = mass_migration(i,j);
				else
					nxtstep -> population = reproduce(i,j);
				darwinism(i,j);
			}
			else if (flock && current -> pop_num != -1)
				nxtstep -> pop_num = current -> pop_num;
		}
	}
	People[++event_number] = total_people;
}

void print_people(){
	unsigned i;
	unsigned total = 0;
	for (i = 0; i <= event_number; i++){
		total += People[i];
		printf("event %d has %d people \n", i, People[i]);
	}
	printf ("Total People: %d\n", total);
}

void print_massmig(char * filename, unsigned ev){
	FILE * f1;
	f1 = fopen(filename, "w");
	unsigned i,j;
	for (i = 0; i < rows; i++){
		for (j = 0; j < columns; j++)
			fprintf(f1,"%d ", A[ev][i][j]-> population);
		fprintf(f1,"\n");
	}
	fclose(f1);
}

void destruction(){ /* frees all the allocated memory */
	printf("Seek and Destroy\n");
	unsigned e, i, j;
	person * tmp = NULL;
	for ( e = 0; e < event_number; e++){
		for (i = 0; i < rows; i++){
			for (j = 0; j < columns; j++){
				if (A[e][i][j] -> population){
					while (A[e][i][j] -> people != NULL){
						tmp = A[e][i][j] -> people;
						A[e][i][j] -> people = A[e][i][j] -> people -> next;
						free(tmp);
					}
				}
				free(A[e][i][j]);
			}
		}
	}
	free(People);
}

/* sort of a main for this section */
void forward_time(){
	People = malloc(sizeof(unsigned) * steps);
	People[0] = total_people;
	printf("%d\n", People[0]);
	
	Leaders = malloc(  (rows * columns) * sizeof(leader) ); 
	unsigned i; 
	for (i = 0; i < rows * columns; i++){
		Leaders[i].lead = -1;
		Leaders[i].xaxis = 0;
		Leaders[i].yaxis = 0;
	}
	
	for (i = 0; i < steps - 1; i++){
		if (People[event_number] == 0){
			printf(" \nThere are no people left so there is no point in continuing this simulation\n");
			break;
		}
		event();
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
	//print_people();
		unsigned ev;
		char  filename[] = "f000/.txt";        /* if you wish for more files just add more digits(chars) and use a similar method */
		for (ev = 0; ev <= event_number; ev+= 10){
			filename[4]++;
			if (filename[4] > '9'){
				filename[4] = '0';
				filename[3]++;
			}
			if (filename[3] > '9'){
				filename[3] = '0';
				filename[2]++;
			}
			if (filename[2] > '9'){
				filename[2] = '0';
				filename[1]++;
			}
			print_massmig(filename, ev);
		}
	free(Leaders);
	printf("Number of people that did not reproduce: %d\n", no_children);
	printf("\n FORWARD COMPLETE \n");
}

/* time complexity is O(steps * rows * columns) */

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
	In this section of the project, the second one called, we receive an initialized instance of a map, along with the populations inhabiting some of its areas,
	and we begin going a number of steps forward in time, where each step equals to a new generation. For each of these steps, the population of each area reproduces
	and each of the children either inhabitates the same area as the parent, or one of the neighbouring areas.
*/