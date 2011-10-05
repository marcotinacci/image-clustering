#ifndef __SIFTGET_H__
#define __SIFTGET_H__

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <omp.h>

#ifdef __GNUC__
#include <stdlib.h>
#endif

#include "file_desc.h"
#include "distance_matrix.h"

using namespace std;

/*
 * metodo che carica i sift delle immagini nei rispettivi descrittori
 * img_names: vettore dei nomi dei file
 * nel: numero dei file
 * return: vettore dei descrittori dei file
 */
SIFTs** get_all_sifts(const string* img_names, const unsigned int nel);

/*
 * metodo che dealloca la memora istanziata dal vettore di puntatori
 * a descrittori
 * desc: vettore di puntatori a descrittori da deallocare
 * nel: numero di elementi del vettore
 */
void destroy_all_sifts(SIFTs** desc, const unsigned int nel);

/*
 * metodo che carica i sift di un'immagine in un descrittore
 */
SIFTs* get_sifts(const char* file);

#endif
