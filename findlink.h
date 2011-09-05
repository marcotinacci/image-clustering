#ifndef __FINDLINK_H__
#define __FINDLINK_H__

#include <math.h>
#include <algorithm>
#include <omp.h>
#include "file_desc.h"
#include "distance_matrix.h"

using namespace std;

/*
 * trova la coppia di cluster da fondere (distanza massima)
 * in una matrice triangolare superiore
 * dist: matrice delle distanze
 * mask: maschera dei clusters attivi
 * nel: dimensione della matrice
 * c1, c2: coppia di clusters da fondere (c1 indice minimo)
 */
void findlink_tri(const sim_metric* dist, const int mask, const unsigned int nel, 
	unsigned int* c1, unsigned int* c2, sim_metric* p_max);

/*
 * trova la coppia di cluster da fondere (distanza massima)
 * in una matrice quadrata
 * dist: matrice delle distanze
 * mask: maschera dei clusters attivi
 * nel: dimensione della matrice
 * c1, c2: coppia di clusters da fondere (c1 indice minimo)
 */
void findlink_quad(const sim_metric* dist, const int mask1, const unsigned int rows, 
	const int mask2, const unsigned int cols, unsigned int* c1, unsigned int* c2, 
	sim_metric* p_max);

#endif
