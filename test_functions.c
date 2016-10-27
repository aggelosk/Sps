/* In this file, I stored every function used for testing purposes during the creation / debug of the code. Most of them are really simple, and easy to reproduce, but since
I already did the work I kept them here for anyone willing to dig a little deeper into the code. I have organized them in order according to the file they belong. */

/* ------------------------------------------- Create Map  -------------------------------------------*/

void see_the_sea(){		/* finds all the areas whose type is "sea" */
	unsigned x;
	for (x = 0; x < 108; x++)
		printf("-");
	printf("\n");
	unsigned r;
	unsigned c;
	for (r = 0; r < rows; r++){
		for (c =0; c< columns; c++){
			if (A[0][r][c] -> type == sea){
				printf (" |%d : %d| ", r, c);
			}
		}
	}
	for (x = 0; x < 108; x++)
		printf("-");
	printf("\n");
}


/* ----------------------------------------- Forward Time  ------------------------------------------ */

void print_incoming_people(unsigned ev){ /* prints the immigrants joining each area in the given event */
	assert(ev < event_number + 1);
	unsigned i, j, x;
	for (x = 0; x < 108; x++)
		printf("-");
	printf("\n");
	unsigned pop_test = 0;
	for (i = 0; i < rows; i++){
		printf("%d:", i);
		for (j = 0; j < columns; j++){
			if (A[ev][i][j] -> incoming > 0){
				pop_test += A[ev][i][j] -> incoming;
				printf(" | %d -> %d | ", j, A[ev][i][j] -> incoming);
			}
		}
		printf("\n");
	}
	printf("incoming: %d\n",pop_test);
	for (x = 0; x < 108; x++)
		printf("-");
	printf("\n");
}

void print_area_people(unsigned ev){ /* prints the population of each area in the given event */
	assert(ev < event_number + 1);
	unsigned i, j, x;
	for (x = 0; x < 108; x++)
		printf("-");
	printf("\n");
	unsigned pop_test = 0;
	for (i = 0; i < rows; i++){
		printf("%d:", i);
		for (j = 0; j < columns; j++){
			if (A[ev][i][j] -> population > 0){
				pop_test += A[ev][i][j] -> population;
				printf(" | %d -> %d | ", j, A[ev][i][j] -> population);
			}
		}
		printf("\n");
	}
	printf("still: %d\n", pop_test);
	for (x = 0; x < 108; x++)
		printf("-");
	printf("\n");
}

/* -------------------------------------- Rewind Time -------------------------------------- */

void print_queue(){	/* self-explanatory - used for testing pursposes */
	printf("---------------- queue start ----------------\n");
	speciment * tmp = head;
	unsigned counter = 0;
	while (tmp){
		printf("speciment: %d \n", tmp -> prs -> pid);
		tmp = tmp -> next;
		counter++;
	}
	printf ("in queue: %d\n", counter);
	printf("---------------- queue end ----------------\n");
}

void print_sampleList(){	/* self-explanatory - used for testing pursposes */
	printf("---------------- start of samples ----------------\n");
	person * tmp = samp;
	unsigned counter = 0;
	while (tmp){
		printf("sample: %d \n", tmp  -> pid);
		tmp = tmp -> next;
		counter++;
	}
	printf ("sampled: %d\n", counter);
	printf("---------------- end of samples ----------------\n");
}

void print_segments(){
	speciment * tmp = head;
	segment * seg;
	while (tmp){
		printf("speciment: %d is the common ancestor with segments:", tmp -> prs -> pid);
		seg = tmp -> segments;
		while (seg){
			printf(" [%d %d] ", seg -> start, seg -> end);
			seg= seg -> next;
		}
		printf("\n");
		tmp = tmp -> next;
	}
}

void print_children(speciment * s){
	printf("----------- start of children  -----------\n");
	speciment * tmp = s -> children;
	unsigned counter = 0;
	while (tmp){
		printf("child: %d \n", tmp -> prs -> pid);
		tmp = tmp -> next;
		counter++;
	}
	printf ("children: %d\n", counter);
	printf("----------- end of children -----------\n");
}

void print_present_children(speciment * s){
	printf("----------- start of present children  -----------\n");
	person * tmp = s -> present_children;
	unsigned counter = 0;
	while (tmp){
		printf("child: %d \n", tmp  -> pid);
		tmp = tmp -> next;
		counter++;
	}
	printf ("children: %d\n", counter);
	printf("----------- end of present children -----------\n");
}