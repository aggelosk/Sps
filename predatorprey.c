unsigned long int lotka_volterra(predator_type * type, predators * predator, preys * prey){
	assert ( type != "undef");
	unsigned long int result;
	double kills = 0;
	double deaths = 0;
	if (!type == "prey") {
		predators * pd = predator;
		while (pd)
			kills = kills + pd -> current -> population * ratio;
	}
	if (!type == "predator"){
		preys * py = prey;
		while (py)
			deaths = deaths + py -> current -> population * ratio;
	}
	result = logistic_function(hybrid) + kills - deaths;
	return result;
}

// κάτι με χαλάει αρκετά 

unsigned long int lotka_volterra_prey(unsigned long int prey, unsigned long int predator, double kda){
	unsigned long int result;
	result = logistic_function(prey) - kda * predator; /* kda is the ratio of kills for each predator */
	return result;
}

unsigned long int lotka_volterra_predator(unsigned long int prey, unsigned long int predator, double kda){
	unsigned long int result;
	result = logistic_function(predator) + kda * prey;
	return result;
}

unsigned long int lotka_volterra_hybrid(unsigned long int hybrid, unsigned long int prey, unsigned long int predator, double prey_kda, double predator_kda){
	unsigned long int result;
	result = logistic_function(hybrid) + prey_kda * prey - predator_kda * predator;
	return result;
}

/* ------------------------------------------------------------------------------------ */

species * add_species(char * name, predator_type type){
	species * tmp = malloc(sizeof(species));
	tmp -> name = name;
	tmp -> type = type;
	tmp -> dies = NULL;
	tmp -> kills = NULL;
	tmp -> comp = NULL;
	tmp -> pop_num = -1;
	tmp -> spec_capacity = 0;
	tmp -> population = 0;
	tmp -> fit_people = 0;
	tmp -> people = NULL;
	tmp -> incoming = 0;
	tmp -> immigrants = NULL;
	tmp -> next = spec_list;
	spec_list = tmp;
	return tmp;
}

void add_predator(species * prey, species * predator, double ratio){
	/* adding a new predator for the prey */
	predators * prd;
	prd = malloc(sizeof(predators));
	prd -> current = predator;
	prd -> ratio = ratio;
	prd -> next = prey -> die;
	prey -> die = prd;
	/* adding a new prey for the predator */
	preys * pry;
	pry = malloc(sizeof(preys));
	pry -> current = prey}
	pry -> ratio = ratio;
	pry -> next = predator -> kill;
	predator -> kill = pry;
}
