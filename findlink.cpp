#include "findlink.h"

void findlink_tri(const sim_metric* dist, const int mask, const unsigned int nel, 
		unsigned int* c1, unsigned int* c2, sim_metric* p_max)
{
        //printf("\nfindlink_tri\n");
	unsigned int N = nel*(nel-1)/2;
	// massimo globale
	sim_metric sh_max = 0;
	unsigned int sh_max_i = 0;
	#pragma omp parallel num_threads(omp_get_num_procs())
	{
		// massimo locale
		sim_metric max = 0;
		unsigned int max_i = 0;
		#pragma omp for nowait schedule(static)
		for(int i=0; i<(int)N; i++){
			// se il bit della maschera e' 1 e la distanza e' massima
			if( check_mask_element(mask, column_index(i,nel)) && 
					check_mask_element(mask, row_index(i,nel)) && dist[i] > max)
			{
                            //printf("\n%d > %d\n",dist[i],max);
				// aggiorna massimo e indice
				max = dist[i];
				max_i = i;
			}
		}
		// max reduction
		#pragma omp critical
		{
			if(max > sh_max){
				sh_max = max;
				sh_max_i = max_i;
			} 
		}
	}

	// inserisci i valori nei parametri per riferimento
	*c1 = row_index(sh_max_i,nel);
	*c2 = column_index(sh_max_i,nel);
	*p_max = sh_max;
	// mantieni in c1 il massimo
	if(*c1 > *c2) swap(c1,c2);
        
        printf("\n<c1,c2,max,mask> : (%d,%d,%d,%X)\n",*c1,*c2,*p_max,mask);
}

void findlink_quad(const sim_metric* dist, const int mask1, const unsigned int rows, 
		const int mask2, const unsigned int cols, unsigned int* c1, unsigned int* c2, sim_metric* p_max)
{
        //printf("\nfindlink_quad\n");
	unsigned int N = rows*cols;
	// massimo globale
	sim_metric sh_max = 0;
	unsigned int sh_max_i = 0;
	#pragma omp parallel num_threads(omp_get_num_procs())
	{
		// massimo locale
		sim_metric max = 0;
		unsigned int max_i = 0;
		#pragma omp for nowait schedule(static)
		for(int i=0; i<(int)N; i++){
			// se il bit della maschera e' 1 e la distanza e' massima
			if( check_mask_element(mask1, i / cols) && 
                                check_mask_element(mask2, i % cols) && 
                                        dist[i] > max)
			{
                            //printf("\n%d > %d\n", dist[i], max);
				// aggiorna massimo e indice
				max = dist[i];
				max_i = i;
			}
		}
		// max reduction
		#pragma omp critical
		{
			if(max > sh_max){
				sh_max = max;
				sh_max_i = max_i;
			} 
		}
	}
	// inserisci i valori nei parametri per riferimento
	*c1 = sh_max_i / cols;
	*c2 = sh_max_i % cols;
	*p_max = sh_max;
        
        printf("\n<c1,c2,max,mask1,mask2> : (%d,%d,%d,%X,%X)\n",*c1,*c2,*p_max,mask1,mask2);
}
