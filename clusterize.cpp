#include "clusterize.h"

void clusterize(cluster* clusters, sim_metric* dist, int* mask, unsigned int nel){
	// alloca la lista di cluster, ogni cluster è una lista di indici
	for(int i=0; i < (int)nel; i++){
		clusters->insert(pair<unsigned int, list<unsigned int> > (i,list<unsigned int>(1,i)));
	}

	// crea la maschera di elementi attivi, 1 attivo, 0 spento
	*mask = 0xFFFFFFFF;
	
	/*
		TODO dividere la procedura con mpi
	*/
	// fermati quando hai trovato il numero di cluster definito
	for(unsigned int k=0; k < nel -NUM_CLUSTER; k++){
		unsigned int c1, c2;
		findlink_tri(dist, *mask, nel, &c1, &c2, 0);
		merge(clusters, dist, mask, nel, c1, c2);
	}
}
