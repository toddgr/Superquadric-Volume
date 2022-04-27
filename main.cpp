/*
	Grace Todd
	CS 475: Intro to Parallel Programming
	Spring 2022

	PROJECT 2: Use parallel reduction to compute the volume of a superquadric using R=1.2. and N=2.5.
	Source code provided by Dr. Mike Bailey
*/


#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Print debugging messages
#ifndef DEBUG
#define DEBUG	false
#endif

					// In benchmark script:
int NUMT;			// Set the number of threads: 1, 2, and 4
int NUMNODES;		// Set the number of subdivisions: at least 8 different ones

// How many tries to discover the maximum performance:
#ifndef NUMTRIES
#define NUMTRIES	20
#endif

const float N = 2.5f;
const float R = 1.2f;

#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.

float Height(int, int);	// function prototype


int main(int argc, char* argv[]) {
#ifndef _OPENMP		// Check for OpenMP support
	fprintf(stderr, "No OpenMP support!\n");
	return 1;
#endif

	if (argc >= 2) {
		NUMT = atoi(argv[1]);
	}

	if (argc >= 3) {
		NUMNODES = atoi(argv[2]);
	}
	
	// Set the number of threads to use in parallelizing the for-loop:
	omp_set_num_threads(NUMT);
	double  maxPerformance = 0.;

	// Calculate the weighted heights certain number of times
	for (int times = 0; times < NUMTRIES; times++)
	{
		double time0 = omp_get_wtime();
		float fullTileArea = (((XMAX - XMIN) / (float)(NUMNODES - 1)) *		// The area of a single full-sized tile:
			((YMAX - YMIN) / (float)(NUMNODES - 1)));						// (not all tiles are full-sized, though)
		float halfTileArea = fullTileArea/2;
		float quartTileArea = fullTileArea/4;

		float volume = 0, area = 0;											// Initialize volume, area for each new trial
		
		// Sum up the weighted heights into the variable "volume"
		// using an OpenMP for-loop and a reduction:
#pragma omp parallel for default(none) shared(NUMNODES, area, fullTileArea, halfTileArea, quartTileArea, stderr) reduction(+:volume)
		for (int i = 0; i < NUMNODES * NUMNODES; i++)
		{
			int iu = i % NUMNODES;
			int iv = i / NUMNODES;
			float z = Height(iu, iv);				// A pin's contribution to the overall volume is z * A
													// Check the position of the node; If edge, corner, or middle node
			if (iu == 0 || iu == NUMNODES-1) {
				if (iv == 0 || iv == NUMNODES-1) {
					area = quartTileArea;
				}
				area = halfTileArea;
			}
			else if (iv == 0 || iv == NUMNODES-1) {
				area = halfTileArea;
			} 
			else {
				area = fullTileArea;
			}

			volume = volume + (z * area);			
													
													
		}											// As the superquadric has a negative half in
													// addition to a positive half, double the 0.-Height
		volume = volume * 2;						// volume as the final answer.		

		double time1 = omp_get_wtime();
		double megaHeightsPerSecond = ((double)NUMNODES * (double)NUMNODES) / (time1 - time0) / 1000000.;
		if (megaHeightsPerSecond > maxPerformance) {
			maxPerformance = megaHeightsPerSecond;
		}

		// Alternative print statement not for use in CSV files
		/*fprintf(stderr, "%2d threads : %8d subdivisions ; volume = %6.2lf ; megaheights/sec = %6.2lf\n",
			NUMT, NUMNODES, volume, maxPerformance);*/

		fprintf(stderr, "%2d, %8d, %6.2lf, %6.4lf\n", NUMT, NUMNODES, volume, maxPerformance);
	}
}


// Auxillary function to calculate the heights of the nodes
float Height (int iu, int iv) {	// iu,iv = 0 .. NUMNODES-1
	float x = -1. + 2. * (float)iu / (float)(NUMNODES - 1);	// -1. to +1.
	float y = -1. + 2. * (float)iv / (float)(NUMNODES - 1);	// -1. to +1.

	float xn = pow(fabs(x), (double)N);
	float yn = pow(fabs(y), (double)N);
	float rn = pow(fabs(R), (double)N);
	float r = rn - xn - yn;
	if (r <= 0.)
		return 0.;
	float height = pow(r, 1. / (double)N);
	return height;
}
