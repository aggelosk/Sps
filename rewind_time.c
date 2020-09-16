#include "rewind_time.h"

#define current	A[event_number][i][j]

#define DEFAULT_SAMPLES 100

time_t t;

extern unsigned event_number;
extern unsigned rows;
extern unsigned columns;
extern spot **** A;
extern unsigned * People;
extern void destruction();

extern unsigned samples = 0;
int event_backwards;
unsigned int negative_generations = 0;		/* in case we need to go further back in time to discover the common ancestor, we keep track of our steps */
unsigned curr_gen = 0;
unsigned prev_gen = 0;
unsigned keepsake = 0;				/* for tracing further back in time purposes */
char smpl = 1;					/* checks if we are still in sampling or not. Default value 1(true) since we begin this step by picking samples */

unsigned total = 0; 				/* total number of people we ve encountered in our journey back in time */

extern person * samp;				/* besides using the sampled people to find common ancestors, we need to keep them in a list for future use */
//person * samp_tail;
speciment * head;
speciment * tail;
rec_free * rec_head = NULL;

unsigned max_segment = 1000;

unsigned * affect;

unsigned currK = 10;
unsigned nk =0;

int ben_pos = -1; /* position for the beneficial mutation */

/* ----- specified sampling start ------- */
char boxs = 0; 		/* default is to pick a uniformed sample */
char ancs = 0;		/* default is to pick samples from the present only */
boxed * boxhd = NULL;
void area_sampling(unsigned sam, unsigned srow, unsigned erow, unsigned scol, unsigned ecol, unsigned gen);
void sampling(unsigned samples);
extern double rec_rate;
/* ------ specified sampling end -------- */

unsigned created_people = 0;

void add_recfree(preschild * p){
  rec_free * tmp = malloc(sizeof(rec_free));
  tmp -> pres = p;
  tmp -> next = rec_head;
  rec_head = tmp;
}


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

		if (seg1 -> start == seg2 -> start ){	/* start from the same point ~ overlapping */
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

		else if (seg1 -> end == seg2 -> end){	/* end at the same point ~ overlapping */
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

		else if(seg1 -> start < seg2 -> start){	/* seg1 starts before seg2 */
			if (t == NULL)
				t = seg1;
			else{
				t -> next = seg1;
				t = t -> next;
			}
			if (!(t -> end < seg2 -> start - 1)){ /* overlapping */
				t -> end = max(t -> end, seg2 -> end);
				seg2 = seg2 -> next;
			}
			seg1 = seg1 -> next;
		}

		else{	/* seg2 starts before seg1 */
			if (t == NULL)
				t = seg2;
			else{
				t -> next = seg2;
				t = t -> next;
			}
			if (!(t -> end < seg1 -> start - 1)){ /* overlapping */
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
	return t;
}

speciment * merge(speciment * s1, speciment * s2){
	preschild * p1 = s1 -> present_children;
	preschild * p2 = s2 -> present_children;
	preschild * tmp = NULL;;
	while ( p1 && p2){
		if (p1 -> pid == p2  -> pid){ /* duplicate prevention */
			p1 -> segments = merge_segments(p1 -> segments, p2 -> segments);
			//print_segments(p1 -> segments);
			p1 = p1 -> next;
			p2 = p2 -> next;
		}
		else if (p1  -> pid > p2 -> pid){ /* need to merge */
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
	free(s2 -> prs);
	free(s2);
	return s1;
}

/* --------------------------------------- insert - delete queue start ---------------------------------------- */

void insert_queue(speciment * s){
	assert(s != NULL); /* just a safety clause should never be triggered */
	s -> prev = NULL;
	if (!head){
		head = s;
		tail = head;
		++prev_gen;
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
			++pg;
			tmp = tmp -> next;
		}
	}
	head -> prev = s;
	s -> next = head;
	head = s;
	++prev_gen;
}

/* ------------------------------------------- back in time start ------------------------------------------- */

void recombine(speciment * s){
	/* initialing the speciments describing the 2 parents */
	speciment * s1 = malloc(sizeof(speciment));
	speciment * s2 = malloc(sizeof(speciment));
	s1 -> prs = malloc(sizeof(person));
	s2 -> prs = malloc(sizeof(person));
	double random  = rand() / (double)RAND_MAX;


  int cutting_point = (int)(rand() % (max_segment + 1)) - 1;	/* we choose a cutting point */

  /* we need to decide which parent takes the left part of the genome and which the other one */
	if (cutting_point > ben_pos){ /* always true when there is no beneficial mutation but doesn't affect the outcome so no worries */
    memcpy(s1 -> prs, s -> prs -> parent1, sizeof(person));
    memcpy(s2 -> prs, s -> prs -> parent2, sizeof(person));
	}
	else {
    memcpy(s1 -> prs, s -> prs -> parent2, sizeof(person));
    memcpy(s2 -> prs, s -> prs -> parent1, sizeof(person));
	}

	s1 -> prs -> flag = 'r'; /* flagging it for use in free */
	s2 -> prs -> flag = 'r'; /* flagging it for use in free */
	s1 -> present_children = NULL;
	s2 -> present_children = NULL;
	s1 -> next = NULL;
	s1 -> prev = NULL;
	s2 -> next = NULL;
	s2 -> prev = NULL;

	/* add the parent position to the coords */

  /* --------------------------------- we now need to "cut" the genome -----------------------------------*/
	if ( cutting_point < 0){ /* give it all to the second parent */
		s2 -> present_children = s -> present_children;
		free(s1 -> prs);
		free(s1);
		insert_queue(s2);
		return;
	}
	if (cutting_point == max_segment){ /* give it all to the first parent */
		s1 -> present_children = s -> present_children;
		free(s2 -> prs);
		free(s2);
		insert_queue(s1);
		return;
	}

	if (event_number == event_backwards){ /* on our first step back ~ meaning we only have one present child w/ one segment*/
		s1 -> present_children = malloc(sizeof(preschild));
		s1 -> present_children -> pid = s -> prs -> pid;
		s1 -> present_children -> next = NULL;
		s1 -> present_children -> prev = NULL;
		s1 -> present_children -> segments = malloc(sizeof(segment));
		s1 -> present_children -> segments -> start = 0;
		s1 -> present_children -> segments -> end = cutting_point;
		s1 -> present_children -> segments -> next = NULL;
		s1 -> present_children -> segments -> coords = malloc(sizeof(coord));
		s1 -> present_children -> segments -> coords -> r = s -> prs -> row;
		s1 -> present_children -> segments -> coords -> c = s -> prs -> col;
	 	s1 -> present_children -> segments -> coords -> next = NULL;

		s2 -> present_children = malloc(sizeof(preschild));
		s2 -> present_children -> pid = s -> prs -> pid;
		s2 -> present_children -> next = NULL;
		s2 -> present_children -> prev = NULL;
		s2 -> present_children -> segments = malloc(sizeof(segment));
		s2 -> present_children -> segments -> start = cutting_point +1;
		s2 -> present_children -> segments -> end = max_segment;
		s2 -> present_children -> segments -> next = NULL;
  	s2 -> present_children -> segments -> coords = malloc(sizeof(coord));
    s2 -> present_children -> segments -> coords -> r = s -> prs -> row;
    s2 -> present_children -> segments -> coords -> c = s -> prs -> col;
    s2 -> present_children -> segments -> coords -> next = NULL;

    insert_queue(s1);
		insert_queue(s2);
		return;
	}

	/* ---------------------------------- we now proceed to more "complex" cases ---------------------------------- */
	preschild * prc = s -> present_children;
	preschild * ch1 = NULL;
	s1 -> present_children = ch1;
	preschild * ch2 = NULL;
	s2 -> present_children = ch2;
	while (prc != NULL){
		if (ch1 == NULL){
			ch1 = malloc(sizeof(preschild));
			ch1 -> pid = prc -> pid;
			ch1 -> segments = NULL;
			ch1 -> next = NULL;
			ch1 -> prev = NULL;
			s1 -> present_children = ch1;
		}
		else{
		  ch1 -> next = malloc(sizeof(preschild));
			ch1 -> next -> pid = prc -> pid;
			ch1 -> next -> segments = NULL;
			ch1 -> next -> next = NULL;
			ch1 -> next -> prev = ch1;
			ch1 = ch1 -> next;
		}
		if (ch2 == NULL){
		  ch2 = malloc(sizeof(preschild));
			ch2 -> pid = prc -> pid;
			ch2 -> segments = NULL;
			ch2 -> next = NULL;
			ch2 -> prev = NULL;
			s2 -> present_children = ch2;
		}
		else{
			ch2 -> next = malloc(sizeof(preschild));
			ch2 -> next -> pid = prc -> pid;
			ch2 -> next -> segments = NULL;
			ch2 -> next -> next = NULL;
			ch2 -> next -> prev = ch2;
			ch2 = ch2 -> next;
		}

		/* -------------------------------------- we now handle the segments -------------------------------------- */
		segment * tmp = prc -> segments;
		assert(tmp != NULL);
		segment * seg1 = NULL;
		segment * seg2 = NULL;

		ch1 -> segments = seg1;
		ch2 -> segments = seg2;
    coord * co = NULL;
		while (tmp != NULL && tmp -> start < cutting_point){ /* while this present_child still has segments && we haven't passed the cutting point */

      /* we first add the new spatial origin to each segment before we determine its parent, since they have to be from the same area to reproduce */
      co = malloc(sizeof(coord));
      co -> r = s1 -> prs -> row;
      co -> c = s1 -> prs -> col;
      co -> next = tmp -> coords;
      tmp -> coords = co;

			if ( tmp -> end < cutting_point){ /* segment before the split */
				if (seg1 != NULL){
					seg1 -> next = malloc(sizeof(segment));
          memcpy(seg1 -> next, tmp, sizeof(segment));
					seg1 -> next -> coords = malloc(sizeof(coord));
          memcpy(seg1 -> next -> coords, tmp -> coords, sizeof(coord));
					seg1 -> next -> next = NULL;
					seg1 = seg1 -> next;
				}
				else{
					seg1 = malloc(sizeof(segment));
					seg1 -> start = tmp -> start;
					seg1 -> end = tmp -> end;
          seg1 -> coords = malloc(sizeof(coord));
          memcpy(seg1 -> coords, tmp -> coords, sizeof(coord));
					seg1 -> next = NULL;
					ch1 -> segments = seg1;
				}
			}
			else{  /* cutting point somewhere in the segment */
				if (seg1 != NULL){ /* first(pre-cutting point) part goes to parent 1 */
					seg1 -> next = malloc(sizeof(segment));
					seg1 -> next -> start = tmp -> start;
					seg1 -> next -> end = cutting_point;
          seg1 -> next -> coords = malloc(sizeof(coord));
          memcpy(seg1 -> next -> coords, tmp -> coords, sizeof(coord));
					seg1 -> next -> next = NULL;
					seg1 = seg1 -> next;
				}
				else{
				  seg1 = malloc(sizeof(segment));
					seg1 -> start = tmp -> start;
					seg1 -> end = cutting_point;
          seg1 -> coords = malloc(sizeof(coord));
          memcpy(seg1 -> coords, tmp -> coords, sizeof(coord));
					seg1 -> next = NULL;
					ch1 -> segments = seg1;
				}
				if (cutting_point != tmp -> end){ /* if the whole segment does not go to parent 1 */
					if (seg2 != NULL){ /* second(post-cutting point) part goes to parent 2 */
						seg2 -> next = malloc(sizeof(segment));
						seg2 -> next -> start = cutting_point + 1;
						seg2 -> next -> end = tmp -> end;
            seg2 -> next -> coords = malloc(sizeof(coord));
            memcpy(seg2 -> next -> coords, tmp -> coords, sizeof(coord));
						seg2 -> next -> next = NULL;
						seg2 = seg2 -> next;
					}
					else{
						seg2 = malloc(sizeof(segment));//leak
						seg2 -> start = cutting_point + 1;
						seg2 -> end = tmp -> end;
            seg2 -> coords = malloc(sizeof(coord));
            memcpy(seg2 -> coords, tmp -> coords, sizeof(coord));
						seg2 -> next = NULL;
						ch2 -> segments = seg2;
					}
				}
			}
			tmp = tmp -> next;
		}
		while (tmp != NULL){ /* passed the cutting point, but there were still segments - they all go to parent2 */
			if (seg2 != NULL){
				seg2 -> next = malloc(sizeof(segment));
				seg2 -> next -> start = tmp -> start;
				seg2 -> next -> end = tmp -> end;
        seg2 -> next -> coords = malloc(sizeof(coord));
        memcpy(seg2 -> next -> coords, tmp -> coords, sizeof(coord));
				seg2 -> next -> next = NULL;
				seg2 = seg2 -> next;
			}
			else{
        seg2 = malloc(sizeof(segment));
				seg2 -> start = tmp -> start;
				seg2 -> end = tmp -> end;
        seg2 -> coords = malloc(sizeof(coord));
        memcpy(seg2 -> coords, tmp -> coords, sizeof(coord));
				seg2 -> next = NULL;
				ch2 -> segments = seg2;
			}
			tmp = tmp -> next;
		}
		if (ch1 -> segments == NULL){ /* parent1 ended up not influencing this present child */
			free_seg(ch1 -> segments);
			free_present(ch1);
			ch1 = NULL;
			s1 -> present_children = NULL;
		}
		if (ch2 -> segments == NULL){ /* parent2 ended up not influencing this present child */
			free_seg(ch2 -> segments);
			free_present(ch2);
			ch2 = NULL;
			s2 -> present_children = NULL;
		}
		prc = prc -> next;
	}
	if (s1 -> present_children != NULL) /* if parent1 ended up infuencing no-one there is no need to add him to the tree */
		insert_queue(s1);
	else{
	  free_present(s1 ->present_children); //new
		free(s1 -> prs);
		free(s1);
	}
	if (s2 -> present_children != NULL) /* if parent1 ended up infuencing no-one there is no need to add him to the tree */
		insert_queue(s2);
	else{
	  free_present(s2 -> present_children); //new
		free(s2 -> prs);
		free(s2);
	}
}

void choose_parent(speciment* s){
	/* first we check for recombination */
	if (s -> prs -> parent2 != NULL){
		recombine(s);
		return;
	}
	speciment * tmp = malloc(sizeof(speciment));
	tmp -> prs = malloc(sizeof(person));
	double random = (double)rand() / RAND_MAX;
  memcpy(tmp -> prs, s -> prs -> parent1, sizeof(person));
	tmp -> prs -> flag = 0;
	tmp -> prs -> fitness = s -> prs -> fitness;
	if (event_number == event_backwards){ /* on our first step back */
		tmp -> present_children = malloc(sizeof(preschild));
		tmp -> present_children -> pid = s -> prs -> pid;
		tmp -> present_children -> segments = malloc(sizeof(segment));
		tmp -> present_children -> segments -> start = 0;
		tmp -> present_children -> segments -> end = max_segment;
		tmp -> present_children -> segments -> next = NULL;
		tmp -> present_children -> segments -> coords = malloc(sizeof(coord));
		tmp -> present_children -> segments -> coords -> r = s -> prs -> row;
		tmp -> present_children -> segments -> coords -> c = s -> prs -> col;
    tmp -> present_children -> segments -> coords -> next = NULL;
		tmp -> present_children -> next = NULL;
		tmp -> present_children -> prev = NULL;
	}
	else{
		tmp -> present_children = s -> present_children;
		/* for the spatial origin scenario */
    preschild * pr = tmp -> present_children;
    while (pr != NULL){ /* for each present child */
  		segment * segm = pr -> segments;
  		while ( segm != NULL){
  			/* we need to add the current position to the head of the  list */
  			coord * co = malloc(sizeof(coord));
  			co -> r = tmp -> prs -> row;
  			co -> c = tmp -> prs -> col;
  			co -> next = segm -> coords;
  			segm -> coords = co;
  			segm = segm -> next;
  		}
      pr = pr -> next;
    }
	}
	insert_queue(tmp);
}

void create_parent(speciment * s){ /* we create a new parent to go further back in time. In case of recombination we create 2 of them */
	person * parent1 = malloc(sizeof(person));
	parent1 -> pid = rand() % (People[0]);	/* the range of the possible parents equals the initial population size */
	parent1 -> flag = '1';
	parent1 -> fitness = 1;
	parent1 -> parent1 = NULL;
	parent1 -> parent2 = NULL;
	/* ------ */
	parent1 -> row = s -> prs -> row; /* no migration takes place prior to gen 0 */
	parent1 -> col = s -> prs -> col; /* no migration takes place prior to gen 0 */
	unsigned recombination = ((float)rand()/(float)(RAND_MAX));
	if (recombination < rec_rate){
		person * parent2 = malloc(sizeof(person));
		parent2 -> pid = rand() % (curr_gen + prev_gen);
		while (parent2 -> pid == parent1 -> pid) /* making sure we don't end up with a single parent */
			parent2 -> pid = rand() % (curr_gen + prev_gen);
		parent2 -> flag = '1';
		parent2 -> fitness = 1;
    parent2 -> row = s -> prs -> row; /* no migration takes place prior to gen 0 */
    parent2 -> col = s -> prs -> col; /* no migration takes place prior to gen 0 */
		parent2 -> parent1 = NULL;
		parent2 -> parent2 = NULL;
		s -> prs -> parent1 = parent1;
		s -> prs -> parent2 = parent2;
		recombine(s);
		return;
	}
	speciment * tmp = malloc (sizeof(speciment));
	tmp -> prs = parent1;
	tmp -> present_children = s -> present_children;
  /* for the spatial origin scenario */
  preschild * pr = tmp -> present_children;
  while (pr != NULL){ /* for each present child */
    segment * segm = pr -> segments;
    while ( segm != NULL){
      /* we need to add the current position to the head of the  list */
      coord * co = malloc(sizeof(coord));
      co -> r = tmp -> prs -> row;
      co -> c = tmp -> prs -> col;
      co -> next = segm -> coords;
      segm -> coords = co;
      segm = segm -> next;
    }
    pr = pr -> next;
  }
	insert_queue(tmp);
	++created_people;
}

void add_present(speciment * s){
  //  static int nk = 0;
	static int lid = 0;
	++nk;
	if(nk > currK - 1){
		currK += currK;
		anc = realloc(anc, currK * sizeof(struct ancestry));
	}
	if (s -> present_children == NULL){ /* meaning we are in the present */
	  anc[nk-1].affChildren = calloc(1, sizeof(affected));
	  s -> prs -> pid = --lid;		/* we use negative indexes to seperate them from the rest */
	  anc[nk-1].affChildren[0].num = s -> prs -> pid;
	  anc[nk-1].presentChildren = 1;
	  anc[nk-1].id = s -> prs -> pid;
	  /* -------------------------------------------------------- */
	  anc[nk-1].affChildren[0].segments = malloc(sizeof(segment));
	  anc[nk-1].affChildren[0].segments -> start = 0;
	  anc[nk-1].affChildren[0].segments -> end = max_segment;
	  anc[nk-1].affChildren[0].segments -> next = NULL;
	  anc[nk-1].affChildren[0].segments -> coords = malloc(sizeof(coord));
	  anc[nk-1].affChildren[0].segments -> coords -> r = s -> prs -> row;
    anc[nk-1].affChildren[0].segments -> coords -> r = s -> prs -> col;
	  anc[nk-1].affChildren[0].segments -> coords -> next = NULL;
	  /* -------------------------------------------------------- */
	  ++affect[1];
	  return;
	}

	preschild * tmp = s -> present_children;
	unsigned counter = 0;
	while (tmp != NULL){
		tmp = tmp -> next;
		++counter;
	}

	anc[nk-1].affChildren = calloc(counter, sizeof(affected));
	anc[nk-1].presentChildren = 0;
	anc[nk-1].id = s -> prs -> pid;
	unsigned i = 0;
	tmp = s -> present_children;
	segment * seg = NULL;
	while (tmp != NULL){
	  ++i;
	  ++anc[nk-1].presentChildren;
	  anc[nk-1].affChildren[i-1].num = tmp -> pid; /* compare with original */
	  seg = tmp -> segments;
	  anc[nk-1].affChildren[i-1].segments = tmp -> segments;
	  tmp = tmp -> next;
	}
	++affect[i];
}

void go_back(){
  //printf("go back\n");
  speciment * tmp;
  while ( event_backwards > 0 && prev_gen != 1){  /* we stop going back either when we find a common ancestor for each segment or when we reach step 0 */
    if (ancs && boxhd != NULL && event_backwards != event_number && event_backwards == event_number - boxhd -> gen){ /* add samples for this generation */
      area_sampling(boxhd -> samples, boxhd -> srow, boxhd -> erow, boxhd -> scol, boxhd -> ecol, boxhd -> gen);
      boxed * boxf = boxhd;
      boxhd = boxhd -> next;
      free(boxf);
    }
    if ( prev_gen != 0 ) /* so in every step but the first */
      curr_gen = prev_gen; /* we move a generation back in time */
    prev_gen = 0;
    while (curr_gen > 0){
      tmp = tail;
      add_present(tmp);
      choose_parent(tmp);
      tail = tail -> prev;
      assert(tail != NULL);
      if (tmp != NULL){
      	free(tmp -> prs);
      	free(tmp);
      }
      tail -> next = NULL;
	    curr_gen--;
    }
    total += prev_gen;
    event_backwards--;
  }
}

void further_back(){	/* we might need to go to generations before zero to trace a common ancestor */
  //printf("go further back\n");
  speciment * tmp;
  while ( prev_gen != 1){	/* we stop going back only when we discover a common ancestor for each segment */
    curr_gen = prev_gen;	/* we move a generation back in time */
    prev_gen = 0;
    //printf ("Negative Event %d has %d people \n", negative_generations, curr_gen);
    while (curr_gen > 0){
      tmp = tail;
      add_present(tmp);
      create_parent(tmp);
      tail = tail -> prev;
      assert(tail != NULL);
      if (tmp != NULL){
	free(tmp -> prs);
	free(tmp);
      }
      tail -> next = NULL;
      curr_gen--;
    }
    total += prev_gen;
    ++negative_generations;
  }
}

/* ---------------------------------- back in time end  ---------------------------------- */

/* ------------------------------------- free start ----------------------------------------- */

/* recombination causes additional allocations that we need to free to avoid memory leaks */
void recomb_free(){
  rec_free * tmp = NULL;
  while (rec_head != NULL){
    tmp = rec_head;
    free_present(tmp -> pres);
    rec_head = rec_head -> next;
    free(tmp);
  }
}

void free_coords(coord * co){
  if (co == NULL)
    return;
  if (co -> next != NULL)
    free_coords(co -> next);
  free(co);
}

void free_seg(segment * seg){ /* self-explanatory */
  if (seg == NULL)
    return;
  if (seg -> next != NULL){
    free_coords(seg -> coords);
    free_seg(seg -> next);
  }
  free(seg);
}

void free_present(preschild * p){
  preschild * tmp = NULL;
  while (p != NULL){
    tmp = p;
    p = p -> next;
    tmp -> prev = NULL;
    free_seg(tmp -> segments);
    free(tmp);
  }
}

void tree_destruction(speciment * s){ /* destroys the generation tree we created */
  speciment * tmp = NULL;
  while (s != NULL){
    tmp = s;
    s = s -> next;
    tmp -> prev = NULL;
    free_present(tmp -> present_children);
    free(tmp -> prs);
    free(tmp);
  }
}

/* -------------------------------------- free end ------------------------------------------ */
void print_coords(){
  FILE * f1 = fopen("coordinates.txt", "w");
    preschild * pr = head -> present_children;
    while (pr != NULL){
      segment * seg = pr -> segments;
      while (seg != NULL){
        /* print rows first */
        fprintf(f1, "\n%d %u %u ", pr-> pid, seg -> start, seg -> end);
        coord * co = seg -> coords;
        while (co != NULL){
          fprintf(f1, "%u ", co -> r);
          co = co -> next;
        }
        /* print columns after */
        fprintf(f1, "\n%d %u %u ", pr-> pid, seg -> start, seg -> end);
        co = seg -> coords;
        while (co != NULL){
          fprintf(f1, "%u ", co -> c);
          co = co -> next;
        }
        seg = seg -> next;
      }
      pr = pr -> next;
    }
  fclose(f1);
}

void print_present_children(speciment * s){
	FILE * f1;
	f1 = fopen("present_children.txt", "a");
	fprintf(f1,"---------------------------\nchildren of %d	" , s -> prs -> pid);
	preschild * tmp = s -> present_children;
	if (tmp == NULL)
		fprintf(f1,"already a present child ");
	segment * seg;
	unsigned counter = 0;
	while (tmp){
		fprintf(f1, "child: %d, with segments:\t", tmp -> pid);
		seg = tmp -> segments;
		while (seg){
			fprintf(f1, "[%d %d] ", seg -> start, seg -> end);
			seg = seg -> next;
		}
		tmp = tmp -> next;
		++counter;
	}
	fprintf (f1, "--------------------\nchildren: %d\n", counter);
	fclose(f1);
}

void print_segment_representation(speciment *s){
	FILE * f1;
	f1 = fopen("segment_rep.txt", "a");
	preschild * tmp = s -> present_children;
	if (tmp == NULL)
		fprintf(f1,"1.00 ");
	segment * seg;
	double size = 0.0;
	while (tmp){
		seg = tmp -> segments;
		while (seg){
			size = (seg -> end - seg -> start +1)/(max_segment+1);
			fprintf(f1, "%lf ", size);
			seg = seg -> next;
		}
		tmp = tmp -> next;
	}
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
  fprintf(stderr, "%d\n", ben_pos);
	if (samples == 0)
		samples = DEFAULT_SAMPLES;
	currK = 2*samples;
	anc = calloc(currK, sizeof(struct ancestry));
	event_backwards = event_number;
	negative_generations = 0;
	if (ancs || boxs){
		samples = 0;
		print_boxes();
		assert(boxhd != NULL);
		boxed * tmp = NULL;
		while (boxhd != NULL){
			area_sampling(boxhd -> samples, boxhd -> srow, boxhd -> erow, boxhd -> scol, boxhd -> ecol, boxhd -> gen);
			tmp = boxhd;
			boxhd = boxhd -> next;
			free(tmp);
		}
	}
	else
		sampling(samples);
	total = samples;
	affect = malloc(sizeof(unsigned) * (samples +1));
	unsigned i;
	for (i = 0; i <= samples; ++i)
		affect[i] = 0;

  // fprintf(stderr, "\nSAMPLING DONE\n");
	go_back();
  // fprintf(stderr, "\nWENT BACK\n");
	assert(head != NULL);
	keepsake = People[0]; 	/* keep track of the people we have in generation zero */
	if ( prev_gen > 1 ){		  /* meaning we partially "failed" to trace the MRCA during the initial traceback */
    fprintf(stderr, "more gens\n");
    	further_back();
  }
	assert(head != NULL);
	add_present(head);
  print_coords();
	destruction();         /* not useful anymore so no need to keep all this memory allocated */
	free(affect);
	// fprintf (stderr, " \nREWIND COMPLETE\n");
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
ii) we deduce the path back for that node and follow it. We add the parent of that person to the queue. We keep an integer that show us how many people we have in the previous generation,
which we increase it's time we find a parent.
iii) if two people have the same parent, we merge the segments we took from each, thus keeping a single node for each unique parent.
iv) we repeat till all the current generations parents have been deduced.
v) we check for done and not - done segments. As long as we have segments that exist in multiple people we are not done.
*/
