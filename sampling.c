#include "sampling.h"

#define current	A[event_number][i][j]

extern unsigned samples;

extern unsigned event_number;
extern unsigned rows;
extern unsigned columns;
extern spot **** A;
extern unsigned * People;

extern boxed * boxhd;

person * samp;								/* besides using the sampled people to find common ancestors, we need to keep them in a list for future use */

extern char smpl;								/* checks if we are still in sampling or not. Default value 1(true) since we begin this step by picking samples */
extern speciment * head;
extern speciment * tail;

extern gsl_rng * r;

void print_boxes(){
	FILE * fb = fopen("boxes.txt", "w");
	boxed * tmp = boxhd;
	while (tmp != NULL){
		fprintf(fb, "samples: %d, x[%d %d], y[%d %d] gen %d\n", tmp -> samples, tmp -> srow, tmp -> erow, tmp -> scol, tmp -> ecol, tmp -> gen);
		tmp = tmp -> next;
	}
	fclose(fb);
}

void area_sampling(unsigned sam, unsigned srow, unsigned erow, unsigned scol, unsigned ecol, unsigned gen){
	gen = event_number - gen;
	if (sam > People[gen]){ /* if we do not have enough people to sample */
		printf("\n Not enough samples in generation: %d\n", gen);
		assert(0);
	}

	unsigned random, counter, x, i, j, k;
	unsigned  sampflag = 0;
	smpl = 1;
	speciment * s;
	person * tmp;

	/* we need to calculate the number of people in the box */
	long unsigned totalbox = 0;
	for (i = srow; i <= erow; ++i){
		for (j = scol; j <= ecol; ++j)
			totalbox += A[gen][i][j] -> population;
	}
	if (sam > totalbox){ /* in case we don't have enough sam in the box */
	 fprintf(stderr, "\n Not enough samples in the selected region [%u %u] [%u %u]\n", srow, erow, scol, ecol);
	 assert(0);
	}

	int *samplesID = malloc(sam * sizeof(int));
	for(x = 0; x < sam; ++x){
		sampflag = 0;
		random = rand() % totalbox;

		for(j = 0; j < x; ++j){
			if (samplesID[j] == random){
				sampflag =1;
				break;
			}
		}
		if (sampflag){
			x--;
			sampflag = 0;
			continue;
		}
		samplesID[x] = random;
	}

  gsl_permutation * perm = gsl_permutation_alloc((size_t)totalbox);
  gsl_permutation_init(perm);
  gsl_ran_shuffle(r, perm -> data, (size_t)totalbox, sizeof(size_t) );

  if (samp == NULL)
    samp = malloc(sizeof(person) * sam);
  else
    samp = realloc(samp, sizeof(person) * (samples + sam));


	for (x = 0; x < sam; ++x){
		random = perm -> data[x];	/* each time we choose a random person from the total population until we reach the desired amount */
		counter = 0;
		for ( i = srow; i <= erow; ++i){
			for ( j = scol; j <= ecol; ++j){
				if (A[gen][i][j] -> population > 0 ){
					if (random - counter <= A[gen][i][j] -> population){
						s = malloc(sizeof(speciment));
						s -> prs = malloc(sizeof(person));
						memcpy(s -> prs, &current -> people[random - counter], sizeof(person));
						memcpy(&samp[samples + x], s -> prs, sizeof(person));

						s -> present_children = NULL;
						s -> next = NULL;
						s -> prev = NULL;
						/* we add it to the queue */
						insert_queue(s);
						j = columns;
						i = rows;
					}
					else
						counter += A[gen][i][j] -> population;
				}
			}
		}
	}
	samples += sam; /* adding these samples to the total, for creating the table of polymorphisms later on */
	free(samplesID);
	smpl = 0;	/* done with sampling */
	assert(head != NULL);
}

void sampling(unsigned samples){ /* self-explanatory */
	if (samples > People[event_number]){ /* if we do not have enough people to sample */
		printf("Number of samples exceeds population. Sampling is not possible.\n");
		assert(0);
	}
	unsigned random, counter, x, i, j, k;
	unsigned  sampflag = 0;
	speciment * s;
	//person * tmp;

  gsl_permutation * perm = gsl_permutation_alloc((size_t)People[event_number]);
  gsl_permutation_init(perm);
  gsl_ran_shuffle(r, perm -> data, (size_t)People[event_number], sizeof(size_t) );

  samp = malloc(sizeof(person) * samples);

  for (x = 0; x < samples; ++x){
		random = perm -> data[x];	/* find the person to sample */
		counter = 0;
		for ( i = 0; i < rows; ++i){
			for ( j = 0; j < columns; ++j){
				if (current -> population > 0 ){
					if (random - counter < current -> population){

						s = malloc(sizeof(speciment));
						s -> prs = malloc(sizeof(person));
						memcpy(s -> prs, &current -> people[random - counter], sizeof(person));
		        memcpy(&samp[x], s -> prs, sizeof(person));

						s -> present_children = NULL;
						s -> next = NULL;
						s -> prev = NULL;
						/* we add it to the queue */
						insert_queue(s);
						j = columns;
						i = rows;
					}
					else
						counter += current -> population;
				}
			}
		}
	}
  gsl_permutation_free(perm);
	smpl = 0;	/* done with sampling */
}
