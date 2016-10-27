#include "mutation.h"

time_t t;

extern void print_children(speciment * s);

extern speciment * head; 		/* pointer to the common ancestors list we discovered during the previous step */
extern person * samp;			/* pointer to the people we chose as a sample for this process */
extern unsigned total;			/* total number of people we encountered during our journey back in time */
extern unsigned samples;		/* number of samples we took in the previous step */


unsigned mut = 3;					/* number of mutations we plan on applying ~ Default three */
short p_flag = 0;					/* determines how the table will be printed */

short ** muTable;				/* a table where we store the mutations */

speciment * queue;				/* pointer to the head of the queue */
speciment * tail;					/* pointer to the last speciment in the queue */

double power(double x, unsigned y){ /* x ^ y */
    if ( y == 0)
       return 1;
	unsigned counter;
	for (counter = 0; counter < y; counter++)
		x = x * x;
	return x;
}

unsigned factorial(unsigned n){ /* n! ~ giving very high values could result in an overflow of the unsigned value */
	unsigned x = 1;
	if ( n > 1)
		x = n * factorial(n -1);
	return x;
}

void poisson_mutation(unsigned k, double mu){ /* uses a Poisson equation in order to speculate the number of mutations */
	double poisson;
	double x = power(mu, k);
	poisson =  exp(-mu) * (x / factorial(k));
	mut = 3;
}

/* ------------------- queue management start ------------------- */

speciment * pop_spec(){
	if (!tail)
		return;
	speciment * s = tail;
	tail = tail -> prev;
	return s;
}

void push_spec(speciment * s){
	speciment * tmp = malloc(sizeof(speciment));
	tmp = s;
	if (queue) /* if we already have something inside the queue */
		tmp -> next = queue;
	else
		queue = malloc(sizeof(speciment));
	queue = tmp;
}

void empty_specs(){
	if (!queue)/* can't empty something already empty */
		return;
	speciment * tmp;
	while (tail){
		tmp = pop_spec();
		free(tmp);
	}
}

/* -------------------- queue management end -------------------- */

speciment * occurance(){	/* a simple BFS algorithm which allows us to find the place of the mutation */
	printf("occurance\n");
	empty_specs();	/* we make sure the queue is empty, since we might repeat this step a few times */
	speciment * s = head; /* in case rand returns 0, so we don't enter the loop */
	push_spec(s);
	tail = s;
	speciment * tmp = malloc(sizeof(speciment));
	unsigned random = rand() % total;
	unsigned counter;
	tmp = s -> children;
	assert(s -> children -> next);
	for (counter = 0; counter < random; counter++){
		s = pop_spec();
		if ( s -> children ){ /* so in every generation but the present one */
			tmp = s -> children;
			while ( tmp != NULL){
				print_children(tmp);
				/* push_spec(tmp); */
				tmp = tmp -> next;
			}
			break;
		}
	}
	
	return s; /* the speciment of the person where the mutation is gonna occur */
}

short isSubsegment(segment * seg, unsigned mutation){ /* checks if a person contains the specif genome? in their segment list */
	segment * tmp = seg;
	assert(tmp);
	while ( tmp && !(tmp -> start > mutation ) ){
		if ( !( tmp -> end < mutation ) )
			return 1;
		tmp = tmp -> next;
	}
	return 0;
}

void create_table_row(speciment * s, unsigned row){
	person * smp = samp;
	person * prs = s -> present_children;
	assert(s -> present_children -> pid);
	unsigned counter = 0;
	unsigned mutation = rand() % 1000;	/* choose the place for the mutation to occur */
	if ( isSubsegment(s -> segments, mutation) ){	/* if the mutation does land on a genome that does not affect the present generation we simply ignore it */
		printf("valid mutation\n");
		while (smp){
			if(smp -> pid && (rand() % 2) && s -> segments -> start < mutation && s -> segments -> end > mutation)
				muTable[row][counter] = 1;
			smp = smp -> next;
			counter++;
		}
	}
}

print_temp_queue(){
	speciment * tmp = queue;
	while (tmp){
		printf("%d\n",  tmp -> prs -> pid);
		tmp = tmp -> next;
	}
}

void print_table_mutation(){ 	/* rows are mutations, columns are samples */
	FILE  * f1;
	f1 = fopen("results.txt", "w"); /* "w" can be changes to "a" in case you want to stack results into the same file. "w" erases the old content of the file. */ 
	int i, j;
	unsigned x;
	for (i = 0; i < mut; i++){
		for (x = 0; x < 108; x++)
			fprintf(f1, "-");
		fprintf(f1,"\n site: %d\t", i);
		for (j = 0; j < samples; j++)
			fprintf (f1,"%d | ", muTable[i][j]);
		fprintf(f1,"\n");
	}
}

void print_table_sample(){ 	/* rows are samples, columns are mutations */
	int i, j;
	for (i = 0; i < samples; i++){
		for (j = 0; j < mut; j++)
			printf ("%d | ", muTable[i][j]);
		printf("\n");
	}
	printf("---------------------------------------------------------------------------------- \n");
}

void allocate_mutation_table(unsigned rows, unsigned columns){
	unsigned r, c;
	muTable = (short **)malloc(rows * sizeof(short *));
    for (r=0; r < rows; r++){
        muTable[r] = (short *)malloc( columns * sizeof(short *));
		for (c = 0; c < columns; c++)
			muTable[r][c] = 0;
	}
}

/* sort of a main for this step */
void mutate(){
	double theta = 0.61;
	poisson_mutation(samples, theta);
	allocate_mutation_table(mut, samples);
	unsigned counter;
	printf("Samples: %d\n", samples);
	speciment * s  = malloc(sizeof(speciment));
	for (counter = 0; counter < mut; counter++){
		s = occurance(head, total);
		assert(s -> children);
		create_table_row(s, counter);
	}	
	if (p_flag)
		print_table_sample();
	else
		print_table_mutation();
	empty_specs(); /* we no longer need the queue */
	printf ("\n MUTATION COMPLETE \n");
}