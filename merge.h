#ifndef __MERGE_H__
#define __MERGE_H__

#include <map>
#include <list>
#include <omp.h>
#include <stdio.h>

#include "file_desc.h"
#include "distance_matrix.h"
#include "cluster.h"

using namespace std;

/*
 * fusione dei due clusters
 * clusters: insieme di liste di indici che identificano immangini
 * dist: matrice delle distanze
 * mask: maschera degli elementi attivi
 * nel: dimensione matrice
 * c1, c2: indici dei clusters da fondere
 */
void merge(cluster* clusters, sim_metric* dist, int* mask, const unsigned int nel, 
	const unsigned int c1, const unsigned int c2);

/*
 * fusione delle distanze della matrice
 * dist: matrice delle distanze
 * mask: maschera degli elementi attivi
 * nel: dimensione matrice
 * c1, c2: indici dei clusters da fondere
 */
void merge_dist(sim_metric* dist, int* mask, const unsigned int nel, const unsigned int c1, const unsigned int c2);

/*
 * fusione delle liste dei clusters
 * clusters: insieme di liste di indici che identificano immangini
 * c1, c2: indici dei clusters da fondere
 */
void merge_clusters(cluster* clusters, const unsigned int c1, const unsigned int c2);

#endif