#ifndef __CLUSTERIZE_H__
#define __CLUSTERIZE_H__

#include <map>
#include <list>
#include "distance_matrix.h"
#include "file_desc.h"
#include "findlink.h"
#include "merge.h"
#include "cluster.h"

#define DEBUG
#define NUM_CLUSTER 2

/*
 * metodo di clustering agglomerativo
 * clusters: mappa di liste di indici a immagini
 * dist: matrice triangolare "stesa" delle distanze
 * nel: dimensione matrice
 */
void clusterize(cluster* clusters, sim_metric* dist, int* mask, unsigned int nel);

#endif