#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include <map>
#include <list>
#include <iostream>

using namespace std;

// ----------------------------------------------------------------
// STRUTTURE
// ----------------------------------------------------------------

/*
 * definizione della struttura che memorizza i cluster
 */
typedef map<unsigned int, list<unsigned int> > cluster;

// ----------------------------------------------------------------
// PROTOTIPI
// ----------------------------------------------------------------

/*
 * costruttore dei cluster
 */
cluster* init_cluster();

/*
 * distruttore dei cluster
 */
void destroy_cluster(cluster* clusters);

/*
 * metodo di stampa dei cluster
 */
void print_clusters(cluster* clusters);

#endif