#ifndef __SIFTCMP_H__
#define __SIFTCMP_H__

#include <math.h>
#include <limits>
#include <omp.h>
#include "file_desc.h"
#include "distance_matrix.h"

#define ALPHA 0.7

/*
	FIX il parametro desc_array deve essere const (error?)
*/

/*
 * metodo che calcola la matrice delle distanze
 * desc: vettore di puntatori a descrittori
 * nel: dimensione matrice
 * return: matrice triangolare "stesa" delle distanze
 */
sim_metric* compute_matrix_dist(SIFTs** desc_array, const unsigned int nel);

/*
 * metodo che determina il numero di matching sift tra le due immagini
 * d1, d2: descrittori da confrontare
 * return: grado di similarità
 */
sim_metric sim_degree(const SIFTs* d1, const SIFTs* d2);

/*
 * metodo che calcola la distanza euclidea tra due sift
 * s1, s2: sift
 * return: distanza tra s1 e s2
 */
float sift_dist(const unsigned char* s1, const unsigned char* s2);

#endif