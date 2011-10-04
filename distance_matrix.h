#ifndef __DISTANCE_MATRIX_H__
#define __DISTANCE_MATRIX_H__

#include <math.h>
#include <limits.h>
#include <iostream>
#include "file_desc.h"

//#define DEBUG

using namespace std;

// ----------------------------------------------------------------
// STRUTTURE
// ----------------------------------------------------------------

/*
 * metrica di misura della similarita'
 */
typedef int sim_metric;

// ----------------------------------------------------------------
// PROTOTIPI
// ----------------------------------------------------------------
/*
	TODO inserire i const nelle firme dei metodi
*/

/*
 * inizializza la matrice triangolare delle distanze 
 * (in forma di array "steso")
 * nel: dimensione della matrice (numero immagini)
 * return: puntatore al vettore rappresentante la matrice delle 
 * distanze
 */
sim_metric* init_matrix_dist(const unsigned int nel);

/*
 * delloca lo spazio della matrice delle distanze
 * dist: matrice delle distanze
 */
void destroy_matrix_dist(sim_metric* dist);

/*
 * dealloca lo spazio della matrice delle parti, ogni elemento è una 
 * sottomatrice
 * parts: vettore di matrici (parts[0] e' triangolare, le successive sono 
 *      rettangolari)
 * nel_parts: numero elementi parts
 * 
 */
void destroy_matrix_parts(sim_metric** parts, int nel_parts);

/*
 * indice di riga di una matrice triangolare non diagonale 
 * memorizzata per righe
 * i: indice vettore
 * nel: dimensione matrice
 * return: corrispondente indice di riga nella matrice
 */
unsigned int row_index(const unsigned int i, const unsigned int nel);

/*
 * indice di colonna di una matrice triangolare non diagonale 
 * memorizzata per righe
 * i: indice vettore
 * nel: dimensione matrice
 * return: corrispondente indice di colonna della matrice
 */
unsigned int column_index(const unsigned int i, const unsigned int nel);

/*
 * setter riga/colonna di una matrice triangolare
 * memorizzata per righe
 */
void set(sim_metric* dist, const unsigned int nel, const unsigned int row, 
	const unsigned int col, const sim_metric val);

/*
 * getter riga/colonna di una matrice triangolare 
 * memorizzata per righe
 */
sim_metric get(const sim_metric* dist, const unsigned int nel, 
	const unsigned int row, const unsigned int col);

/*
 * metodo che controlla la maschera su un elemento
 * mask: maschera
 * index: indice elemento
 * return: false se il valore della maschera dell'elemento è uguale a 0, 
 * true altrimenti 
 */
bool check_mask_element(const int mask, const unsigned int index);

/*
 * metodo di stampa della matrice che mostra gli indici degli elementi
 * dist: matrice
 * nel: dimensione matrice
 */
void print_matrix_index(const sim_metric* dist, const unsigned int nel);

/*
 * metodo di stampa della matrice triangolare superiore non diagonale, 
 * viene visualizzata come se fosse una matrice quadrata indicando gli 
 * elementi vuoti con un underscore
 * dist: matrice
 * nel: dimensione matrice
 */
void print_matrix(const sim_metric* dist, const unsigned int nel);

/*
 * metodo di stampa della maschera degli elementi attivi
 * mask: maschera
 * nel: numero elementi della maschera
 */
void print_mask(const int mask, const unsigned int nel);

#endif
