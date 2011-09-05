#include "file_desc.h"

SIFTs* init_desc(const unsigned int nel){
	SIFTs* desc = new SIFTs;
	desc->sift_array = new unsigned char[SIFT_SIZE*nel];
	desc->nel = nel;
	return desc;
}

void print_desc(const SIFTs* d){
	cout << "File descriptor:" << endl;
	cout << "Sift number: " << d->nel << endl;
	for(unsigned int i = 0; i < d->nel; i++){
		cout << "SIFT " << i << ": ";
		print_sift(get_sift(d,i));
	}
	cout << endl;
}

unsigned char* get_sift(const SIFTs* desc, const unsigned int i){
	return &(desc->sift_array[i*SIFT_SIZE]);
}

void print_sift(const unsigned char* s){
	for(int j = 0; j < SIFT_SIZE; j++){
		cout << (unsigned short)s[j] << ',';
	}
	cout << endl;
}