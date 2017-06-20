#include "mutation.h"

time_t t;

extern void free_present();
extern void tree_destruction(speciment * s);
extern void print_sampleList();
extern void print_mutated(person * p);

extern speciment * head; 		/* pointer to the common ancestors list we discovered during the previous step */
extern person * samp;			/* pointer to the people we chose as a sample for this process */
extern unsigned total;			/* total number of people we encountered during our journey back in time */
extern unsigned samples;		/* number of samples we took in the previous step */
extern unifree * uhead;

unsigned mut = 200;					/* number of mutations we plan on applying ~ Default ten*/
double theta = 0.1;	 				/* input for the Poisson distribution */
short p_flag = 0;					/* determines how the table will be printed */

short ** muTable;					/* a table where we store the mutations */
unsigned * ntons;

extern gsl_rng * r;

speciment * queue;				/* pointer to the head of the queue */
speciment * tail;					/* pointer to the last speciment in the queue */

/* ------------------- queue management start ------------------- */

speciment * pop_spec(){
	if (!tail) /* for safety purposes should never happen */
		assert(0);
	speciment * s = tail;
	tail = tail -> prev;
	if (!tail)
		queue = NULL;
	return s;
}

void push_spec(speciment * s){
	speciment * tmp = malloc(sizeof(speciment));
	tmp -> prs = s -> prs;
	tmp -> children = s -> children;
	tmp -> present_children = s -> present_children;
	tmp -> segments = s -> segments;
	tmp -> next = queue;
	tmp -> prev = NULL;
	if (queue)
		queue -> prev = tmp;
	else
		tail = tmp;
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
	//printf("occurance\n");
	speciment * s = head; /* in case rand returns 0, so we don't enter the loop */
	push_spec(s);
	speciment * tmp;
	unsigned random = rand() % total;
	unsigned counter;
	tmp = s -> children;
	for (counter = 0; counter < random; counter++){
		s = pop_spec();
		if (s -> children){ /* so in every generation but the present one */
			tmp = s -> children;
			while (tmp != NULL){
				push_spec(tmp);
				tmp = tmp -> next;
			}
		}
		if (counter != random - 1) /* if it ain't the one we are looking for */
			free(s);
	}
	s -> next = NULL;
	return s; /* the speciment of the person where the mutation is gonna occur */
}

short isSubsegment(segment * seg, unsigned mutation){ /* checks if a person contains the specif genome in their segment list */
	segment * tmp = seg;
	while (tmp != NULL){
		if ( !(tmp -> start > mutation) && !(tmp -> end < mutation) )
			return 1;
		tmp = tmp -> next;
	}
	return 0;
}

void create_table_row(speciment * s, unsigned row){
	person * sampled = samp;
	speciment * mutated = s -> present_children;
	unsigned counter = 0;
	unsigned mutation = rand() % 1000;	/* choose the place for the mutation to occur */
	if ( isSubsegment(s -> segments, mutation) ){	/* if the mutation does land on a genome that does not affect the present generation we ignore it */
		//printf("valid mutation\n");
		while (mutated != NULL && sampled != NULL){
			if ( (sampled -> pid == mutated -> prs -> pid) && isSubsegment(mutated -> segments, mutation) ){
				muTable[row][counter] = 1;
				mutated = mutated -> next;
			}
			sampled = sampled -> next;
			counter++;
		}
	}
}

void print_table_mutation(){ 	/* rows are mutations, columns are samples */
	FILE  * f1;
	f1 = fopen("mutation_table.txt", "w");
	int i, j, counter;
	for (i = 0; i < mut; i++){
		counter = 0;
		fprintf(f1,"site:%d\t", i);
		for (j = 0; j < samples; j++){
			fprintf (f1,"%d", muTable[i][j]);
			if (muTable[i][j])
				counter ++;
		}
		ntons[counter]++;
		fprintf(f1,"\n");
	}
	FILE  * f2;
	f2 = fopen("ntons.txt", "a");
	for (j = 0; j <= samples; j++)
		fprintf (f2,"%d\t", ntons[j]);
	fprintf(f2,"\n");
	fclose(f1);
	fclose(f2);
}

void print_table_sample(){ 	/* rows are samples, columns are mutations */
	int i, j;
	for (i = 0; i < samples; i++){
		for (j = 0; j < mut; j++)
			printf ("%d | ", muTable[i][j]);
		printf("\n");
	}
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


void freedom(unsigned rows){  /* we free all that's left allocated */
	unsigned r;
	for (r = 0; r < rows; r++)
		free(muTable[r]);
	printf("tree_destruction\n");
	//tree_destruction(head);
	free_present(uhead);
}


/* sort of a main for this step */
void mutate(){
	mut = gsl_ran_poisson(r, (theta * total));
	allocate_mutation_table(mut, samples);
	unsigned counter;
	printf("Samples: %d\n", samples);
	speciment * s;
	ntons = malloc(sizeof(unsigned) * (1+samples));
	unsigned j;
	for ( j = 0; j <= samples; j++)
		ntons[j] = 0;
	for (counter = 0; counter < mut; counter++){
		s = occurance(head, total);
		create_table_row(s, counter);
		//free(s);
		empty_specs();	/* we make sure the queue is empty, since we might repeat this step a few times */
	}	
	if (p_flag)
		print_table_sample();
	else
		print_table_mutation();
	freedom(mut);
	printf ("\n MUTATION COMPLETE \n");
}


/*
Διαλέγω x random αριθμούς. Parse το δέντρο μόλις πέσω σ έναν απ τους αριθμούς, mutate σε τυχαίο σημείο(segment) τα present children.
Υποστηρίζω overlapping mutations.
*/
