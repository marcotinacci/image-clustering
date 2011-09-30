#ifndef __TEST_H__
#define __TEST_H__

#include <time.h>
#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <mpi.h>
#include <sstream>

#include "siftcmp.h"
#include "clusterize.h"
#include "siftget.h"
#include "file_desc.h"
#include "distance_matrix.h"
#include "img2key.h"
#include "mpi_routines.h"
#include "mask.h"
#include "print_results.h"

using namespace std;

#define NUM_CLUSTERS 4
#define DEBUG

/*
 * test globale
 */
void test_main();

/*
 * test sulla clusterizzazione
 */
void test_clusterize();

/*
 * test sull'inizializzazione dei cluster, confronto tra init sequenziale
 * e init parallela con accesso controllato alla sezione critica: visti i
 * tempi si utilizza l'approccio sequenziale
 */
void test_init_cluster();

/*
 * test sulla conversione delle immagini jpg->key
 */
void test_img2key();

/*
 * test min reduction in omp
 */
void test_min_reduction();

/*
 * test della fase di lettura dei sift
 * nel: numero immagini
 */
void test_get_all_sifts(const unsigned int nel);

/*
 * test della fase di elaborazione della matrice di similarità
 * pr_desc_array: vettore dei descrittori precomputato
 * nel: numero immagini
 */
void test_compute_matrix_dist(SIFTs** pc_desc_array, const unsigned int nel);

/*
 * test della fase di ricerca della coppia di cluster da fondere
 * dist: matrice di similarità
 * mask: maschera elementi attivi
 * nel: numero immagini
 */
void test_findlink(const sim_metric* dist, const int mask, const unsigned int nel);

/*
 * test dei tempi di esecuzione multi-thread omp
 */
void test_omp();

/*
 * test su MPI
 */
void test_mpi(int argc, char* argv[]);

/*
 * test sul main con MPI
 */
void test_main_mpi(int argc, char* argv[]);

/*
 * test del metodo get_img_map
 * nel: numero immagini
 * np: numero terminali
 */
void test_get_img_map(unsigned int nel, int np);

#endif
