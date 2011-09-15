#include "mask.h"

int * init_mask(const int np){
	int *mask = new int[np];
	for(int i = 0; i < np; i++){
		mask[i] = 0xFFFFFFFF;
	}
	return mask;
}

int get_mask_part_tri(const int *mask, const int rank){
	return mask[rank];
}

int get_mask_col_quad(const int *mask, const int rank, const int index_part,
		const int np){
	int index = rank + index_part;
	if(index < np){
		return mask[index];
	}else{
		return mask[rank];
	}
}

int get_mask_row_quad(const int *mask, const int rank, const int index_part,
		const int np){
	int index = rank + index_part;
	if(index < np){
		return mask[rank];
	}else{
		return mask[index - np];
	}
}

void update_mask(int *mask, const int rank, const int index_part,
		const int local_row, const int local_col, const int np){
	int idx_col, idx_row, index;
	index = rank + index_part;
	if(index < np){
		idx_row = index;
		idx_col = rank;
	}else{
		idx_row = rank;
		idx_col = index-np;
	}
	// disattiva gli indici sulla maschera
	mask[idx_row] &= ~(0x01 << local_col);
	mask[idx_col] &= ~(0x01 << local_row);
}


