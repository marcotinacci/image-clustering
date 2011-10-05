#ifndef __MPI_ROUTINES_H__
#define __MPI_ROUTINES_H__

#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <unistd.h>

#include <string>

#include "file_desc.h"
#include "distance_matrix.h"
#include "siftcmp.h"
#include "img2key.h"
#include "siftget.h"
#include "findlink.h"
#include "mask.h"
#include "cluster.h"
#include "merge.h"

#define TAG_DISTRIB_IMGS 		1
#define TAG_TRANSFER_INFO 		2
#define TAG_ARRAY_NEL_DISTS             3
#define TAG_ARRAY_SIFT			4
#define TAG_CLUSTER_1 			5
#define TAG_CLUSTER_2 			6
#define TAG_PRINT			7
#define IMG_MAX_LEN 350000
//#define IMG_FOLDER "images"
#define FILENAME_MAX_LEN 30

//#define SKIP_CONVERSION

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

using namespace std;

typedef struct _max_info{
	int rank;
	unsigned int index_part;
	unsigned int row;
	unsigned int col;
	sim_metric val;
}max_info;

void slave_clusterize(const unsigned int myrank, const unsigned int nel,
		const unsigned int np, sim_metric ***matrix_parts,
		const unsigned int nel_parts);
	
void master_clusterize(const max_info &maxinfo, const int *map,
		const unsigned int nel, const unsigned int np,
		sim_metric ***matrix_parts, const unsigned int matrix_nel,
		cluster* clusters, int* mask);

void send_stripe(const unsigned int myrank, const unsigned int rank_dest, 
                const unsigned int np, const unsigned int nel, 
                unsigned int * local_info, sim_metric** matrix_parts);

sim_metric * recv_stripe(const unsigned int myrank,
                const unsigned int np, const unsigned int nel, 
                unsigned int * remote_info, sim_metric ***matrix_parts);

sim_metric * get_local_stripe(const unsigned int myrank, const unsigned int np,
		const unsigned int nel, unsigned int * info, sim_metric **matrix_parts);

void master_max_reduce(max_info * local_max, max_info * global_max,
		const int myrank, const int np, int* mask);

void slave_max_reduce(max_info * local_max);

void master_scatter_keys(const int myrank, const unsigned int nel,
		const unsigned int np, string* img_names ,unsigned int* local_nel,
		SIFTs*** desc);
	
void slave_scatter_keys(const int myrank, const unsigned int nel, const unsigned int np, 
	unsigned int *local_nel, SIFTs ***desc_array);
	
void slave_compute_matrix(const int myrank, const unsigned int nel, const unsigned int np, 
	SIFTs** desc_array, unsigned int local_nel, sim_metric ***matrix_parts, unsigned int *nel_parts);
	
void compute_map(const unsigned int np, int **map, unsigned int *nel_map);

void master_compute_matrix(const int myrank, const unsigned int nel, const unsigned int np, 
	const unsigned int local_nel, SIFTs** desc_array, sim_metric ***matrix_parts, unsigned int *nel_parts);

sim_metric* compute_submatrix_dist(SIFTs** desc1, const unsigned int nel1, SIFTs** desc2, 
	const unsigned int nel2);

void print_matrix_parts(sim_metric** parts, const unsigned int nel_parts, const int myrank, 
	const unsigned int np, const unsigned int nel);

void print_submatrix(const sim_metric* dist, const unsigned int row, const unsigned int col);

sim_metric* mpi_compute_matrix(SIFTs **desc, const unsigned int local_nel, const unsigned int nel, 
	const unsigned int np, const int myrank, const int mit, const int dest);

SIFTs** get_array_desc(unsigned char* array_sift, unsigned int* array_nel, unsigned int nel);

unsigned int get_nel_parts(int myrank, unsigned int np);

unsigned int get_index_part(const int myrank, const unsigned int np, const unsigned int row, 
	const unsigned int col);
	
void update_local_cluster(sim_metric *part, sim_metric *buf, const unsigned int index_cluster, 
	const unsigned int row, const unsigned int col, const unsigned int direction);

void local_max_reduce(sim_metric** parts, const unsigned int nel_parts,
		const int myrank, const unsigned int np, const unsigned int nel,
		max_info *maxinf, const int *mask);
//void local_max_reduce(sim_metric** parts, const unsigned int nel_parts, const int myrank,
//	const unsigned int np, const unsigned int nel, sim_metric* max, unsigned int* index_part,
//	unsigned int* c1, unsigned int* c2);
	
/*
 * metodo init della matrice che mappa le sotto-matrici sui terminali
 * np: numero dei terminali
 * return: riferimento alla matrice "stesa", gli elementi indicano i terminali
 */
int* init_matrix_map(const int np);

void print_matrix_map(const int* map, const unsigned int nel);

int get_map(const int* dist, const unsigned int nel, 
	const unsigned int row, const unsigned int col);

void set_map(int* map, const unsigned int nel, const unsigned int row, 
	const unsigned int col, const int val);

/*
 * metodo che dealloca la matrice di mappatura
 * map: matrice di mappatura
 */
void destroy_matrix_map(const int* map);

/*
 * metodo che restituisce il numero di file associati al rango
 * rank: rango del terminale
 * q: nel / np
 * r: nel % np
 * pn: numero di terminali
 * return: numero di file
 */
unsigned int get_nel_by_rank(const int rank, const int q, const int r, const int np);

unsigned int get_nel_by_rank(const int rank, const unsigned int nel, const int np);

void get_part_dim(const int myrank, const unsigned int np, const unsigned int nel,
	const unsigned int index_part, unsigned int &row, unsigned int &col);
	
void get_map_index(const int myrank, const unsigned int index_part, const unsigned int np,
	unsigned int &row, unsigned int &col);

/*
 * metodo che restituisce il percorso relativo dell'immagine nella
 * destinazione
 * rank: rango del terminale di destinazione
 * index: indice locale dell'immagine 
 */
const char* dest_path(const int rank, const unsigned int index);

/*
 * metodo che restituisce la mappa immagine->terminale, le
 * immagini vengono distribuite sugli np-1 slaves (si esclude
 * il master)
 * nel: numero immagini
 * np: numero terminali
 * return: vettore di rank
 */
int* get_img_map(unsigned int nel, int np);

/*
 * metodo che spedisce il file specificato alla destinazione 
 * (MPI_Isend non bloccante)
 * filename: nome del file
 * dest: rango della destinazione
 */
void send_file(const char* filename, int dest);

/*
 * metodo che riceve il file col nome specificato 
 * (MPI_Recv bloccante)
 * filename: nome del file
 * src: rango della sorgente
 */
void receive_file(const char* filename, int src, int dest);

/*
 * metodo che copia un file in locale
 * sourcename: nome del file
 * destname: nome della destinazione
 */
void copy_local_file(const char* sourcename, const char* destname);

/*
 * spedisci la maschera completa a tutti gli slaves
 * mask: vettore maschera
 * np: numero terminali / lunghezza vettore maschera
 */
void send_mask(int *mask, int np);

/*
 * ricevi la maschera completa dal master
 * np: numero terminali / lunghezza vettore maschera
 */
int* receive_mask(int np);

/*
 * converti gli indici riga e colonna della matrice globale a indici riga e
 * colonna della matrice mappa
 */
void matrix_to_map_index(const int matrix_row, const int matrix_col,
		const int nel, int *map_row, int *map_col,
		const int np);

/*
 * stampa la matrice triangolare globale (master)
 */
void master_print_global_matrix(sim_metric ***matrix_parts,
		int *map, const int nel, const int np);

/*
 * stampa la matrice triangolare globale (slave)
 */
void slave_print_global_matrix(const int myrank,
		sim_metric ***matrix_parts, int *map, const int nel,
		const int np);

/*
 * stampa il vettore di maschera globale
 * mask: vettore maschera
 * nel: numero immagini
 * np: numero terminali
 */
void print_global_mask(const int* mask, const int nel, const int np);

/*
 * passa da indici locali a indici globali della matrice
 * myrank: rango terminale
 * np: numero terminali
 * nel: numero immagini
 * index_part: indice della parte
 * l_row: indice riga locale
 * l_col: indice colonna locale
 * g_row: (output) indice riga globale
 * g_col: (output) indice colonna globale
 */
void get_global_index(const unsigned int myrank, const unsigned int np, 
        const unsigned int nel, const unsigned int index_part, 
        unsigned int l_row, unsigned int l_col, unsigned int &g_row, 
        unsigned int &g_col);

#endif
