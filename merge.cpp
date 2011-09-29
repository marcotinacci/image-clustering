#include "merge.h"

void merge(cluster* clusters, sim_metric* dist, int* mask, const unsigned int nel, 
	const unsigned int c1, const unsigned int c2)
{		
	#pragma omp parallel num_threads(omp_get_num_procs())
	{
		#pragma omp single nowait
		{	
			merge_clusters(clusters, c1, c2);
		}
		merge_dist(dist, mask, nel, c1, c2);
	}
	// disattiva c2 sulla maschera
	*mask = *mask & ~(0x01 << c2);
}

void merge_dist(sim_metric* dist, int* mask, const unsigned int nel, const unsigned int c1, const unsigned int c2){
	#pragma omp for schedule(dynamic) nowait
	for(int i = 0; i < (int)nel; i++){
		// esegui l'aggiornamento solo se gli elementi sono attivi
		if(!check_mask_element(*mask, i) || (int)c2 == i) continue;
		// aggiornamento della colonna 'c1'
		sim_metric a,b;
		a = get(dist,nel,i,c1);
		b = get(dist,nel,i,c2);
		if(a > b){
			set(dist,nel,i,c1,b);
		}
	}
}

void merge_clusters(cluster* clusters, const unsigned int c1, const unsigned int c2){
        printf("\n MERGE: %d,%d\n",c1,c2);
	// aggiorna i cluster
	list<unsigned int> *a_list = &(*clusters)[c2];
	list<unsigned int> *b_list = &(*clusters)[c1];
	// aggiungi alla lista a gli elementi della lista b
	for(list<unsigned int>::iterator it = b_list->begin(); it != b_list->end(); it++){
		a_list->push_back(*it);		
	}
	// elimina la lista b dai clusters
	clusters->erase(c1);
}
