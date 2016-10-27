#include "rewind_time.h"

#define current	A[event_number][i][j]
#define MAX_SEGMENT 1000

time_t t;

extern unsigned event_number;
extern unsigned rows;
extern unsigned columns;
extern spot **** A;
extern unsigned * People;

unsigned samples = 10;
int event_backwards;
unsigned int negative_generations = 0;	/* in case we need to go further back in time to discover the common ancestor, we keep track of our steps */
unsigned curr_gen = 0;
unsigned prev_gen = 0;
unsigned keepsake = 0;						/* for tracing further back in time purposes */
char smpl = 1;										/* checks if we are still in sampling or not. Default value 1(true) since we begin this step by picking samples */

unsigned total = 0; 								/* total number of people we ve encountered in our journey back in time */

person * samp;									/* besides using the sampled people to find common ancestors, we need to keep them in a list for future use */
speciment * head;								
speciment * tail;

FILE * f1;

unsigned min(unsigned x, unsigned y){
	if ( x < y )
		return x;
	return y;
}

unsigned max(unsigned x, unsigned y){
	if ( x > y )
		return x;
	return y;
}

segment * merge_segments(segment * node, segment * s){ /* supporting function to merge ~ merges the segments and fixes overlapping cases */
	segment * t;
	segment * overlap;
	segment * leftover1;
	segment * leftover2;
	segment * seg1 = node;
	segment * seg2 = s;			
	while (seg1 && seg2){
		if (seg1 -> start == seg2 -> start && seg1 -> end == seg2 -> end) /* exactly the same segments */
			return node;
		else if ( seg1 -> end < seg2 -> start){			/* seg1 ends before seg2 starts */
			if (!t)
				t = seg1;
			else{
				t -> next = seg1;
				t = t -> next; 
			}
			seg1 = seg1 -> next;
		}
		else if (seg2 -> end < seg1 -> start ){			/* seg2 ends before seg1 starts */
			if (!t)
				t = seg2;
			else{
				t -> next = seg2;
				t = t -> next;
			}
			seg2 = seg2 -> next;
		}
		else {														/* overlapping ~ we break the segs in 3 pieces, the common seg, the one before, and the one after that */
			overlap = malloc(sizeof(segment));
			overlap -> start = max(seg1 -> start, seg2 -> start);
			overlap -> end = min(seg1 -> end, seg2 -> end);
			if (overlap -> start > overlap -> end){
				unsigned x = overlap -> end;
				overlap -> end = overlap -> start;
				overlap -> start = x;
			}
			overlap -> next = NULL;
			if (seg1 -> start != seg2 -> start) {  /* if the 2 segments don't have the exact same start */
				leftover1 = malloc(sizeof(segment));
				leftover1 -> start = min(seg1 -> start, seg2 -> start);
				if(overlap -> start > 0)
					leftover1 -> end = overlap -> start - 1;
				else	/* if we cut exaclty at zero */
					leftover1 -> end = overlap -> start;
				if (!t)
					t = leftover1;
				else{
					t -> next = leftover1;
					t = t -> next;
				}
			}
			if (!t)
				t = overlap;
			else{
				t -> next = overlap;
				t = t -> next;
			}
			if (seg1 -> end != seg2 -> end){
				leftover2 = malloc(sizeof(segment));
				if (overlap -> end < MAX_SEGMENT)
					leftover2 -> start = overlap -> end + 1;
				else /* if we cut exaclty at max */
					leftover2 -> start = overlap -> end;
				leftover2 -> end = max(seg1 -> end, seg2 -> end);
				leftover2 -> next = NULL;
				if (seg1 -> next && ( !(leftover2 -> end < seg1 -> next -> start) ) ){	/* meaning we have an overlap with the next segment of seg1*/
					t -> next = merge_segments(seg1 -> next , leftover2);
					return t; 
				}
				else if (seg2 -> next && (!(leftover2 -> end < seg2 -> next -> start) ) ){ /* meaning we have an overlap with the next segment of seg2*/
					t -> next = merge_segments(seg2 -> next, leftover2);
					return t;
				}
				else {
					t -> next = leftover2;
					t = t -> next;
				}
			}
			seg1 = seg1 -> next;
			seg2 = seg2 -> next;
		}
	}
	if (seg1) /* which means seg2 has ended */
		t -> next = seg1;
	else /* which means that either both have ended or just seg1 */
		t -> next = seg2;
	return t;
}

speciment * merge(speciment * s1, speciment * s2){
	s1 -> segments = merge_segments(s1 -> segments, s2 -> segments);
	person * dummy = malloc(sizeof(person)); /* we use a dummy list in order to store the present children in ascending order */
	person * prescld = dummy;
	person * p1 = s1 -> present_children;
	person * p2 = s2 -> present_children;
	p1 = s1 -> present_children;
	p2 = s2 -> present_children;
	while ( p1 && p2 ){
		if ( p1 -> pid == p2 -> pid ) /* duplicate prevention */
			p1 = p1 -> next;
		else if (p1 -> pid < p2 -> pid){
			if (dummy != NULL){
				dummy -> next = p1;
				dummy = dummy -> next;
			}
			else
				dummy = p1;
			p1 = p1 -> next;
		}
		else {
			if (dummy != NULL){
				dummy -> next = p2;
				dummy = dummy -> next;
			}
			else
				dummy = p2;
			p2 = p2 -> next;
		}
	}
	if (p1){ /* meaning that p2 reached it's end */
		if(!dummy){
			dummy = p1;
		}
		else
			dummy -> next = p1;
	}
	else{
		if(!dummy)
			dummy = p2;
		else
			dummy -> next = p2;
	}
 	while (prescld -> next && prescld -> pid == 0)
		prescld = prescld -> next;
	s1 -> present_children = prescld;
	speciment * cld = s1 -> children;
	while(cld -> next)
		cld = cld -> next;
	cld -> next = s2 -> children;
	return s1;
}

/* --------------------------------------- insert - delete queue start ---------------------------------------- */

void insert_queue(speciment * s){
	s -> prev = NULL;
	if(!head){
		head = s;
		tail = head;
		prev_gen++;
		return;
	}
	unsigned pg = 0; /* previous generation counter ~ so that we know how far we search for duplicates */
	speciment * tmp = head; 
	if (!smpl) {
		while (tmp && pg < prev_gen){
			if ( tmp -> prs -> pid == s -> prs -> pid){ /* if we alredy got this person from another child */
				tmp = merge(tmp, s);
				return;
			}
			pg++;
			tmp = tmp -> next;
		}
	}
	head -> prev = s;
	s -> next = head;
	head = s;
	prev_gen++;
}

void sample_storage(person * p){
	if (!samp){								/* for the first sample */
		samp = p;
		return;
	}
	p -> next = samp;
	samp = p;
}

/* ---------------------------------------- insert - delete queue end ----------------------------------------- */

void sampling(unsigned samples){ /* self-explanatory */ 
	unsigned random, counter, x, i, j, k;
	segment * seg = malloc(sizeof(segment));
	seg -> start = 0;
	seg -> end = MAX_SEGMENT;
	seg -> next = NULL;
	speciment * s;
	person * tmp;
	for (x = 0; x < samples; x++){
		random = ( rand() % (People[event_number] -2) ) + 1; /* each time we choose a random person from the total population until we reach the desired amount */
		if (random < 1)
			random == 1;
		counter = 0;
		for ( i = 0; i < rows; i++){
			for ( j = 0; j < columns; j++){
				if (current -> population > 0 ){
					if (random - counter <= current -> population){
						tmp = current -> people;
						k = 1;
						for ( k = counter; k < random; k++ )
							k++;
						tmp -> next = NULL;
						sample_storage(tmp);	/* keeping a trace of the samples */
						s = malloc(sizeof(speciment));
						s -> prs = tmp;
						s -> segments = malloc(sizeof(segment));
						s -> segments = seg;
						s -> children = malloc(sizeof(speciment));
						s -> children = NULL;
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
	smpl = 0;	/* done with sampling */
}

/* ------------------------------------------- back in time start ------------------------------------------- */

void recombine(speciment * s){
	/* initialing the speciments describing the 2 parents */
	speciment * s1 = malloc(sizeof(speciment));
	speciment * s2 = malloc(sizeof(speciment));
	if (rand() % 99 < 50){
		s1 -> prs = s -> prs -> parent1;
		s2 -> prs = s -> prs -> parent2;
	}
	else {
		s1 -> prs = s -> prs -> parent2;
		s2 -> prs = s -> prs -> parent1;
	}
	s1 -> prs -> next = NULL;
	s2 -> prs -> next = NULL;
	
	s1 -> children = malloc(sizeof(speciment));
	s1 -> children = s;
	s1 -> children -> next = NULL;
	s2 -> children = malloc(sizeof(speciment));
	s2 -> children = s;
	s2 -> children -> next = NULL;
	if (event_number == event_backwards){ /* on our first step back */
		s1 -> present_children = s -> prs;
		s1 -> present_children -> next = NULL;
		s2 -> present_children = s -> prs;
		s2 -> present_children -> next = NULL;
	}
	else{
		s1 -> present_children = s -> present_children;
		s2 -> present_children = s -> present_children;
	}
	segment * seg1 = malloc(sizeof (segment)); 
	segment * seg2 = malloc(sizeof (segment));
	
	unsigned cutting_point = rand() % (MAX_SEGMENT -2) + 1;	/* we choose a cutting point from 1 to max segment - 1 */
	if (s -> segments -> next){ /* if there are more than one segments */
		segment * tmp = s -> segments;
		segment * tmp1 = malloc(sizeof(segment));
		seg1 = tmp1;
		while (tmp &&  cutting_point > tmp -> start){	/* till we reach the cutting point  */
			if (!tmp1){ /* for the first time we enter the loop */
				tmp1 = tmp;
			}
			else {
				tmp1 -> next = tmp;
				tmp1 = tmp1 -> next;
			}
			tmp = tmp -> next;
		}
		if (tmp && !tmp1){ /* if the cutting point was in the first segment */
			segment * seg1 = malloc(sizeof (segment)); 
			seg1 -> start = tmp -> start;
			seg1 -> end = cutting_point;
			seg2 -> start = cutting_point +1;
			seg2 -> end = tmp -> end;
			seg2 -> next = tmp -> next;
		}
		else if (tmp && cutting_point < tmp -> end ){
			tmp1 -> next = malloc(sizeof(segment));
			tmp1 -> next -> start = tmp -> start;
			tmp1-> next -> end = cutting_point;
			tmp1 -> next -> next = NULL;
			seg2 -> start = cutting_point + 1;																																
			seg2 -> end = s -> segments -> end;
			seg2 -> next = tmp -> next;
		}
		else{ /* meaning that the cutting point was before the firsrt or after the last segment */
			free(seg2);
			s1 -> segments = s -> segments;
			insert_queue(s1);
			return;
		}
	}
	else if ( cutting_point < s -> segments -> start && cutting_point > s -> segments -> end) { /* if the random cutting point doesn't match the person's segment we simply do not need to recombine, we keep whichever parent we chose as s1 */
		free(s2);	/* no need for this anymore */
		s1 -> segments = s -> segments;
		insert_queue(s1);
		return;
	}
	else{
		seg1 -> start = s -> segments -> start;
		seg1 -> end = cutting_point;
		seg2 -> start = cutting_point + 1;
		seg2 -> end = s -> segments -> end;
	}
	s1 -> segments = seg1;
	s2 -> segments = seg2;
	insert_queue(s1);
	insert_queue(s2);
}


void choose_parent(speciment* s){
	if (s -> prs -> parent1 -> pid != s -> prs -> parent2 -> pid){	/* for single-parented children recombination is obviously useless */
		unsigned recombination = rand() % 99;
		if ( recombination < 1 ) {
			recombine(s);
			return;
		}
	}
	speciment * tmp = malloc(sizeof(speciment));
	if ( (s -> prs -> parent1 -> pid == s -> prs -> parent2 -> pid) || (rand() % 99 < 50) ) 	/* single parented so we don't need to choose or we randomly select the first */
		tmp -> prs = s -> prs -> parent1;
	else
		tmp -> prs = s -> prs -> parent2;
	tmp -> prs -> next = NULL;
	tmp -> segments = s -> segments;
	if (event_number == event_backwards){ /* on our first step back */
		tmp -> present_children = s -> prs;
		tmp -> present_children -> next = NULL;
	}
	else
		tmp -> present_children = s -> present_children;
	 /* we are creating a new top - down tree this time so we can go forward afterwards */
	tmp -> children = s;
	assert(tmp -> children);
	insert_queue(tmp);
}

void create_parent(speciment * s){ /* we create a new parent to go further back in time. In case of recombination we create 2 of them */
	person * parent1 = malloc(sizeof(person));																			
	parent1 -> pid = rand() % (curr_gen + prev_gen);	/* the range of the possible parents equals the things we have in queue thus far */
	parent1 -> xaxis = 0;
	parent1 -> yaxis = 0;
	parent1 -> weight = 0;
	parent1 -> next = NULL;
	parent1 -> parent1 = NULL;
	parent1 -> parent2 = NULL;
	
	speciment * tmp = malloc (sizeof(speciment));
	tmp -> prs = parent1;
	tmp -> prs -> next = NULL;
	unsigned recombination = rand() % 99;	
	segment * seg = s -> segments;
	if ( recombination < 1 ){
		person * parent2 = malloc(sizeof(person));
		parent2 -> pid = rand() % rand() % (curr_gen + prev_gen);
		while (parent2 -> pid == parent1 -> pid) /* making sure we don't end up with a single parent */
			parent2 -> pid = rand() % rand() % (curr_gen + prev_gen);
		parent2 -> xaxis = 0;
		parent2 -> yaxis = 0;
		parent2 -> weight = 0; 
		parent2 -> next = NULL;
		parent2 -> parent1 = NULL;
		parent2 -> parent2 = NULL;
		s -> prs -> parent1 = parent1;
		s -> prs -> parent2 = parent2;
		recombine(s);
		return;
	}
	
	tmp -> segments = seg;
	tmp -> present_children = s -> present_children;
	tmp -> children = s; /* we are creating a new top - down tree this time so we can go forward afterwards */
	insert_queue(tmp);
}

void go_back(){
	printf("go back\n");
	speciment * tmp = malloc(sizeof(speciment));
	while ( event_backwards > 0 && prev_gen != 1){  /* we stop going back either when we find a common ancestor for each segment or when we reach step 0 */ 
		if ( prev_gen != 0 ) /* so in every step but the first */
			curr_gen = prev_gen; /* we move a generation back in time */
		prev_gen = 0;
		printf ("Event %d has %d people \n", event_backwards, curr_gen);
		while (curr_gen > 0){
			tmp = tail;
			choose_parent(tmp);
			tail = tail -> prev;
			tail -> next = NULL;
			curr_gen--;
		}
		total +=prev_gen;
		event_backwards--;
	}
}

void further_back(){	/* we might need to go to generations before zero to trace a common ancestor */
	printf("go further back\n");
	speciment * tmp = malloc(sizeof(speciment));
	while ( prev_gen != 1){  /* we stop going back only when we discover a common ancestor for each segment */ 
		curr_gen = prev_gen; /* we move a generation back in time */
		prev_gen = 0;
		printf ("Negative Event %d has %d people \n", negative_generations, curr_gen);
		while (curr_gen > 0){
			tmp = tail;
			create_parent(tmp);
			tail = tail -> prev;
			tail -> next = NULL;
			curr_gen--;
		}
		total +=prev_gen;
		negative_generations++;
	}
}

/* ---------------------------------- back in time end  ----------------------------------*/

void print_children(speciment * s){
	f1 = fopen("f1.txt","a");
	fprintf(f1, "----------- start of children  %d-----------\n", s -> prs -> pid);
	speciment * tmp = s -> children;
	unsigned counter = 0;
	while (tmp){
		fprintf(f1, "child: %d \n", tmp -> prs -> pid);
		tmp = tmp -> next;
		counter++;
	}
	fprintf (f1,"children: %d\n", counter);
	fprintf(f1, "----------- end of children -----------\n");
	fclose(f1);
}


/* sort of a main for this step */
void rewind_time(){
	event_backwards = event_number;
	sampling(samples);
	go_back();
	keepsake = prev_gen; 	/* keep track of the people we have in generation zero */
	if ( prev_gen !=1 )			/* meaning we partially "failed" during the initial traceback */
		further_back();
	printf("Total people encountered: %d\n", total);
	printf("Speciment: %d is the common ancestor", head -> prs -> pid);
	free(A); 						/* not useful anymore so no need to keep all this memory allocated */
	printf (" \n REWIND COMPLETE \n");
}

/* ------------------------------------------------------------------------------------------------- BRIEF SUMMARY OF THE CODE ------------------------------------------------------------------------------------------------- */

/*
1) We sample N people from the last instance of the map as created in the forward step

2) We add these children in a double linked list(implementation of a FIFO queue) that besides these people contains a list of all the segments we are examining and a boolean of whether we are 
done with the current segment. Done with

a segment simply means that all the samples have found a common ancestor from whom they inhereted the current segment. A segment is the fragment of a chain consisting of chromosomes , 
dna or anything else we are examining.

3) We now go back in time, each time examining the previous generation in the search for one or more common ancestors. There are three condition that we check and if either one is complete 
we terminate the loop.The first is finding a single common ancestor. The second(which sort of includes the first but we have to check both), is reaching a point in time were for each segment we 
have found again a common ancestor. The third equals the failure of the first two since
when we reach step zero, that is the starting point of the experiment we have no other choice but to stop. While none of the above stands correct, we take these steps:
	i) we pop the last node of the queue
	ιι) we deduce the path back for that node and follow it. We add the parent of that person to the queue. We keep an integer that show us how many people we have in the previous generation,
	which we increase it's time we find a parent.
	ιιι) if two people have the same parent, we merge the segments we took from each, thus keeping a single node for each unique parent.
	iv) we repeat till all the current generations parents have been deduced.
	v) we check for done and not - done segments. As long as we have segments that exist in multiple people we are not done.
*/