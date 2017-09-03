#include "mutation.h"

time_t t;

extern void free_present();
extern void tree_destruction(speciment * s);
extern void print_sampleList();
extern void print_mutated(person * p);
extern void print_present_children(speciment * s);

extern speciment * head; 			/* pointer to the common ancestors list we discovered during the previous step */
extern person * samp;				/* pointer to the people we chose as a sample for this process */
extern unsigned total;				/* total number of people we encountered during our journey back in time */
extern unsigned samples;			/* number of samples we took in the previous step */
extern unifree * uhead;
//extern struct ancestry *anc;


unsigned mut = 20;					/* number of mutations we plan on applying ~ Default twenty*/
double theta = 0.01;	 				/* input for the Poisson distribution */
short p_flag = 1;						/* determines how the table will be printed */

unsigned parsed;

short ** muTable;						/* a table where we store the mutations */
short ** muTable2;
unsigned * ntons;
unsigned * ntons2;

unsigned * mutPlaces;

extern gsl_rng * r;

speciment * queue;					/* pointer to the head of the queue */
speciment * tail;						/* pointer to the last speciment in the queue */

deq * dq;
deq * dt;

/* ------------------- queue management start ------------------- */

speciment * pop_spec(){
  if (tail == NULL) /* for safety purposes should never happen */
    assert(0);
  speciment * s = tail;
  tail = tail -> prev;
  if (tail == NULL)
    queue = NULL;
  else
    tail -> next = NULL;
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
	if (!queue){/* can't empty something already empty */
		tail = NULL;
		return;
	}
	speciment * tmp;
	while (tail){
		tmp = pop_spec();
		free(tmp);
	}
	tail = NULL;
}

/* -------------------- queue management end -------------------- */

//speciment * occurance(){	/* a simple BFS algorithm which allows us to find the place of the mutation */
//	speciment * s = head; /* in case rand returns 0, so we don't enter the loop */
//	push_spec(s);
//	speciment * tmp;
//	unsigned random = rand() % total;
//	unsigned counter;
//	tmp = s -> children;
//	for (counter = 0; counter < random; counter++){
//		s = pop_spec();
//		if (s -> children){ /* so in every generation but the present one */
//			tmp = s -> children;
//			while (tmp != NULL){
//				push_spec(tmp);
//				tmp = tmp -> next;
//			}
//		}
//		if (counter != random - 1) /* if it ain't the one we are looking for */
//			free(s);
//	}
//	return s; /* the speciment of the person where the mutation is gonna occur */
//} 


speciment * occurance2(speciment * s, unsigned counter, unsigned random){
	if (counter == random)
		return s;
	if (s -> children == NULL) 
		return NULL;
	speciment * tmp = s -> children;
	speciment * result = NULL;
	while (tmp != NULL && result == NULL){
		result = occurance2(tmp, ++parsed, random);
		tmp = tmp -> next;
	}
	return result;
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

void create_table_column(speciment * s, unsigned column){
	person * sampled = samp;
	speciment * mutated = s -> present_children;
	mutated = s -> present_children;
	unsigned counter = 0;
	if (mutated == NULL){ /* meaning we are in a leaf node */
		while (sampled -> pid != s -> prs -> pid){
			counter++;
			sampled = sampled -> next;
		}
		muTable2[counter][column] = 1;
	}
	unsigned mutation = rand() % 1000;
	if (isSubsegment(s -> segments, mutation) ){
		while (mutated != NULL){
			if ( sampled -> pid == mutated -> prs -> pid /*&& isSubsegment(mutated -> segments, mutation) */ ){
				muTable2[counter][column] = 1;
				mutated = mutated -> next;
			}
			sampled = sampled -> next;
			counter++;
		}
	}
}

void create_table_row(speciment * s, unsigned row){
	person * sampled = samp;
	speciment * mutated = s -> present_children;
	unsigned counter = 0;
	unsigned mutation = rand() % 1000;	/* choose the place for the mutation to occur */
	if ( isSubsegment(s -> segments, mutation) ){	/* if the mutation does land on a genome that does not affect the present generation we ignore it */
		while (mutated != NULL && sampled != NULL){
			if ( (sampled -> pid == mutated -> prs -> pid)/* && isSubsegment(mutated -> segments, mutation) */ ){
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
		for (j = 0; j < samples; j++){
			fprintf (f1,"%d ", muTable[i][j]);
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
  fprintf(stdout, "PRINT MUTATIONS");
	FILE  * f1;
	f1 = fopen("mutation_table2.txt", "w");
	int i, j, counter;
	for (i = 0; i < samples; i++){
		counter = 0;
		for (j = 0; j < mut; j++){
			fprintf (f1,"%d", muTable2[i][j]);
			if (muTable2[i][j])
				counter ++;
		}
		ntons2[counter]++;
		fprintf(f1,"\n");
	}
	FILE  * f2;
	f2 = fopen("ntons2.txt", "w");
	for (j = 0; j <= samples; j++)
		fprintf (f2,"%d\t", ntons2[j]);
	fprintf(f2,"\n");
	fclose(f1);
	fclose(f2);
}

/* ------------------- ancestree print start ------------------- */

deq * pop_depth(){
	if (!dt)
		assert(0);
	unsigned d = dt -> num;
	dt = dt -> prev;
	if (!dt)
		dq = NULL;
	return d;
}

void push_depth(unsigned d){
	deq * tmp = malloc(sizeof(deq));
	tmp -> num = d;
	tmp -> next = dq;
	tmp -> prev = NULL;
	if (dq != NULL)
		dq -> prev = tmp;
	else
		dt = tmp;
	dq = tmp;
}


void print_mutation_tree(){
	dq = NULL;
	dt = dq;
	speciment * s = head;
	push_spec(s);
	push_depth(0);
	speciment * tmp;
	tmp = s -> children;
	unsigned curr = 0;
	unsigned prev = -1;
	unsigned depth = 0;
	FILE * fml = fopen("ancestree.txt", "w");
	while (1){
		s = pop_spec();
		if (s == NULL)
			break;
		curr = pop_depth();
		fprintf(fml, "%d ", s -> prs -> pid);
		if (curr != prev){ /* we have a depth change */
			fprintf(fml, "\n------\n");
			prev = curr;
			depth++;
		}
		if (s -> children){
			tmp = s -> children;
			while (tmp != NULL){
				push_spec(tmp);
				push_depth(depth);
				tmp = tmp -> next;
			}

		}
	}
	fclose(fml);
}

/* -------------------- ancestree print end -------------------- */

void allocate_mutation_table2(unsigned rows, unsigned columns){
	unsigned r, c;
	muTable2 = (short **)malloc(rows * sizeof(short *));
    for (r = 0; r < rows; r++){
        muTable2[r] = (short *)malloc( columns * sizeof(short *));
		for (c = 0; c < columns; c++)
			muTable2[r][c] = 0;
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

void mutate2(unsigned nmut){
  mut = nmut;
  unsigned i = 0, j=0;
  ntons2 = calloc((1+nmut), sizeof(unsigned));
  unsigned *currentSamplesIDs = calloc(samples, sizeof(unsigned));
  allocate_mutation_table(mut, samples);
  allocate_mutation_table2(samples, mut);
  printf("Samples: %d\n", samples);
  unsigned rp = 0;
  int id = 0;
  for (i = 0; i < nmut; ++i){
    rp = rand() % total;
    //fprintf(stderr, "rp: %d\n", rp);
    //fprintf(stderr, "pc: %d\n", anc[rp].presentChildren);
    for (j = 0; j < anc[rp].presentChildren; ++j){
		id = anc[rp].affChildren[j] + samples;
		//fprintf(stderr, "chID: %d\n", anc[rp].affChildren[j]);
		muTable2[id][i] = 1;
    }
	ntons2[anc[rp].presentChildren]++;
  }
  print_table_sample();
  free(ntons2);
}
  



/* sort of a main for this step */
void mutate(){
	mut = 30;//gsl_ran_poisson(r, (theta * total));
	allocate_mutation_table(mut, samples);
	allocate_mutation_table2(samples, mut);
	printf("Samples: %d\n", samples);
	speciment * s;
	ntons = malloc(sizeof(unsigned) * ( 1 + mut));
	ntons2 = malloc(sizeof(unsigned) * ( 1 + mut));
	mutPlaces = malloc(sizeof(unsigned) * mut);
	unsigned j;
	for ( j = 0; j <= mut; j++){
		ntons[j] = 0;
		ntons2[j] = 0;
	}
	unsigned counter;
	empty_specs();
	unsigned rnd;
	for (counter = 0; counter < mut; counter++){
		//s = occurance();
		parsed = 0;
		rnd = rand() % total;
		s = occurance2(head, parsed, rnd);
		mutPlaces[counter] = s -> prs -> pid;
		print_present_children(s);
		//create_table_row(s, counter);
		create_table_column(s, counter);
		//empty_specs();	/* we make sure the queue is empty, since we might repeat this step a few times */
		///free(s);
	}
	if (p_flag)
		print_table_sample();
	else
		print_table_mutation();
	FILE * fm = fopen("mutPlaces.txt", "w");
	for (counter = 0; counter < mut; counter++)
		fprintf(fm, "%d\n", mutPlaces[counter]);
	fclose(fm);
	free(mutPlaces);
	freedom(mut);
	printf ("\n MUTATION COMPLETE \n");
}


/*
Διαλέγω x random αριθμούς. Parse το δέντρο μόλις πέσω σ έναν απ τους αριθμούς, mutate σε τυχαίο σημείο(segment) τα present children.
Υποστηρίζω overlapping mutations.
*/
