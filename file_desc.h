#ifndef __FILE_DESC_H__
#define __FILE_DESC_H__

#include <iostream>

#define SIFT_SIZE 128

using namespace std;

// ----------------------------------------------------------------
// STRUTTURE
// ----------------------------------------------------------------

/*
 * descrittore di un'immagine, contiene una matrice che descrive
 * tutti i sift e il numero di righe
 */
typedef struct _SIFTs{
	unsigned char* sift_array; // sift per righe, coordinate per colonne
	unsigned int nel; 	// numero elementi array
} SIFTs;

// ----------------------------------------------------------------
// PROTOTIPI
// ----------------------------------------------------------------

/*
 * metodo che istanzia un file descriptor
 * nel: numero di sift contenuti (righe)
 * return: puntatore a descrittore inizializzato
 */
SIFTs* init_desc(const unsigned int nel);

/*
 * metodo di lettura dell'i-esimo sift
 * desc: descrittore
 * i: indice sift
 * return: i-esimo sift di desc
 */
unsigned char* get_sift(const SIFTs* desc, const unsigned int i);

/*
 * metodo di stampa di un descrittore di file .key
 * d: descrittore
 */
void print_desc(const SIFTs* d);

/*
 * metodo di stampa di un sift
 * s: sift, puntatore a char
 */
void print_sift(const unsigned char* s);

#endif
