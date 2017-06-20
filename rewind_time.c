#include "rewind_time.h"

#define current	A[event_number][i][j]
#define MAX_SEGMENT 1000

time_t t;

extern unsigned event_number;
extern unsigned rows;
extern unsigned columns;
extern spot **** A;
extern unsigned * People;
extern void destruction();

unsigned samples = 100;
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
unifree * uhead;

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

segment * merge_segments(segment * first, segment * second){
	segment * t = NULL;
	segment * seg1 = first;
	segment * seg2 = second;
	segment * freeseg = NULL;
	while(seg1 != NULL && seg2 != NULL){
		if (seg1 -> start == seg2 -> start ){
			if (t == NULL)
				t = seg1;
			else{
				t -> next = seg1;
				t = t -> next;
			}
			t -> end = max(seg1 -> end, seg2 -> end);
			seg1 = seg1 -> next;
			seg2 = seg2 -> next;
		}
		else if (seg1 -> end == seg2 -> end){
			if (t == NULL)
				t = seg1;
			else{
				t -> next = seg1;
				t = t -> next;
			}
			t -> start = min(seg1 -> start, seg2 -> start);
			seg1 = seg1 -> next;
			seg2 = seg2 -> next;
		}
		else if(seg1 -> start < seg2 -> start){
			if (t == NULL)
				t = seg1;
			else{
				t -> next = seg1;
				t = t -> next;
			}
			if (!(t -> end < seg2 -> start - 1)){
				t -> end = max(t -> end, seg2 -> end);
				seg2 = seg2 -> next;
			}
			seg1 = seg1 -> next;
		}
		else{
			if (t == NULL)
				t = seg2;
			else{
				t -> next = seg2;
				t = t -> next;
			}
			if (!(t -> end < seg1 -> start - 1)){
				t -> end = max(t -> end, seg1 -> end);
				seg1 = seg1 -> next;
			}
			seg2 = seg2 -> next;
		}
	}
	if (seg1) /* which means seg2 has ended */
		t -> next = seg1;
	else /* which means that either both have ended or just seg1 */
		t -> next = seg2;
	print_segments(t);
	return t;
}

speciment * merge(speciment * s1, speciment * s2){
	s1 -> segments = merge_segments(s1 -> segments, s2 -> segments);
	speciment * p1 = s1 -> present_children;
	speciment * p2 = s2 -> present_children;
	print_present_children(s1);
	print_present_children(s2);
	speciment * tmp = NULL;;
	while ( p1 && p2){
		if (p1 -> prs -> pid == p2 -> prs -> pid){ /* duplicate prevention */
			p1 -> segments = merge_segments(p1 -> segments, p2 -> segments); 
			p1 = p1 -> next;
			p2 = p2 -> next;
		}
		else if (p1 -> prs -> pid > p2 -> prs -> pid){ /* need to merge */
			tmp = p2 -> next;
			if ( !p1 -> prev ){ /* p2 < every p1 */
				p2 -> prev = NULL;
				p2 -> next = p1;
				p1 -> prev = p2;
				s1 -> present_children = p2;
			}
			else {
				p2 -> prev = p1 -> prev;
				p1 -> prev -> next = p2;
				p1 -> prev = p2;
				p2 -> next = p1;
			}
			p2 = tmp;
		}
		else{
			if (!p1 -> next){
				p1 -> next = p2;
				p1 -> next -> prev = p1;
				break;
			}
			p1 = p1 -> next;
		}
	}
	print_present_children(s1);
	speciment * cld = s1 -> children;
	while(cld -> next)
		cld = cld -> next;
	cld -> next = s2 -> children;
	free(s2 -> prs);
	free(s2);
	return s1;
}

/* --------------------------------------- insert - delete queue start ---------------------------------------- */

void insert_queue(speciment * s){
	s -> prev = NULL;
	if (!head){
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

void unique_storage(speciment * s){
	unifree * u = malloc(sizeof(unifree));
	u -> spec = s;
	u -> next = uhead;
	uhead = u;
}

void sample_storage(person * p){ /* stores the samples into a list in ascending order */
	if (!samp){ /* first entry */
		samp = p;
		samp -> next = NULL;
		return;
	}
	if (!( samp -> pid < p -> pid)) { /* add to head of the list */
		if (samp -> pid == p -> pid )
			samples--;
		else{
			p -> next = samp;
			samp = p;
		}
		return;
	}
	person * tmp = samp;
	person * prv = samp;
	while (tmp != NULL && tmp -> pid < p -> pid ){
		prv = tmp;
		tmp = tmp -> next;
	}
	if ( tmp != NULL &&  tmp -> pid == p -> pid ){ /* duplicate prevention */
		printf("duplicate %d\n", p -> pid);
		assert(0);
		samples--;
		return;
	}
	prv -> next = p;
	p -> next = tmp;
}

/* ---------------------------------------- insert - delete queue end ----------------------------------------- */

void sampling(unsigned samples){ /* self-explanatory */ 
	unsigned random, counter, x, i, j, k;
	unsigned  sampflag = 0;
	speciment * s;
	person * tmp;
	
	int *samplesID = malloc(samples * sizeof(int));
	
	for(x = 0; x < samples; ++x){
		sampflag = 0;
		random = rand() % People[event_number];
		
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
	
	for (x = 0; x < samples; x++){
		random = samplesID[x];	/* each time we choose a random person from the total population until we reach the desired amount */
		counter = 0;
		for ( i = 0; i < rows; i++){
			for ( j = 0; j < columns; j++){
				if (current -> population > 0 ){
					if (random - counter < current -> population){
						tmp = current -> people;
						for ( k = counter; k < random; k++ )
							tmp = tmp -> next;
						assert(tmp);
						s = malloc(sizeof(speciment));
						s -> prs = malloc(sizeof(person));
						s -> prs -> pid = tmp -> pid;
						s -> prs -> flag = 'u';
						s -> prs -> fitness = tmp -> fitness;
						s -> prs -> parent1 = tmp -> parent1;
						s -> prs -> parent2 = tmp -> parent2;
						s -> prs -> next = NULL;
						sample_storage(s -> prs);	/* keeping a trace of the samples */
						s -> segments = malloc(sizeof(segment));
						s -> segments -> start = 0;
						s -> segments -> end = MAX_SEGMENT;
						s -> segments -> next = NULL;
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
	free(samplesID);
	smpl = 0;	/* done with sampling */
}

/* ------------------------------------------- back in time start ------------------------------------------- */

void recombine(speciment * s){
	unsigned cutting_point = rand() % MAX_SEGMENT;	/* we choose a cutting point */
	/* initialing the speciments describing the 2 parents */
	speciment * s1 = malloc(sizeof(speciment));
	speciment * s2 = malloc(sizeof(speciment));
	s1 -> prs = malloc(sizeof(person));
	s2 -> prs = malloc(sizeof(person));
	if (rand() % 99 < 50){
		s1 -> prs -> pid = s -> prs -> parent1 -> pid;
		s1 -> prs -> flag = 'u';
		s1-> prs -> parent1 = s -> prs -> parent1-> parent1;
		s1 -> prs -> parent2 = s -> prs -> parent1 -> parent2;
		s2 -> prs -> pid = s -> prs -> parent2 -> pid;
		s2 -> prs -> parent1 = s -> prs -> parent2-> parent1;
		s2 -> prs -> parent2 = s -> prs -> parent2 -> parent2;
	}
	else {
		s1 -> prs -> pid = s -> prs -> parent2 -> pid;
		s1 -> prs -> flag = 'u';
		s1-> prs -> parent1 = s -> prs -> parent2-> parent1;
		s1 -> prs -> parent2 = s -> prs -> parent2 -> parent2;
		s2 -> prs -> pid = s -> prs -> parent1 -> pid;
		s2-> prs -> parent1 = s -> prs -> parent1-> parent1;
		s2 -> prs -> parent2 = s -> prs -> parent1 -> parent2;
	}
	s2 -> prs -> flag = 'r'; /* flagging it for use in free */
	s1 -> prs -> next = NULL;
	s2 -> prs -> next = NULL;
	s1 -> children = s;
	s2 -> children = s;
	
	s1 -> present_children = NULL;
	s2 -> present_children = NULL;
	char multiflag = 0;
	if (event_number == event_backwards){ /* on our first step back */
		s1 -> present_children = malloc(sizeof(speciment)); 
		s1 -> present_children -> prs = malloc(sizeof(person));
		s1 -> present_children -> prs -> pid = s -> prs -> pid;
		s1 -> present_children -> prs -> flag = 'u';
		s1 -> present_children -> prs -> fitness = 0;
		s1 -> present_children -> prs -> parent1 = NULL;
		s1 -> present_children -> prs -> parent2 = NULL;
		s2 -> present_children = malloc(sizeof(speciment));
		s2 -> present_children -> prs = malloc(sizeof(person));
		s2 -> present_children -> prs -> pid = s -> prs -> pid;
		s2 -> present_children -> prs -> flag = 'u';
		s2 -> present_children -> prs -> fitness = 0;
		s2 -> present_children -> prs -> parent1 = NULL;
		s2 -> present_children -> prs -> parent2 = NULL;
	}
	else{
		if ( s -> present_children -> next != NULL) /* multiple present children */
			multiflag = 1;
		else{
			s1 -> present_children = malloc(sizeof(speciment));
			s1 -> present_children -> prs = malloc(sizeof(person));
			s1 -> present_children -> prs -> pid = s -> present_children -> prs -> pid;
			s1 -> present_children -> prs -> flag = 'u';
			s1 -> present_children -> prs -> fitness = 0;
			s1 -> present_children -> prs -> parent1 = NULL;
			s1 -> present_children -> prs -> parent2 = NULL;
			s2 -> present_children = malloc(sizeof(speciment));
			s2 -> present_children -> prs = malloc(sizeof(person));
			s2 -> present_children -> prs -> pid = s -> present_children -> prs -> pid;
			s2 -> present_children -> prs -> flag = 'u';
			s2 -> present_children -> prs -> fitness = 0;
			s2 -> present_children -> prs -> parent1 = NULL;
			s2 -> present_children -> prs -> parent2 = NULL;
		}
	}
	if (!multiflag){
		s1 -> present_children -> children = NULL;
		s1 -> present_children -> present_children = NULL;
		s1 -> present_children -> segments = malloc(sizeof(segment)); 
		s1 -> present_children -> segments -> start = 0;
		s1 -> present_children -> segments -> end = cutting_point;
		s1 -> present_children -> segments -> next = NULL;
		s1 -> present_children -> next = NULL;
		s1 -> present_children -> prev = NULL;
		s2 -> present_children -> children = NULL;
		s2 -> present_children -> present_children = NULL;
		s2 -> present_children -> segments = malloc(sizeof(segment));
		if (cutting_point != MAX_SEGMENT)
			s2 -> present_children -> segments -> start = cutting_point + 1;
		else
			s2 -> present_children -> segments -> start = cutting_point;
		s2 -> present_children -> segments -> end = MAX_SEGMENT;
		s2 -> present_children -> segments -> next = NULL;
		s2 -> present_children -> next = NULL;
		s2 -> present_children -> prev = NULL;
		unique_storage(s1 -> present_children); 
		unique_storage(s2 -> present_children);
	}
	segment * seg1 = malloc(sizeof(segment)); 
	seg1 -> start = 0;
	seg1 -> end = 0;
	seg1 -> next = NULL;
	segment * seg2 = malloc(sizeof(segment));
	seg2 -> start = 0;
	seg2 -> end = 0;
	seg2 -> next = NULL;
	
	if (s -> segments -> next){ /* if there are more than one segments */
		segment * tmp = malloc(sizeof(segment));
		tmp = s -> segments;
		segment * tmp1 = calloc(1, sizeof(segment));
		seg1 = tmp1;
		while (tmp &&  cutting_point > tmp -> start){	/* till we reach the cutting point  */
			if (!tmp1) /* for the first time we enter the loop */
				tmp1 = tmp;
			else {
				tmp1 -> next = tmp;
				tmp1 = tmp1 -> next;
			}
			tmp = tmp -> next;
		}
		if (tmp !=NULL && !tmp1){ /* if the cutting point was in the first segment */
			seg1 -> start = tmp -> start;
			seg1 -> end = cutting_point;
			seg2 -> start = cutting_point +1;
			seg2 -> end = tmp -> end;
			seg2 -> next = tmp -> next;
		}
		else if (tmp != NULL && cutting_point < tmp -> end){
			tmp1 -> next = malloc(sizeof(segment));
			tmp1 -> next -> start = tmp -> start;
			tmp1-> next -> end = cutting_point;
			tmp1 -> next -> next = NULL;
			seg2 -> start = cutting_point + 1;																																
			seg2 -> end = s -> segments -> end;
			seg2 -> next = tmp -> next;
		}
		else{ /* meaning that the cutting point was before the first or after the last segment */
			s1 -> segments = s -> segments;
			s1 -> present_children = s -> present_children;
			s1 -> present_children -> segments = seg1;
			insert_queue(s1);
			free(s2 -> prs);
			//free(s2 -> present_children -> segments);
			s2 -> present_children = NULL;
			if (!multiflag)
				free(s2 -> present_children);
			free(seg2);
			return;
		}
	}
	else if ( cutting_point < s -> segments -> start && cutting_point > s -> segments -> end) { /* if the random cutting point doesn't match the person's segment we simply do not need to recombine, we keep whichever parent we chose as s1 */
		s1 -> segments = s -> segments;
		s1 -> present_children = s -> present_children;
		s1 -> present_children -> segments = seg1;
		insert_queue(s1);
		free(s2 -> prs);
		//free(s2 -> present_children -> segments);
		s2 -> present_children = NULL;
		if (!multiflag)
			free(s2 -> present_children);
		free(s2);	/* no need for this anymore */
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
	if (multiflag){ /* if there are multiple present_children */
		speciment * multmp = s -> present_children;
		speciment * mulspec1 = NULL;
		speciment * mulspec2 = NULL;
		while (multmp != NULL){
			if (mulspec1 != NULL){
				mulspec1 -> next = malloc(sizeof(speciment));
				mulspec1 -> next -> prs = malloc(sizeof(person));
				mulspec1 -> next -> prs -> pid = multmp -> prs -> pid;
				mulspec1 -> next -> prs -> flag = 'r';
				mulspec1 -> next -> prs -> fitness = 0;
				mulspec1 -> next -> prs -> parent1 = NULL;
				mulspec1 -> next -> prs -> parent2 = NULL;
				mulspec1 -> next  -> children = NULL;
				mulspec1 -> next  -> present_children = NULL;
				mulspec1 -> next  -> segments = seg1;
				mulspec1 -> next  -> next = NULL;
				mulspec1 -> next  -> prev = NULL;
				mulspec2 -> next  = malloc(sizeof(speciment));
				mulspec2 -> next -> prs = malloc(sizeof(person));
				mulspec2 -> next -> prs -> pid = multmp -> prs -> pid;
				mulspec2 -> next -> prs -> flag = 'r';
				mulspec2 -> next -> prs -> fitness = 0;
				mulspec2 -> next -> prs -> parent1 = NULL;
				mulspec2 -> next -> prs -> parent2 = NULL;
				mulspec1 -> next  -> children = NULL;
				mulspec2 -> next  -> children = NULL;
				mulspec2 -> next  -> present_children = NULL;
				mulspec2 -> next  -> segments = seg2;
				mulspec2 -> next  -> next = NULL;
				mulspec2 -> next  -> prev = NULL;
				unique_storage(mulspec1 -> next); //edw
				unique_storage(mulspec2 -> next);
			}
			else{
				mulspec1 = malloc(sizeof(speciment));
				mulspec1 -> prs = malloc(sizeof(person));
				mulspec1 -> prs -> pid = multmp -> prs -> pid;
				mulspec1 -> prs -> flag = 'r';
				mulspec1 -> prs -> fitness = 0;
				mulspec1 -> prs -> parent1 = NULL;
				mulspec1 -> prs -> parent2 = NULL;
				mulspec1 -> children = NULL;
				mulspec1 -> present_children = NULL;
				mulspec1 -> segments = seg1;
				mulspec1 -> next = NULL;
				mulspec1 -> prev = NULL;
				mulspec2 = malloc(sizeof(speciment));
				mulspec2 -> prs = malloc(sizeof(person));
				mulspec2 -> prs -> pid = multmp -> prs -> pid;
				mulspec2 -> prs -> flag = 'r';
				mulspec2 -> prs -> fitness = 0;
				mulspec2 -> prs -> parent1 = NULL;
				mulspec2 -> prs -> parent2 = NULL;
				mulspec2 -> children = NULL;
				mulspec2 -> present_children = NULL;
				mulspec2 -> segments = seg2;
				mulspec2 -> next = NULL;
				mulspec2 -> prev = NULL;
				s1 -> present_children = mulspec1;
				s2 -> present_children = mulspec2;
				unique_storage(mulspec1); //edw
				unique_storage(mulspec2);
			}
			mulspec1 = mulspec1 -> next;
			mulspec2 = mulspec2 ->next;
			multmp = multmp -> next;
		}
	}
	insert_queue(s1);
	insert_queue(s2);
}

void choose_parent(speciment* s){
	if (s -> prs -> parent1 -> pid != s -> prs -> parent2 -> pid){	/* for single-parented children recombination is obviously useless */
		unsigned recombination = rand() % 99;
		if ( recombination < 1) {
			recombine(s);
			return;
		}
	}
	speciment * tmp = malloc(sizeof(speciment));
	tmp -> prs = malloc(sizeof(person));
	if ( (s -> prs -> parent1 -> pid == s -> prs -> parent2 -> pid) || (rand() % 99 < 50) ){ 	/* single parented so we don't need to choose or we randomly select the first */
		tmp -> prs -> pid = s -> prs -> parent1 -> pid;
		tmp -> prs -> parent1 = s -> prs -> parent1 -> parent1;
		tmp -> prs -> parent2 = s -> prs -> parent1 -> parent2;
	}
	else{
		tmp -> prs -> pid = s -> prs -> parent2 -> pid;
		tmp -> prs -> parent1 = s -> prs -> parent2 -> parent1;
		tmp -> prs -> parent2 = s-> prs -> parent2 -> parent2;
	}
	tmp -> prs -> flag = 0;
	tmp -> prs -> fitness = s -> prs -> fitness;
	tmp -> prs -> next = NULL;
	tmp -> segments = s -> segments;
	if (event_number == event_backwards){ /* on our first step back */
		tmp -> present_children = malloc(sizeof(speciment));
		tmp -> present_children -> prs = malloc(sizeof(person));
		tmp -> present_children -> prs -> pid = s -> prs -> pid;
		tmp -> present_children -> prs -> flag = 'u';
		tmp -> present_children -> prs -> fitness  = s -> prs -> fitness;
		tmp -> present_children -> prs -> parent1 = NULL;
		tmp -> present_children -> prs -> parent2 = NULL;
		tmp -> present_children -> prs -> next = NULL;
		tmp -> present_children -> children = NULL;
		tmp -> present_children -> present_children = NULL;
		tmp -> present_children -> segments = malloc(sizeof(segment));
		tmp -> present_children -> segments -> start = 0;
		tmp -> present_children -> segments -> end = MAX_SEGMENT;
		tmp -> present_children -> segments -> next = NULL;
		tmp -> present_children -> next = NULL;
		tmp -> present_children -> prev = NULL;
		unique_storage(tmp -> present_children); 
	}
	else{
		tmp -> present_children = s -> present_children;
	}
	 /* we are creating a new top - down tree this time so we can go forward afterwards */
	 tmp -> children = s;
	insert_queue(tmp);
}

void create_parent(speciment * s){ /* we create a new parent to go further back in time. In case of recombination we create 2 of them */
	person * parent1 = malloc(sizeof(person));														
	parent1 -> pid = rand() % (curr_gen + prev_gen);	/* the range of the possible parents equals the things we have in queue thus far */
	parent1 -> flag = 1;
	parent1 -> fitness = s -> prs -> fitness;
	parent1 -> next = NULL;
	parent1 -> parent1 = NULL;
	parent1 -> parent2 = NULL;
	
	unsigned recombination = rand() % 99;
	if ( recombination < 1 ){
		person * parent2 = malloc(sizeof(person));
		parent2 -> pid =  rand() % (curr_gen + prev_gen);
		while (parent2 -> pid == parent1 -> pid) /* making sure we don't end up with a single parent */
			parent2 -> pid = rand() % (curr_gen + prev_gen);
		parent2 -> flag = 1;
		parent2 -> fitness = s -> prs -> fitness;
		parent2 -> next = NULL;
		parent2 -> parent1 = NULL;
		parent2 -> parent2 = NULL;
		s -> prs -> parent1 = parent1;
		s -> prs -> parent2 = parent2;
		recombine(s);
		return;
	}
	speciment * tmp = malloc (sizeof(speciment));	
	tmp -> prs = parent1;
	tmp -> segments = s -> segments;
	tmp -> present_children = s -> present_children;
	tmp -> children = s;
	insert_queue(tmp);
}

void go_back(){
	printf("go back\n");
	speciment * tmp;
	while ( event_backwards > 0 && prev_gen != 1){  /* we stop going back either when we find a common ancestor for each segment or when we reach step 0 */ 
		if ( prev_gen != 0 ) /* so in every step but the first */
			curr_gen = prev_gen; /* we move a generation back in time */
		prev_gen = 0;
		//printf ("Event %d has %d people \n", event_backwards, curr_gen);
		while (curr_gen > 0){
			tmp = tail;
			choose_parent(tmp);
			tail = tail -> prev;
			tail -> next = NULL;
			curr_gen--;
		}
		total += prev_gen;
		event_backwards--;
	}
}

void further_back(){	/* we might need to go to generations before zero to trace a common ancestor */
	printf("go further back\n");
	speciment * tmp;
	while ( prev_gen != 1){	/* we stop going back only when we discover a common ancestor for each segment */ 
		curr_gen = prev_gen;	/* we move a generation back in time */
		prev_gen = 0;
		printf ("Negative Event %d has %d people \n", negative_generations, curr_gen);
		while (curr_gen > 0){
			tmp = tail;
			create_parent(tmp);
			tail = tail -> prev;
			tail -> next = NULL;
			curr_gen--;
		}
		total += prev_gen;
		negative_generations++;
	}
}

/* ---------------------------------- back in time end  ---------------------------------- */

/* ------------------------------------- free start ----------------------------------------- */
void free_present(unifree * u){
	if (u -> next != NULL)
		free_present(u -> next);
	//if (u -> spec -> prs -> flag == 'u')
	//	free_seg(u -> spec -> segments);
	free(u -> spec -> prs);
	free(u -> spec);
	free(u);
}


void free_seg(segment * seg){ /* self-explanatory */
	if (seg == NULL)
		return;
	if (seg -> next != NULL)
		free_seg(seg -> next);
	free(seg);
}

void tree_destruction(speciment * s){ /* destroys the generation tree we created */
	if (s == NULL)
		return;
	if (s -> prs -> flag != 'r'){ /* 2nd parent in recombination. Same children as parent1 so already set free  */
		if (s -> children != NULL)
			tree_destruction(s -> children);
	}
	if (s -> next != NULL)
		tree_destruction(s -> next);
	if (s -> prs -> flag == 'u' || s -> prs -> flag == 'r')
		free_seg(s -> segments);
	s -> prev = NULL;
	free(s -> prs);
	free(s);
}

/* -------------------------------------- free end ------------------------------------------ */

void print_present_children(speciment * s){
	FILE * f1;
	f1 = fopen("present_children.txt", "a");
	fprintf(f1,"----present children of %d in gen %d ---\n", s -> prs -> pid, (event_backwards - negative_generations));
	speciment * tmp = s -> present_children;
	segment * seg;
	unsigned counter = 0;
	while (tmp){
		fprintf(f1, "child: %d with segments:\t", tmp -> prs -> pid);
		seg = tmp -> segments;
		while (seg){
			fprintf(f1, "[%d %d] ", seg -> start, seg -> end);
			seg = seg -> next;
		}
		fprintf(f1, "\n");
		tmp = tmp -> next;
		counter++;
	}
	fprintf (f1, "children: %d\n", counter);
	fclose(f1);
}

void print_segments(segment * tmp){
	FILE * f1;
	f1 = fopen("merge_seg.txt", "a");
	segment * seg = tmp;
	while (seg){
		fprintf(f1, " [%d %d] ", seg -> start, seg -> end);
		seg = seg -> next;
	}
	fprintf(f1,"\n");
	tmp = tmp -> next;
	fclose(f1);
}

/* sort of a main for this step */
void rewind_time(){
	event_backwards = event_number;
	sampling(samples);
	go_back();
	keepsake = prev_gen; 	/* keep track of the people we have in generation zero */
	if ( prev_gen !=1 )		/* meaning we partially "failed" during the initial traceback */
		further_back();
	printf("Total people encountered: %d\n", total);
	printf("Speciment: %d is the common ancestor\n", head -> prs -> pid);
	destruction();/* not useful anymore so no need to keep all this memory allocated */	
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