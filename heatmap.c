/* This code is for test purposes. It is used for printing the population of every area in 3 different generations. Since it won't be used in the
main program, it is hard-coded, and it might require changes if someone wants to test it */

#include <stdio.h>
#include <stdlib.h>

#include "spot.h"

#define SIZE 50

extern spot **** A;
extern unsigned event_number;

int Heatmap0[50][50]; 	/* first */
int Heatmap1[50][50];		/* intermediate */
int Heatmap2[50][50];		/* final */

void heatmap(){
	int x, y;
	for(x = 0; x < 50; x++){
		for(y = 0; y < 50; y++){
			Heatmap0[x][y] = A[0][x][y] -> population;
			Heatmap1[x][y] = A[5][x][y] -> population;
			Heatmap2[x][y] = A[event_number][x][y] -> population;
		}
	}
	mapheat();
}

void mapheat(){
	FILE  * f1, * f2, * f3;
	f1 = fopen("f1.txt", "w");
	f2 = fopen("f2.txt", "w");
	f3 = fopen("f3.txt", "w");
	int x, y;
	for ( x = 0; x< 50; x++){
		for ( y = 0; y < 50; y++){
			fprintf(f1, "%d ", Heatmap0[x][y]);
			fprintf(f2, "%d ", Heatmap1[x][y]);
			fprintf(f3, "%d ", Heatmap2[x][y]);
		}
		fprintf(f1,"\n");
		fprintf(f2,"\n");
		fprintf(f3,"\n");
	}
	fclose(f1);
	fclose(f2);
	fclose(f3);
}