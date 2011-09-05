#include "distance_matrix.h"

sim_metric* init_matrix_dist(const unsigned int nel){
	if(nel < 2) return NULL;
	return new sim_metric[nel*(nel-1)/2];
}

void destroy_matrix_dist(sim_metric* dist){
	delete [] dist;
}

void set(sim_metric* dist, const unsigned int nel, const unsigned int row, 
		const unsigned int col, const sim_metric val)
{
	if(col != row){
		if(row > col){
			dist[(nel-2)*col - (col-1)*col/2 + row -1] = val;
		}else{
			dist[(nel-2)*row - (row-1)*row/2 + col -1] = val;
		} 
	}else{
		cerr << "set su elemento diagonale " << col << endl;
	}
}

sim_metric get(const sim_metric* dist, const unsigned int nel, 
		const unsigned int row, const unsigned int col)
{
	// gli elementi diagonali sono sempre 0 nella matrice delle distanze
	if(row == col) return UINT_MAX;
	if(row > col){
		return dist[(nel-2)*col - (col-1)*col/2 + row -1];		
	}else{
		return dist[(nel-2)*row - (row-1)*row/2 + col -1];
	}
}

unsigned int row_index(const unsigned int i, const unsigned int nel){
	double n = nel-1;
   double row = (-2*n - 1 + sqrt( (4*n*(n+1) - 8*(double)i - 7) )) / -2;
   if( row == (double)(int) row ) row -= 1;
	return (unsigned int) row;
}

unsigned int column_index(const unsigned int i, const unsigned int nel){
   unsigned int row = row_index(i, nel);
   return (i - (nel-1) * row + row*(row+1) / 2)+1;
}

bool check_mask_element(const int mask, const unsigned int index){
	return (mask >> index) & 0x01;
}

void print_matrix_index(const sim_metric* dist, const unsigned int nel){
	unsigned int n = nel*(nel-1)/2;
	cout << "stampa da vettore" << endl;
	for(unsigned int i=0; i<n; i++){
		cout << "d[" << row_index(i,nel) << "][" << column_index(i,nel) << "] = " << dist[i] << endl;
	}
	cout << "stampa da matrice" << endl;
	for(unsigned int i=0; i<nel; i++){
		for(unsigned int j=0; j<nel; j++){
			if(i<j){
				cout << "d[" << i << "][" << j << "]: " << get(dist,nel,i,j) << endl;
			}
		}
	}
}

void print_mask(const int mask, const unsigned int nel){
	cout << "mask: " << endl << "|";
	for(unsigned int i=0; i<nel; i++){
		cout << ((mask >> i) & 0x01) << "|";
	}
	cout << endl;
}

void print_matrix(const sim_metric* dist, const unsigned int nel){
	unsigned int k = 0;
	for(unsigned int i=0; i<nel; i++){
		for(unsigned int j=0; j<nel; j++){
			if(i<j){
				cout << dist[k] << ' ';
				k++;
			}else{
				cout << "_ ";
			}
		}
		cout << endl;
	}
}
