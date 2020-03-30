#include "mutation.h"

time_t t;

extern void recomb_free();
extern void free_seg();
extern void free_present();
extern void tree_destruction(speciment * s);
extern void print_sampleList();
extern void print_mutated(person * p);
extern void print_present_children(speciment * s);

extern speciment * head; 			/* pointer to the common ancestors list we discovered during the previous step */
extern person * samp;				/* pointer to the people we chose as a sample for this process */
extern unsigned total;				/* total number of people we encountered during our journey back in time */
extern unsigned samples;			/* number of samples we took in the previous step */
//extern unifree * uhead;
extern struct ancestry *anc;
extern unsigned nk;

extern unsigned max_segment;		/* num of bases our gene has */
extern int ben_pos;

unsigned mut = 20;					/* number of mutations we plan on applying ~ Default twenty*/

unsigned parsed;
unsigned invalid = 0; /* mutations that did not affect anyone */

char * poly;
short ** muTable;					/* a table where we store the mutations */
unsigned * ntons;
unsigned * mut_pos;					/* position each mutation landed */

extern gsl_rng * r;

void print_table_sample(){ /* rows are samples, columns are mutations */
	//printf("PRINT MUTATIONS\n");

	unsigned * mut_success = calloc( mut, sizeof(unsigned));

	unsigned i,j, counter = 0;
	for (i = 0; i < mut; ++i){
		if (poly[i] == 0)
			++invalid;
	}

	FILE  * f1;
	f1 = fopen("mutation_table.txt", "w");
	fprintf(f1, "//\nsegsites: %d\npositions: ", (mut-invalid));

	for (i = 0; i < mut; ++i){ /* print the positions where the mutation occcured */
		if (poly[i])
			fprintf(f1, "%lf ", ( mut_pos[i] / (float)(max_segment+1) ));
	}
	fprintf(f1, "\n");
	for (i = 0; i < samples; ++i){
		counter = 0;
		for (j = 0; j < mut; ++j){
			if (poly[j])
				fprintf (f1,"%u", muTable[i][j]);
			if (muTable[i][j] == 1)
			  ++mut_success[j];
		}
   	//++ntons[counter];
		fprintf(f1,"\n");
	}
	fprintf(f1,"\n");
	fclose(f1);

	for (i = 0; i < mut; ++i)
	  ++ntons[mut_success[i]];

	FILE  * f2;
	f2 = fopen("ntons.txt", "w");
	for (j = 0; j <= samples; ++j)
		fprintf (f2,"%d ", ntons[j]);
	fprintf(f2, "\n");
	fclose(f2);
	free(mut_success);
}

/* ---------------------- allocation/deallocation start ---------------------- */

void allocate_mutation_table(unsigned rows, unsigned columns){
	unsigned r, c;
	muTable = (short **)malloc(rows * sizeof(short *));
    for (r = 0; r < rows; ++r){
        muTable[r] = (short *)malloc( columns * sizeof(short *));
		for (c = 0; c < columns; ++c)
			muTable[r][c] = 0;
	}
}

void free_ancestry(){
  affected * aff = NULL;
  segment * seg = NULL;
  unsigned i;
  unsigned j = 0;
  for (i = 0; i < nk; ++i){
    for (j = 0; j < anc[i].presentChildren; ++j){
      if (i < samples)
				free_seg(anc[i].affChildren[j].segments);
    }
    free(anc[i].affChildren);
  }
  free(anc);
}

void freedom(unsigned rows){ /* we free all that's left allocated */
	free(ntons);
	free(mut_pos);
	free(poly);
	unsigned r;
	for (r = 0; r < rows; ++r)
		free(muTable[r]);
	free(muTable);
	free(samp);
	free_ancestry();
	tree_destruction(head);
	recomb_free();
}

/* ---------------------- allocation/deallocation end ---------------------- */

short isSubsegment(segment * seg, unsigned mutation){ /* checks if a person contains the specif genome in their segment list */
	assert(seg != NULL);
	segment * tmp = seg;
	while (tmp != NULL){
		if ( !(tmp -> start > mutation) && !(tmp -> end < mutation) )
			return 1; /* found it */
		tmp = tmp -> next;
	}
	return 0;
}

void print_spatial_origin(){
	FILE * f1 = fopen("spatial_origin.txt", "w");
	unsigned k;
//	for (k = 0; k <anc[rp].presentChildren; ++k){

//	}
	fclose(f1);
}

/* short of a main for this step of the code */
void mutate(unsigned nmut){
	if (nmut != -1) /* user defined mutation rate */
	  mut = nmut;
	if (ben_pos != -1) /* if there is indeed a beneficial mutation */
		mut = mut + 1;
	unsigned i = 0, j=0;

	/* allocate everything required */
	ntons = calloc((1 + samples), sizeof(unsigned));
	poly = calloc(mut, sizeof(char));
	allocate_mutation_table(samples,  mut);

	mut_pos = calloc(mut, sizeof(unsigned));
	unsigned rp = 0;
	unsigned place = 0;
	unsigned misses = 0; /* how many mutations did not affect the present */
	int id = 0;

	/* choose the positions for the mutations */
	unsigned max_possible = (max_segment + 1);
	gsl_permutation * perm = gsl_permutation_alloc((size_t)max_possible);
	gsl_permutation_init(perm);
	gsl_ran_shuffle(r, perm -> data, (size_t)max_possible, sizeof(size_t) );


	for (i = 0; i < mut; ++i)
		mut_pos[i] = perm -> data[i];
	if (ben_pos != -1) /* not yet placed so in final position*/
		mut_pos[mut - 1] = ben_pos;

	/* sort the mutations to comply with the Hudson's ms format */
	unsigned swp = 0;
	for (i = 0; i < mut; ++i){
		for (j = i + 1; j< mut; ++j){
			if (mut_pos[j] < mut_pos[i]){
				swp = mut_pos[i];
				mut_pos[i] = mut_pos[j];
				mut_pos[j] = swp;
			}
			else if (mut_pos[j] < mut_pos[i]){ /* same mutation twice which we do not want */
				swp = mut_pos[i];
				mut_pos[i] = mut_pos[--mut];
				mut_pos[mut] = swp;
			}
		}
	}

	for (i = 0; i < mut; ++i){
		rp = rand() % total;
		misses = 0;
		for (j = 0; j < anc[rp].presentChildren; ++j){
		  id = anc[rp].affChildren[j].num + samples;
			if (isSubsegment(anc[rp].affChildren[j].segments, mut_pos[i]))
				muTable[id][i] = 1;
			else
				++misses;
		}
		if (misses != j)
			poly[i] = 1;
		++ntons[anc[rp].presentChildren];
	}
	print_table_sample();
	freedom(samples);
}
