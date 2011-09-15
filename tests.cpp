#include "tests.h"

void test_main_mpi(int argc, char* argv[]){
	unsigned int nel;
	int np, myrank;
	string* img_names;
	ostringstream oss;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	oss << myrank;
	// crea folder images_i su ogni terminale
	system(("mkdir images_"+oss.str()).c_str());
	
	// genera i nomi di tutti i file su master
	if(myrank == 0){ // master
		generate_img_list_file();
		get_image_names(&img_names, &nel);
	}
	
	// informa tutti del numero totale di elementi
	MPI_Bcast(&nel,1,MPI_INT,0,MPI_COMM_WORLD);

	unsigned int local_nel = 0;
	SIFTs** desc_array = NULL;
		
	if(myrank == 0){ // master
		cout << "DISTRIBUZIONE E CONVERSIONE IMMAGINI" << endl;
		master_scatter_keys(0,nel,np,img_names,&local_nel,&desc_array);

		MPI_Barrier(MPI_COMM_WORLD); // TODO eliminare in deploy
		cout << "ELABORAZIONE MATRICE DISTANZE" << endl;
		
		int *map = NULL;
		unsigned int nel_map = 0;
		sim_metric **matrix_parts = NULL;
		unsigned int nel_parts = 0;
		master_compute_map(np, &map, &nel_map);
		master_compute_matrix(0, nel, np, local_nel, desc_array, &matrix_parts,
				&nel_parts);
		//print_matrix_parts(matrix_parts, nel_parts, 0, np, nel);

		MPI_Barrier(MPI_COMM_WORLD); // TODO eliminare in deploy
		cout << "CLUSTERING" << endl;

		// crea i cluster
		cluster *clusters = init_cluster();
		// alloca la lista di cluster, ogni cluster e' una lista di indici
		for(int i=0; i < (int)nel; i++){
			clusters->insert(pair<unsigned int, list<unsigned int> > (i,
					list<unsigned int>(1,i)));
		}

		// crea maschera
		int *mask = init_mask(np);

		for(unsigned int i=0; i < nel - NUM_CLUSTERS; i++){
			cout << "Giro " << i << endl;
			unsigned int c1, c2;
			max_info local_max, global_max;

			send_mask(mask, np);
			local_max_reduce(matrix_parts, nel_parts, 0, np, nel, &local_max,
					mask);
			//printf("master max = %d, index = %d (%d,%d)\n", max, index_max, c1, c2);

			master_max_reduce(&local_max, &global_max, 0, np, clusters, mask);

			master_clusterize(global_max, map, nel, np, &matrix_parts,
					nel_parts, clusters, mask);
		}
		//print_matrix_parts(matrix_parts, nel_parts, 0, np, nel);
		/*
			TODO 
				clustering distribuito
				scrittura del file html di output
		*/
		// distruggi i cluster
		destroy_cluster(clusters);
	}else{ // slaves
		slave_scatter_keys(myrank, nel, np, &local_nel, &desc_array);
		
		MPI_Barrier(MPI_COMM_WORLD); // TODO eliminare in deploy
		
		sim_metric **matrix_parts = NULL;
		unsigned int nel_parts = 0;
		slave_compute_matrix(myrank, nel, np, desc_array, local_nel, &matrix_parts, &nel_parts);
		//print_matrix_parts(matrix_parts, nel_parts, myrank, np, nel);

		MPI_Barrier(MPI_COMM_WORLD); // TODO eliminare in deploy

		int* mask;
		for(unsigned int i=0; i < nel -NUM_CLUSTERS; i++){
			max_info local_max;
			mask = receive_mask(np);
			local_max_reduce(matrix_parts, nel_parts, myrank, np, nel,
					&local_max, mask);
			slave_max_reduce(&local_max);
			slave_clusterize(myrank, nel, np, &matrix_parts, nel_parts);
		}

	}
	MPI_Finalize();	
}

void test_get_img_map(unsigned int nel, int np){
	int* vett = get_img_map(nel, np);
	for(unsigned int i=0; i<nel; i++){
		printf("map[%d] = %d \n",i,vett[i]);
	}
}

void test_mpi(int argc, char* argv[]){
	int np, myrank;
	unsigned int nel;
	double wall_timer;	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np); 
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	string* img_names;
	generate_img_list_file();
	get_image_names(&img_names, &nel);
	if(myrank == 0){
		for(unsigned int i=0; i<nel; i++){
			send_file(img_names[i].c_str(), 1);
		}
	}else{
		wall_timer = omp_get_wtime();
		for(unsigned int i=0; i<nel; i++){
			receive_file(dest_path(myrank, i), 0, 1);
		}
		convert_all(&img_names,nel);
		get_all_sifts(img_names,nel);
		cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;
	}
	MPI_Finalize();
}

void test_omp(){
	int i, nthreads;
	clock_t clock_timer;
	double wall_timer;
	for(nthreads = 1; nthreads <=8; nthreads++) {
		clock_timer = clock();
		wall_timer = omp_get_wtime();
		#pragma omp parallel for private(i) num_threads(nthreads)
		for (i = 0; i < 10; i++) sleep(0.5);
		cout << nthreads << " threads: time on clock() = " << 
			((double) (clock() - clock_timer) / CLOCKS_PER_SEC) <<
			", on wall = " << (omp_get_wtime() - wall_timer) << endl;
	}
}

void test_min_reduction(){
	int vett[10] = {1,3,2,4,7,7,7,7,7,7};
	int sh_min = 1000;
	int sh_min2 = 1000;
	int num_procs = omp_get_num_procs();
	cout << "num_procs = " << num_procs << endl;
	#pragma omp parallel num_threads(2)
	{
		int min = 1000;
		int min2 = 1000;
		#pragma omp for schedule(static)
		for(int i = 0; i < 10; i++){
			printf("thread %d - vett[%d] = %d \n",omp_get_thread_num(),i,vett[i]);
			if(vett[i] < min){
				min2 = min;
				min = vett[i];
			}else if(vett[i] < min2){
				min2 = vett[i];
			}
		}
		#pragma omp critical
		{
			if(min < sh_min){
				sh_min2 = sh_min;
				sh_min = min;				
			} 
			else if(min < sh_min2){
				sh_min2 = min;
			}
			if(min2 < sh_min2){
				sh_min2 = min2;
			}
		}
	}
	cout << "min = " << sh_min << endl;
	cout << "min2 = " << sh_min2 << endl;	
}

void test_img2key(){
	unsigned int nel;
	string* img_names;

	cout << "- Generating img list file" << endl;	
	generate_img_list_file();
	
	cout << "- Test image conversion jpg->key" << endl;
	double wall_timer = omp_get_wtime();	
	// acquisisci i nomi delle immagini
	get_image_names(&img_names, &nel);
	// converti le immagini
	convert_all(&img_names,nel);
	cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;
}

void test_clusterize(){
	unsigned int nel = 7;
	int mask;
	sim_metric matrix[] = {6,5,4,3,2,1,1,2,3,4,5,9,11,3,7,3,4,5,10,1,4};
	
	cout << "- Allocating clusters" << endl;
	cluster* clusters = init_cluster();	
	cout << "- Clusterizing" << endl;
	double wall_timer = omp_get_wtime();	
	// clustering
	clusterize(clusters,matrix,&mask,nel);
	cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;	
	
	print_clusters(clusters);
	destroy_cluster(clusters);
}

void test_init_cluster(){
	cluster* clusters = new cluster();
	double wall_timer = omp_get_wtime();
	for(int i=0; i < 1000000; i++){
		clusters->insert(pair<unsigned int, list<unsigned int> >(i,list<unsigned int>(1,i)));
	}
	cout << "Elapsed time sequential: " << (omp_get_wtime() - wall_timer) << endl;
	
	clusters->clear();
	wall_timer = omp_get_wtime();
	#pragma omp parallel for schedule(dynamic) num_threads(omp_get_num_procs())
	for(int i=0; i < 1000000; i++){
		pair<unsigned int, list<unsigned int> > p(i,list<unsigned int>(1,i));
		#pragma omp critical
		{
			clusters->insert(p);
		}
	}
	cout << "Elapsed time parallel(cs): " << (omp_get_wtime() - wall_timer) << endl;	
}

void test_get_all_sifts(){
	double wall_timer = omp_get_wtime();
	unsigned int nel;
	string* names;
	get_image_names(&names,&nel);
	get_all_sifts(names,nel);
	cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;
}

void test_compute_matrix_dist(SIFTs** pc_desc_array, const unsigned int nel){
	double wall_timer = omp_get_wtime();
	sim_metric* matrix = compute_matrix_dist(pc_desc_array, nel);
	cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;
	print_matrix(matrix,nel);
	cout << "Deallocating resources" << endl;
	destroy_matrix_dist(matrix);
}

void test_findlink(const sim_metric* dist, const int mask, const unsigned int nel){
	unsigned int c1, c2;
	double wall_timer = omp_get_wtime();
	findlink_tri(dist, mask, nel, &c1, &c2, 0);
	cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;
	cout << "c1: " << c1 << " - c2: " << c2 << endl;
}

void test_main(){
	double wall_timer = omp_get_wtime();
	// numero di immagini
	unsigned int nel;
	// maschera della matrice
	int mask;
	// vettore nomi immagini
	string* img_names;
	
	cout << "- Allocating resources" << endl;
	// clusters
	cluster* clusters = init_cluster();
	// acquisisci i nomi delle immagini
	generate_img_list_file();
	get_image_names(&img_names, &nel);

	// converti le immagini
	cout << "- Converting images" << endl;
	convert_all(&img_names,nel);

	// acquisici sifts
	cout << "- Reading SIFTS from files" << endl;
	SIFTs** desc = get_all_sifts(img_names,nel);

	// elabora la matrice di similarità
	cout << "- Computing distance matrix" << endl;
	sim_metric* matrix = compute_matrix_dist(desc,nel);
	print_matrix(matrix,nel);

	// clustering
	cout << "- Clustering" << endl;
	clusterize(clusters,matrix,&mask,nel);

	#ifdef DEBUG
		print_mask(mask,nel);	
		print_matrix(matrix,nel);
		print_clusters(clusters);
	#endif
	
	cout << "Elapsed time: " << (omp_get_wtime() - wall_timer) << endl;

	// dealloca risorse
	cout << "- Deallocating resources" << endl;
	destroy_cluster(clusters);	
	destroy_matrix_dist(matrix);
	destroy_all_sifts(desc,nel);
}
