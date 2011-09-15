#ifndef MASK_H_
#define MASK_H_

/*
 * costruttore maschera
 */
int* init_mask(const int np);

/*
 * restituisce la porzione di maschera della sotto matrice triangolare
 * specificata dal rango
 */
int get_mask_part_tri(const int *mask, const int rank);

/*
 * restituisce la porzione di maschera (colonna) della sotto-matrice quadrata
 * specificata dal rango
 */
int get_mask_col_quad(const int *mask, const int rank, const int index_part,
		const int np);

/*
 * restituisce la porzione di maschera (riga) della sotto-matrice quadrata
 * specificata dal rango
 */
int get_mask_row_quad(const int *mask, const int rank, const int index_part,
		const int np);

/*
 * aggiorna la il vettore maschera disattivando gli indici dei due cluster fusi
 * mask: vettore maschera
 * rank: rango terminale che possiede l'elemento comune dei due cluster fusi
 * index_part: indice della parte del terminale che contiene l'elemento comune
 * local_row: indice riga locale
 * local_col: indice colonna locale
 * np: numero terminali
 */
void update_mask(int *mask, const int rank, const int index_part,
		const int local_row, const int local_col, const int np);

#endif /* MASK_H_ */
