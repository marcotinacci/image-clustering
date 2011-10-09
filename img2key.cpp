#include "img2key.h"

void convert_all(string** img_names, const unsigned int nel){
	// converti in file key
	#pragma omp parallel for num_threads(omp_get_num_procs()) schedule(dynamic)
	for(int i = 0; i < (int)nel; i++){
		// conversione jpg->pgm
		img2pgm((*img_names)[i].c_str());
		// conversione pgm->key
		pgm2key(ext_pgm((*img_names)[i].c_str()));
		#ifdef DEBUG
			printf("Image %d converted!\n", i);
		#endif
	}
}

int img2pgm(const char* filename){
	pid_t pid = fork();
	if (pid<0) {
		cerr << "Failed to execute conversion img->pgm" << endl;	
		return -1;
	} else if (pid==0) { 
		// Child Process	
		char* output = new char[strlen(filename)+4];

		strcpy(output, filename);
		strcat(output, string(".pgm").c_str());
		execl(CONVERT_PATH, CONVERT_PATH, filename, output , NULL);
		// this is executed in case of error
		cerr << "Failed to run child process" << endl;
		exit(-1);
	} else { 
		// Parent Process
		int status;	
		waitpid(pid, &status, 0);
		return 0; // ok !
	}
}

int pgm2key(const char* filename){
	pid_t pid = fork();
	if (pid<0) {
		cerr << "Failed to execute conversion pgm->key" << endl;	
		return -1;
	} else if (pid==0) { 
		// Child Process
		execl(SIFTPP_PATH, SIFTPP_PATH, filename, NULL);
		// this is executed in case of error
		cerr << "Failed to run child process" << endl;
		exit(-1);
	} else { 
		// Parent Process
		int status;	
		waitpid(pid, &status, 0);
		return 0; // ok !
	}	
}

void generate_img_list_file(){
	/*
		TODO generalizzare il comando
	*/
	system("ls images/*.jpg > temp/filelist");
}

void get_image_names(string** names, unsigned int* nel){
	// apertura file stream
	ifstream inFile;
	inFile.open("temp/filelist");
	if(!inFile) {
		cerr << "Unable to open temp/filelist " << endl;
		exit(1);
	}
	
	// conteggio righe
	*nel = 0;
	while(!inFile.eof()){
		if(inFile.get() == '\n') (*nel)++;
	}
	// alloca spazio delle stringhe
	*names = new string[*nel];
	
	// stream reset
	inFile.clear();
	inFile.seekg(0);

	// lettura file
	for(unsigned int i = 0; i < *nel; i++){
		getline(inFile,(*names)[i]);
	}

	// chiusura file stream
	inFile.close();
}

char* ext_pgm(const char* filename){
	unsigned int len = strlen(filename);
	char* newname = new char[len+4];
	strcpy(newname,filename);
	strcat(newname,".pgm");
	return newname;
}
