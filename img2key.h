#ifndef __IMG2KEY_H__
#define __IMG2KEY_H__

#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <fstream>
#include <omp.h>

#define CONVERT_PATH "/usr/local/bin/convert"
#define SIFTPP_PATH "siftpp/mac/sift"
#define DEBUG

using namespace std;

/*
 * metodo che converte tutte le immagini jpg specificate in file key
 * img_names: puntatore al vettore di nomi dei file da convertire
 * nel: numero di elementi del vettore img_names
 */
void convert_all(string** img_names, const unsigned int nel);

/*
 * metodo che richiama la convert per convertire l'immagine in pgm,
 * verra' creato nel filesystem un file di nome <filename>.pgm
 * filename: nome del file immagine da convertire
 * return: 0 ok, -1 fail
 */
int img2pgm(const char* filename);

/*
 * metodo che richiama il comando per la conversione da file pgm
 * a file .key, verra' creato nel filesystem un file di nome 
 * <filename>{-.pgm}.key
 * filename: nome del file pgm da convertire
 * return: 0 ok, -1 fail
 */
int pgm2key(const char* filename);

/*
 * metodo che genera il file che elenca i nomi delle immagini jpg
 * presenti nella cartella "images/", il file contiene il numero
 * di immagini nella prima riga e i nomi nelle successive (un file
 * per ogni riga). Il nome del file sara' "temp/filelist"
 */
void generate_img_list_file();

/*
 * metodo che ritorna i nomi di ogni file key da elaborare
 * names: array di stringhe contenenti i nomi delle immagini in
 * ordine alfabetico
 * nel: numero di immagini e quindi di elementi del vettore names
 */
void get_image_names(string** names, unsigned int* nel);

/*
 * metodo che modifica l'esensione del nome di un file jpg in pgm
 * i.e. <filename>.jpg -> <filename>.jpg.pgm
 * filename: nome del file jpg
 * return: nome del file pgm
 */
char* ext_pgm(const char* filename);

#endif