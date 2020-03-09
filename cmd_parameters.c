/* Allows the user to define certain parameters of the program. If a parameter is not set, we stick with the default values */

#include "cmd_parameters.h"

/* from spot.c */
extern unsigned seed;

/* from create_map.c */
extern char input;
extern unsigned change_event;

extern unsigned migration_rate;
extern unsigned friction;
extern unsigned growth;
extern unsigned capacity;

extern topology * head_m;
extern topology * head_f;
extern topology * head_g;
extern topology * head_c;

extern int xaxis;
extern init_pop * pop_h;


/* from forward_time.c */
extern char single_parent;
extern char flock;

extern unsigned steps;
extern unsigned rows;
extern unsigned columns;
extern double fitness;
extern double ben_chance;
extern char beneficial;

extern unsigned cram;
extern ram * ram_h;

/* from rewind_time.c */
extern unsigned samples;
extern boxed * boxhd;
extern char boxs;
extern char ancs;
extern double rec_rate;
extern unsigned max_segment;

/* parameters for Poisson and Gaussian */
extern double sigma;
extern double theta;

ram * tail = NULL;
boxed * boxt = NULL;
topology * tail_m = NULL;
topology * tail_f = NULL;
topology * tail_g = NULL;
topology * tail_c = NULL;

char * filename;
unsigned gen;


void add_migr_top(unsigned gen, char * filename){
	topology * tmp = malloc(sizeof(topology));
	tmp -> gen = gen;
	tmp -> filename = strdup(filename);
	tmp -> next = NULL;
	if (head_m == NULL){
		head_m = tmp;
		tail_m = head_m;
		return;
	}
	tail_m -> next = tmp;
	tail_m = tail_m -> next;
}

void add_fric_top(unsigned gen, char * filename){
	topology * tmp = malloc(sizeof(topology));
	tmp -> gen = gen;
	tmp -> filename = strdup(filename);
	tmp -> next = NULL;
	if (head_f == NULL){
		head_f = tmp;
		tail_f = head_f;
		return;
	}
	tail_f -> next = tmp;
	tail_f = tail_f -> next;
}

void add_grth_top(unsigned gen, char * filename){
	topology * tmp = malloc(sizeof(topology));
	tmp -> gen = gen;
	tmp -> filename = strdup(filename);
	tmp -> next = NULL;
	if (head_g == NULL){
		head_g = tmp;
		tail_g = head_g;
		return;
	}
	tail_g -> next = tmp;
	tail_g = tail_g -> next;
}

void add_cpct_top(unsigned gen, char * filename){
	topology * tmp = malloc(sizeof(topology));
	tmp -> gen = gen;
	tmp -> filename = strdup(filename);
	tmp -> next = NULL;
	if (head_c == NULL){
		head_c = tmp;
		tail_c = head_c;
		return;
	}
	tail_c -> next = tmp;
	tail_c = tail_c -> next;
}

void add_pop(unsigned i, unsigned j, unsigned p){
	init_pop * tmp = malloc(sizeof(init_pop));
	tmp -> lat = i;
	tmp -> lon = j;
	tmp -> ppl = p;
	tmp -> next = pop_h;
	pop_h = tmp;
}

void add_ram(unsigned cr){
	ram * tmp = malloc(sizeof(ram));
	tmp -> cram = cr;
	tmp -> next = NULL;
	if (ram_h == NULL){
		cram = cr;
		ram_h = tmp;
		tail = ram_h;
		return;
	}
	tail -> next = tmp;
	tail = tail -> next;
}

void add_box(boxed * b){
	b -> next = NULL;
	if (boxhd == NULL){
		boxhd = b;
		boxt = b;
		return;
	}
	boxt -> next = b;
	boxt = boxt -> next;
}





void print_help(){ /* ancestral sampling needs to be added */

    printf("\n\n ------------ Command Line Parameters ------------ \n\n");

    printf("General purpose commands:\n\n");

    printf("\t-flok: determines whether the migration pattern will be individual based or as a mass migration.\n\n");
    printf("\t-sngp: determines whether we allow children to be single parented.\n\n");

    printf("\t-seed: user-defined seed for the random number generator function.\n\n");

    /* ----- from create_map ----- */
    printf("Used for creating the map:\n\n");
    printf("\t-gens: determines the number of generations the program will simulate. Paremeter value must be a positive integer.\n\n");
    printf("\t-rows: determines the number of rows the discrete map/array has. Paremeter value must be a positive integer.\n\n");
    printf("\t-cols: determines the number of columns the discrete map/array has. Paremeter value must be a positive integer.\n\n");
	printf("\t---\n\n");
    printf("\t-cpct: receives a file as an input. Sets the capacity for each area of the map. Must be of the same dimension as the map.\n\n");
    printf("\t-grth: receives a file as an input. Sets the growth rate for each area of the map. Must be of the same dimension as the map.\n\n");
    printf("\t-migr: receives a file as an input. Sets the migration rate for each area of the map. Must be of the same dimension as the map.\n\n");
    printf("\t-fric: receives a file as an input. Sets the friction rate for each area of the map. Must be of the same dimension as the map.\n\n");
	printf("\t---\n\n");
	printf("\t-cpcta: Sets the capacity for all areas of the map. Paremeter value must be a positive integer.\n\n");
    printf("\t-grtha: Sets the growth rate for all areas of the map. Parameter value from 0 to 1.\n\n");
    printf("\t-migra: Sets the migration rate for all areas of the map. Parameter value from 0 to 1.\n\n");
    printf("\t-frica: Sets the friction rate for every area of the map. Parameter value from 0 to 1.\n\n");

    printf("\t-strt: determines the area for initial population and the people that population will have. It can be given multiple times as input\n\n");

    /* ----- from forward time ----- */
	printf("Used for the forward in time process:\n\n");
	printf("\t-bene: allows for a beneficial mutation to be placed randomly starting from a certain generation.\n\n");
	printf("\t-cram: at the given generation removes from the genealogy tree those individuals that have no descendants in the given generation. Paremeter value must be a positive integer.\n\n");

	/* ----- from rewind time ----- */
	printf("Used for the backward in time process:\n\n");
	printf("\t-smps: determines the number of the samples to be considered. If the samples exceed the number of people in the generation a message is given and the process is terminated. Paremeter value must be a positive integer.\n\n");
	printf("\t-rcmb: determines the recombination rate. Parameter value from 0 to 1.\n\n");
	printf("\t-maxs: determines the size of the segment considered in this process. Paremeter value must be a positive integer.\n\n");
	printf("\t-boxs: sets the region from which samples will be taken from. Takes 5 values: \n\t\t -A set of rows and columns to determine the starting area for the sampling.\n\t\t -A similar set for the area where we stop sampling.\n\t\t -number of samples.\n\t\tIt can be given multiple times as input. \n\n");

	/* ----- from mutation ----- */
	printf("Used for applying the mutations:\n\n");
	printf("\t-psnd: determines the parameter for the Poisson distribution used to determine the number of mutation.Parameter value from 0 to 1.\n\n");

}

void cmd_params(int argc, char** argv){
	int i;
	for (i = 1; i < argv[i]; i++){

		if ( (!strcmp(argv[i], "-flok" ) ) ){
			flock = 1;
			continue;
		}

		if ( (!strcmp(argv[i], "-sngp" ) ) ){
			single_parent = atoi(argv[++i]);
			continue;
		}

		/* ---------- create_map ---------- */

		if ( (!strcmp(argv[i], "-gens" ) ) ){
			steps = atoi(argv[++i]);
			continue;
		}
		if ( (!strcmp(argv[i], "-rows" ) ) ){
			rows = atoi(argv[++i]);
			continue;
		}
		if ( (!strcmp(argv[i], "-cols" ) ) ){
			columns = atoi(argv[++i]);
			continue;
		}

		/* area for initial population */
		if( (!strcmp(argv[i], "-strt" ) ) ){
			xaxis = 1;
			unsigned x = atoi(argv[++i]);
			unsigned y = atoi(argv[++i]);
			unsigned p = atoi(argv[++i]);
			assert(x < rows && y < columns);
			add_pop(x, y, p);
			continue;
		}

		/* sets the migration rate of each area according to the given file(s) */
		if ( (!strcmp(argv[i], "-migr" ) ) ){
			input = 1;
			filename = malloc(sizeof(100));
			filename = strdup(argv[++i]);
			gen = atoi(argv[++i]);
			add_migr_top(gen, filename);
			continue;
		}

		/* sets the friction of each area according to the given file(s) */
		if ( (!strcmp(argv[i], "-fric" ) ) ){
			input = 1;
			filename = malloc(sizeof(100));
			filename = strdup(argv[++i]);
			gen = atoi(argv[++i]);
			add_fric_top(gen, filename);
			continue;
		}

		/* sets the growth rate of each area according to the given file(s) */
		if ( (!strcmp(argv[i], "-grth" ) ) ){
			input = 1;
			filename = malloc(sizeof(100));
			filename = strdup(argv[++i]);
			gen = atoi(argv[++i]);
			add_grth_top(gen, filename);
			continue;
		}

		/* sets the capacity of each area according to the given file(s) */
		if ( (!strcmp(argv[i], "-cpct" ) ) ){
			input = 1;
			filename = malloc(sizeof(100));
			filename = strdup(argv[++i]);
			gen = atoi(argv[++i]);
			add_cpct_top(gen, filename);
			continue;
		}
		/* checks whether the user wishes to define the migration of every deme */
		if ( (!strcmp(argv[i], "-migra" ) ) ){
			migration_rate = atof(argv[++i]);
			continue;
		}

		/* checks whether the user wishes to define the friction of every deme */
		if ( (!strcmp(argv[i], "-frica" ) ) ){
			friction = atof(argv[++i]);
			continue;
		}

		/* checks whether the user wishes to define the growth rate of every deme */
		if ( (!strcmp(argv[i], "-grtha" ) ) ){
			growth = atof(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-cpcta" ) ) ){
			capacity = atoi(argv[++i]);
			continue;
		}


		/* --------- forward_time --------- */

		if ( (!strcmp(argv[i], "-bene" ) ) ){
			beneficial = 1;
			ben_chance = atoi(argv[++i]);
			fitness = atof(argv[++i]);
		}

		if ( (!strcmp(argv[i], "-cram" ) ) ){
			add_ram(atoi(argv[++i]));
			continue;
		}

		/* --------- rewind_time --------- */

		/* number of samples */
		if ( (!strcmp(argv[i], "-smps" ) ) ){
			samples = atoi(argv[++i]);
			continue;
		}

		/* recombination rate */
		if ( (!strcmp(argv[i], "-rcmb" ) ) ){
			rec_rate = atof(argv[++i]);
			continue;
		}

		/* size of gene */
		if ( (!strcmp(argv[i], "-maxs" ) ) ){
			max_segment = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-boxs" ) ) ){
			boxs = 1;
			boxed * tmp = malloc(sizeof(boxed));
			tmp -> samples = atoi(argv[++i]);
			tmp -> srow = atoi(argv[++i]);
			tmp -> scol = atoi(argv[++i]);
			tmp -> erow = atoi(argv[++i]);
			tmp -> ecol = atoi(argv[++i]);
			tmp -> gen = 0;
			tmp -> next = NULL;
			add_box(tmp);
			continue;
		}

		if ( (!strcmp(argv[i], "-ancs" ) ) ){ /* takes samples from certain areas at a past point in time */
			ancs = 1;
			boxed * tmp = malloc(sizeof(boxed));
			tmp -> samples = atoi(argv[++i]);
			tmp -> srow = atoi(argv[++i]);
			tmp -> scol = atoi(argv[++i]);
			tmp -> erow = atoi(argv[++i]);
			tmp -> ecol = atoi(argv[++i]);
			tmp -> gen = atoi(argv[++i]);
			tmp -> next = NULL;
			add_box(tmp);
			continue;
		}

		/* ---------- mutation ----------- */

		/* poisson parameter */
		if ( (!strcmp(argv[i], "-psnd" ) ) ){
			theta = atof(argv[++i]);
			continue;
		}

		/* --------- general use ---------- */

		/* seed for random */
		if ( (!strcmp(argv[i], "-seed" ) ) ){
			seed = atoi(argv[++i]);
			continue;
		}
		/* gaussian parameter */
		if ( (!strcmp(argv[i], "-gsnd" ) ) ){
			sigma = atof(argv[++i]);
			continue;
		}

		/* ----- help ----- */
		if ( (!strcmp(argv[i], "-help" ) ) ){
			print_help();
			exit(0);
		}

		fprintf(stderr, "Argument %s is invalid\n\n\n", argv[i]);
	}

}
