#include "siftget.h"

SIFTs** get_all_sifts(const string* img_names, const unsigned int nel){
	// suffisso file key
	string suffix (".key");
	SIFTs **desc_array = new SIFTs*[nel];
	#pragma omp parallel for num_threads(omp_get_num_procs()) schedule(dynamic)
	for(int i=0; i<(int)nel; i++){
		desc_array[i] = get_sifts((img_names[i] + suffix).c_str());
	}
	return desc_array;
}

void destroy_all_sifts(SIFTs** desc, const unsigned int nel){
	#pragma omp parallel for num_threads(omp_get_num_procs()) schedule(dynamic)
	for(int i = 0; i < (int)nel; i++){
		delete [] desc[i];
	}
	delete [] desc;
}

SIFTs* get_sifts(const char* file){
	SIFTs* desc;
	float temp;
	short x;
	unsigned int nlines;

	// apertura file stream
	ifstream inFile;
	inFile.open( file );
	if(!inFile) {
		cerr << "Unable to open file " << file << endl;
		exit(1);
	}
	
	// conteggio righe
	nlines = 0;
	while(!inFile.eof()){
		if(inFile.get() == '\n') nlines++;
	}

	// alloca spazio del vettore di sift
	desc = init_desc(nlines);
	
	// stream reset
	inFile.clear();
	inFile.seekg(0);

	// lettura file
	// salta i primi quattro dati
	inFile >> temp;
	inFile >> temp;
	inFile >> temp;
	inFile >> temp;
	for(int sift_count = 0; !inFile.eof(); sift_count++){
		// lettura delle 128 distanze
		for(int j = 0; j < SIFT_SIZE; j++){
			inFile >> x;
			desc->sift_array[sift_count*SIFT_SIZE+j] = x;
		}
		// salta i primi quattro dati di ogni nuova riga
		inFile >> temp;
		inFile >> temp;
		inFile >> temp;
		inFile >> temp;
	}

	// chiusura file stream
	inFile.close();
	return desc;
}
