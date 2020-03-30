/* extra from spot.c that I never use but don't want to delete */


void area_config(unsigned i, unsigned j){
	double counter = 0;
	double cap = 0;
	double mig = 0;
	double frc = 0;
	double grt = 0;
	//if (i == 0 && j == 0){ /* top left corner of the map */
		A[0][i][j] -> capacity = 50;
		A[0][i][j] -> growth_rate = 0.3;
		A[0][i][j] -> friction = 0.5;
		A[0][i][j] -> migrate = 0.5;
		return;
	//}

	if (i > 0){ /* meaning we ain't at the first row, so we can check above */
			if (j > 0){ /* ain't in the first column either, so we can check left */
			  cap = A[0][i - 1][j - 1] -> capacity;
				frc = A[0][i - 1][j - 1] -> friction;
				grt = A[0][i - 1][j - 1] -> growth_rate;
				mig = A[0][i - 1][j - 1] -> migrate;
				++counter;
			}
			cap += A[0][i -1][j] -> capacity;
			frc += A[0][i - 1][j] -> friction;
			grt += A[0][i - 1][j] -> growth_rate;
			mig += A[0][i - 1][j] -> migrate;
			++counter;
			if (j < rows  - 1 ){ /* ain't in the last column, so we can check right */
				cap += A[0][i - 1][j + 1] -> capacity;
				frc += A[0][i - 1][j + 1] -> friction;
				grt += A[0][i - 1][j + 1] -> growth_rate;
				mig += A[0][i - 1][j + 1] -> migrate;
				++counter;
			}
	}

	if (j > 0){
		cap += A[0][i ][j - 1] -> capacity;
		frc += A[0][i][j - 1] -> friction;
		grt += A[0][i][j - 1] -> growth_rate;
		mig += A[0][i][j - 1] -> migrate;
		++counter;
	}
	cap = cap/counter;
	frc = frc/counter;
	grt = grt/counter;
	mig = mig/counter;

	double tmp = -1.0; 	/* the minimum capacity is 1, to allow potential migrants to go through a cell */
	while(tmp < 1 )
	  tmp = gsl_ran_gaussian(r, sigma) + cap + 0.5;		/* the 0.5 added is in order for proper rounding during the unsigned typecast to happen */
	A[0][i][j] -> capacity = (unsigned)tmp;
	tmp = -1.0;
	while (tmp <= 0 && tmp > 1)
		tmp = gsl_ran_gaussian(r, sigma) + frc + 0.5;
	A[0][i][j] -> friction = (unsigned)tmp;
	tmp = -1.0;
	while (tmp < 0 || tmp > 0.3 )
		tmp = gsl_ran_gaussian(r, sigma) + grt;
	A[0][i][j] -> growth_rate = tmp;
	tmp = -1.0;
	while (tmp < 0 || tmp > 1)
		tmp = gsl_ran_gaussian(r, sigma) + mig;
	A[0][i][j] -> migrate = tmp;

}
