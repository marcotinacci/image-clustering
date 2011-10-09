#include "siftcmp.h"

sim_metric* compute_matrix_dist(SIFTs** desc_array, const unsigned int nel){
	// inizializzazione della matrice
	sim_metric* dist = init_matrix_dist(nel);
	const int N = nel*(nel-1)/2;

	// partizionamento sull'output
	//#pragma omp parallel for num_threads(omp_get_num_threads()) schedule(dynamic)
	for(int i = 0; i < N; i++){
		dist[i] = sim_degree(desc_array[row_index(i,nel)],desc_array[column_index(i,nel)]);
	}	
	return dist;
}

sim_metric sim_degree(const SIFTs* d1, const SIFTs* d2){
	// contatore del grado di similarita'
	sim_metric counter = 0;
	// numero cicli esterni
	const int N1 = d1->nel;
	// numero cicli interni
	const int N2 = d2->nel;

	/*
		si e' scelto di parallelizzare il ciclo esterno in quanto la riduzione sulla
		somma del counter risulta piu' rapida della riduzione sul minimo, che richiede
		una sezione critica con controlli su primo e secondo minimo. La divisione del
		carico rimane la stessa ma con minore sezione critica
	*/
	/*
		scheduling dinamico in quanto le dimensioni dei descrittori sono variabili e
		l'accumularsi potrebbe portare a uno sbilanciamento del carico
	*/
	
	// cicla su d1
	#pragma omp parallel for reduction(+:counter) num_threads(omp_get_num_procs()) schedule(dynamic)
	for(int i = 0; i < N1; i++){
            // minimi
            float min1, min2;
            min1 = min2 = numeric_limits<float>::max();
            for(int j = 0; j < N2; j++){
                // confronta i sift i e j
                float dist = sift_dist(get_sift(d1,i),get_sift(d2,j));
                // aggiorna i minimi
                if(dist < min1){
                        min2 = min1;
                        min1 = dist;
                } else if(dist < min2){
                        min2 = dist;
                }
            }

            // se il rapporto è minore di alpha incrementa il contatore
            if(min1 / min2 < ALPHA){
                    counter++;
            }
	}
	return counter;
}

float sift_dist(const unsigned char* s1, const unsigned char* s2){
	// calcolo della distanza euclidea
	float dist = 0;
	// sommatoria
	for(int i = 0; i < SIFT_SIZE; i++){
		// differenza quadrata
		// controllo per overflow
		if(s1[i] < s2[i])
			dist += pow(s2[i] - s1[i],2);
		else
			dist += pow(s1[i] - s2[i],2);			
	}
	// radice quadrata
	return pow(dist,0.5);
}
