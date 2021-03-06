#include "mpi_routines.h"

void master_clusterize(const max_info &maxinfo, const int *map,
		const unsigned int nel, const unsigned int np,
		sim_metric ***matrix_parts, const unsigned int matrix_nel,
		cluster* clusters, int* mask)
{
    
	/*
		TODO usare una struttura
	*/
	// info[0]: rank con cui comunicare (rank = np -> terminazione)
	// info[1]: index cluster
	// info[2]: index part
	// info[3]: 0 row, 1 column
	// info[4]: 0 mit, 1 dest
	unsigned int info_mit[5], info_dest[5]; // vettori informazioni
	unsigned int i_row, i_col; // coordinate sotto-matrice sulla mappa
	get_map_index(maxinfo.rank, maxinfo.index_part, np, i_row, i_col);
	for(unsigned int i = 0; i < np; i++){
		// elabora i dati da spedire
		info_mit[0] = get_map(map, np, i_row, i);
		info_dest[0] = get_map(map, np, i, i_col);
		info_mit[1] = maxinfo.row;
		info_dest[1] = maxinfo.col;
		info_mit[2] = get_index_part(info_mit[0], np, i_row, i);
		info_dest[2] = get_index_part(info_dest[0], np, i_col, i);
		info_mit[3] = (i_row < i) ? 0 : 1;
		info_dest[3] = (i_col < i) ? 0 : 1;
		// la cifra specifica il ruolo di chi riceve le info!
		info_mit[4] = 0;
		info_dest[4] = 1;

		// gestione comunicazione
		if(info_mit[0] == 0 && info_dest[0] == 0){ // master-master
			sim_metric *buf = get_local_stripe(0, np, nel, info_mit,
					*matrix_parts);
			// aggiornare elementi locali
			unsigned int row, col, direction;
			get_part_dim(0, np, nel, info_dest[2], row, col);
			direction = (info_dest[2] == 0) ? 2 : info_dest[3];
			update_local_cluster((*matrix_parts)[info_dest[2]], buf, info_dest[1],
					row, col, direction);
		}else if(info_mit[0] == 0){ // master-slave

			MPI_Send(info_mit, 5, MPI_UNSIGNED, info_dest[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
			MPI_Send(info_dest, 5, MPI_UNSIGNED, info_dest[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
                        
                        send_stripe(0, info_dest[0], np, nel, info_mit, *matrix_parts);
                        
		}else if(info_dest[0] == 0){ // slave-master
			MPI_Send(info_dest, 5, MPI_UNSIGNED, info_mit[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
			MPI_Send(info_mit, 5, MPI_UNSIGNED, info_mit[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
                        
                        sim_metric* buf = recv_stripe(0, np, nel, info_mit, matrix_parts);
                        // aggiornare elementi locali
                        unsigned int row, col, direction;
                        get_part_dim(0, np, nel, info_mit[2], row, col);
                        direction = (info_mit[2] == 0) ? 2 : info_mit[3];
                        update_local_cluster((*matrix_parts)[info_mit[2]], buf, info_mit[1],
                                        row, col, direction);
                        
		}else{ // slave-slave
			MPI_Send(info_mit, 5, MPI_UNSIGNED, info_dest[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
			MPI_Send(info_dest, 5, MPI_UNSIGNED, info_dest[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
                        
			MPI_Send(info_mit, 5, MPI_UNSIGNED, info_mit[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
			MPI_Send(info_dest, 5, MPI_UNSIGNED, info_mit[0], TAG_CLUSTER_1,
					MPI_COMM_WORLD);
		}
	}
        
        update_mask(mask, maxinfo.rank, maxinfo.index_part, maxinfo.row,
			maxinfo.col, np);
        unsigned int g_row, g_col;

        get_global_index(maxinfo.rank, np, nel, maxinfo.index_part, maxinfo.row,
                maxinfo.col, g_row, g_col);
        
        merge_clusters(clusters, g_row, g_col);
        
	// invia messaggi di terminazione fase (rank = -1)
	for(unsigned int i = 1; i < np; i++){
		info_mit[0] = np;
		MPI_Send(info_mit, 5, MPI_UNSIGNED, i, TAG_CLUSTER_1, MPI_COMM_WORLD);
                MPI_Send(info_mit, 5, MPI_UNSIGNED, i, TAG_CLUSTER_1, MPI_COMM_WORLD);
	}

}

void slave_clusterize(const unsigned int myrank, const unsigned int nel,
		const unsigned int np, sim_metric ***matrix_parts,
		const unsigned int nel_parts)
{
	unsigned int info_mit[5], info_dest[5];
	MPI_Status state;
	while(true){
		// ricevi rank mittente/destinatario
		MPI_Recv(info_mit, 5, MPI_UNSIGNED, 0, TAG_CLUSTER_1, MPI_COMM_WORLD,
				&state);
		MPI_Recv(info_dest, 5, MPI_UNSIGNED, 0, TAG_CLUSTER_1, MPI_COMM_WORLD,
				&state);
		if(info_mit[0] == np){ // terminare fase
			break;
		}else if(info_dest[0] == info_mit[0]){ // eseguire l'aggiornamento in locale
			unsigned int info_mit2[5], info_dest2[5];
			// seconda ricezione (fittizia, si hanno gia' tutti i dati)
			MPI_Recv(info_mit2, 5, MPI_UNSIGNED, 0, TAG_CLUSTER_1, MPI_COMM_WORLD,
					&state);
			MPI_Recv(info_dest2, 5, MPI_UNSIGNED, 0, TAG_CLUSTER_1, MPI_COMM_WORLD,
					&state);

                        sim_metric *buf = get_local_stripe(myrank, np, nel, 
                                info_mit, *matrix_parts);

			// aggiornare elementi locali
			unsigned int row, col, direction;
			get_part_dim(myrank, np, nel, info_dest[2], row, col);
			direction = (info_dest[2] == 0) ? 2 : info_dest[3];
			update_local_cluster((*matrix_parts)[info_dest[2]], buf, info_dest[1],
					row, col, direction);
		}else{ // comunicare e aggiornare i cluster
			// TODO scegliere in base al bilanciamento del carico
			if(info_dest[0] == myrank){ // questo slave e' destinatario
				sim_metric *buf = recv_stripe(myrank, np, nel, info_mit,
						matrix_parts);
				
				// aggiornare elementi locali
				unsigned int row, col, direction;
				get_part_dim(myrank, np, nel, info_dest[2], row, col);
				direction = (info_dest[2] == 0) ? 2 : info_dest[3];
				update_local_cluster((*matrix_parts)[info_dest[2]], buf, info_dest[1],
						row, col, direction);
			}else{ // questo slave e' mittente
                                send_stripe(myrank, info_dest[0], np, nel, info_mit, *matrix_parts);
			}
		}
	}
}



void update_local_cluster(sim_metric *part, sim_metric *buf,
		const unsigned int index_cluster, const unsigned int row,
		const unsigned int col, const unsigned int direction)
{
	// direction: 0 row, 1 col, 2 tri
	const unsigned int dim = (direction == 0) ? col : row;
	switch(direction){
		case 0: // riga
		for(unsigned int i = 0; i < dim; i++){
			if(part[index_cluster*col+i] > buf[i]){
                            part[index_cluster*col+i] = buf[i];
			}
		}
		break;
		case 1: // colonna
		for(unsigned int i = 0; i < dim; i++){
			if(part[index_cluster+col*i] > buf[i]){
				part[index_cluster+col*i] = buf[i];
			}
		}
		break;
		case 2: // triangolare
		for(unsigned int i = 0; i > dim; i++){
			if(get(part,dim,i,index_cluster) > buf[i]){
                                set(part,dim,i,index_cluster,buf[i]);
			}
		}
		break;
		default:
			cerr << "Errore: codice direction = " << direction << endl;
		break;
	}
}

void send_stripe(const unsigned int myrank, const unsigned int rank_dest, const unsigned int np,
		const unsigned int nel, unsigned int * local_info, sim_metric** matrix_parts)
{
	unsigned int row, col, dim;
	get_part_dim(myrank, np, nel, local_info[2], row, col);
	// numero dati da spedire
	// se e' triangolare (info[2] == 0) col e row sono uguali
	dim = (local_info[3] == 0) ? col : row; // dimensione striscia
	sim_metric *buf = new sim_metric[dim];
	// copia gli elementi della sotto-matrice
	for(unsigned int i = 0; i < dim; i++){
		if(local_info[2] == 0){ // triangolare
			buf[i] = get(matrix_parts[0],dim,i,local_info[1]);
		}else if(local_info[3] == 0){ // riga
			buf[i] = matrix_parts[local_info[2]][local_info[1]*col+i];
		}else if(local_info[3] == 1){ // colonna
			buf[i] = matrix_parts[local_info[2]][col*i+local_info[1]];
		}else{
			cerr << "Errore: codice info[3] = " << local_info[3] << endl;
		}
	}
	MPI_Send(buf, dim * sizeof(sim_metric), MPI_BYTE, rank_dest, TAG_CLUSTER_2,
			MPI_COMM_WORLD);
}

sim_metric * recv_stripe(const unsigned int myrank,
                const unsigned int np, const unsigned int nel, 
                unsigned int * remote_info, sim_metric ***matrix_parts)
{
	MPI_Status state;
	unsigned int row, col, dim;
	get_part_dim(myrank, np, nel, remote_info[2], row, col);
	// numero dati da ricevere
	dim = (remote_info[3] == 0) ? col : row;
	sim_metric *buf = new sim_metric[dim];
	MPI_Recv(buf, dim * sizeof(sim_metric), MPI_BYTE, remote_info[0], TAG_CLUSTER_2,
			MPI_COMM_WORLD, &state);
	return buf;
}

sim_metric * get_local_stripe(const unsigned int myrank, const unsigned int np,
		const unsigned int nel, unsigned int * info, sim_metric **matrix_parts)
{
	unsigned int row, col, dim;
	get_part_dim(myrank, np, nel, info[2], row, col);
	// numero dati da ricevere
	dim = (info[3] == 0) ? col : row;
	sim_metric *buf = new sim_metric[dim];

	// copia gli elementi della sotto-matrice
	for(unsigned int i = 0; i < dim; i++){
		if(info[2] == 0){ // triangolare
			buf[i] = get(matrix_parts[0],dim,i,info[1]);
		}else if(info[3] == 0){ // riga
			buf[i] = matrix_parts[info[2]][info[1]*col+i];
		}else if(info[3] == 1){ // colonna
			buf[i] = matrix_parts[info[2]][col*i+info[1]];
		}else{
			cerr << "Errore: codice info[3] = " << info[3] << endl;
		}
	}
	return buf;
}

unsigned int get_index_part(const int myrank, const unsigned int np,
		const unsigned int row, const unsigned int col)
{
	unsigned int c = col;
	unsigned int r = row;
	if(row > col){
		r = col;
		c = row;
	}
	if((unsigned int)myrank == r){
		return c - myrank;
	}else{
		return r + np - myrank;
	}
}

void get_map_index(const int myrank, const unsigned int index_part,
		const unsigned int np, unsigned int &row, unsigned int &col)
{
	if(myrank + index_part < np){
		row = myrank;
		col = myrank + index_part;
	}else{
		row = (myrank + index_part) % np;
		col = myrank;
	}
}

void matrix_to_map_index(const int matrix_row, const int matrix_col,
		const int nel, int *map_row, int *map_col,
		const int np){
	const int q = nel/np;
	const int r = nel%np;

	// calcola l'indice riga
	if(matrix_row < (np-r) * q){
		*map_row = matrix_row / q;
	}else{
		*map_row = (np-r) + (matrix_row - (np-r)*q ) / (q+1);
	}

	// calcola l'indice colonna
	if(matrix_col < (np-r) * q){
		*map_col = matrix_col / q;
	}else{
		*map_col = (np-r) + (matrix_col - (np-r)*q) / (q+1);
	}
}

void get_part_dim(const int myrank, const unsigned int np,
		const unsigned int nel, const unsigned int index_part,
		unsigned int &row, unsigned int &col)
{
	if(myrank + index_part < np){
		row = get_nel_by_rank(myrank, nel, np);
		col = get_nel_by_rank(myrank + index_part, nel, np);
	}else{
		row = get_nel_by_rank((myrank + index_part) % np, nel, np);
		col = get_nel_by_rank(myrank, nel, np);
	}
}

void master_max_reduce(max_info * local_max, max_info * global_max,
		const int myrank, const int np, int* mask)
{
	max_info *max_vett = new max_info[np];
	// TODO eseguire con MPI_AllReduce MAX_LOC
	MPI_Gather(local_max, sizeof(max_info), MPI_BYTE, max_vett,
			sizeof(max_info), MPI_BYTE, myrank, MPI_COMM_WORLD);

	unsigned int i_max = 0;
	for(int i = 1; i < np; i++){
		if(max_vett[i].val > max_vett[i_max].val){
			// aggiorna massimo
			i_max = i;
		}
	}
	*global_max = max_vett[i_max];
	delete[] max_vett;
}

void slave_max_reduce(max_info * local_max){
	MPI_Gather(local_max, sizeof(max_info), MPI_BYTE, NULL, 0,
			MPI_DATATYPE_NULL, 0, MPI_COMM_WORLD);
}


void master_scatter_keys(const int myrank, const unsigned int nel,
		const unsigned int np, string* img_names, unsigned int* local_nel,
		SIFTs*** desc_array)
{
	// calcola q e r
	unsigned int q = nel / np;
	unsigned int r = nel % np;
	// np1 = np - r : numero di terminali con q elementi
	// np2 = r : numero di terminali con q+1 elementi
	unsigned int np1 = np-r;
	// numero di immagini in locale
	*local_nel = get_nel_by_rank(myrank, q, r, np);
	// vettore di puntatori a descrittori
 	*desc_array = new SIFTs*[*local_nel];

	// scatter immagini
	for(unsigned int i = 0; i < q+1; i++){ // cicla sull'offset
		unsigned int k = i;
		if(i < q){
                    for(unsigned int j = 0; j < np1; j++, k+=q){ // cicla su le partizioni q
                        if(j==0){
                            copy_local_file(img_names[k].c_str(), dest_path(0, i));					
                            const char* local_filename = dest_path(0, i);
                            #ifndef SKIP_CONVERSION
                                    // conversione jpg->pgm
                                    img2pgm(local_filename);
                                    // conversione pgm->key
                                    pgm2key(ext_pgm(local_filename));
                            #endif
                            // leggi i sifts
                            (*desc_array)[i] = get_sifts((string(local_filename) +
                                            string(".key")).c_str());
                        }else{
                            #ifndef SKIP_CONVERSION
                                    send_file(img_names[k].c_str(), j);
                            #endif                                
                        }
                    }
		}else{
                    // fai avanzare l'indice all'ultimo elemento della prima partizione q+1
                    k = i + q * np1;
		}

		for(unsigned int j = np1; j < np; j++, k += q+1){ // cicla su le partizioni q+1
                    send_file(img_names[k].c_str(), j);
		}
	}
}

void slave_scatter_keys(const int myrank, const unsigned int nel,
		const unsigned int np, unsigned int *local_nel, SIFTs ***desc_array)
{
	// calcola q e r
	unsigned int q = nel / np;
	unsigned int r = nel % np;
	// numero di immagini in locale
	*local_nel = get_nel_by_rank(myrank, q, r, np);
	// vettore di puntatori a descrittori
	*desc_array = new SIFTs*[*local_nel];
	// suffisso file key
	string suffix (".key");
	for(unsigned int i = 0; i < *local_nel; i++){
		const char* local_filename = dest_path(myrank, i);
		#ifndef SKIP_CONVERSION
			receive_file(local_filename, 0, myrank);
			// conversione jpg->pgm
			img2pgm(local_filename);
			// conversione pgm->key
			pgm2key(ext_pgm(local_filename));
		#endif
		// leggi i sifts
		(*desc_array)[i] = get_sifts((string(local_filename) + suffix).c_str());
	}
}

void compute_map(const unsigned int np, int **map, unsigned int *nel_map)
{
	// inizializza mappa
	*map = init_matrix_map(np);
	// numero elementi mappa
	*nel_map = np*(np-1)/2;
	// calcola mappa
        #pragma omp parallel for num_threads(omp_get_num_procs()) schedule(static)
	for(unsigned int i = 0; i < *nel_map; i++){
            set_map(*map, np, i % np, (i + i/np + 1) % np, i % np);
	}
}

void master_compute_matrix(const int myrank, const unsigned int nel,
		const unsigned int np, const unsigned int local_nel, SIFTs** desc_array,
		sim_metric ***matrix_parts, unsigned int *nel_parts)
{
	// matrice di assegnamento sezione
	unsigned int num_cycle = (np-1)/2;
	*nel_parts = get_nel_parts(myrank, np);
	sim_metric **mat = new sim_metric*[*nel_parts + 1];
	// elabora la matrice triangolare
	mat[0] = compute_matrix_dist(desc_array,local_nel);
	for(unsigned int i = 0; i < num_cycle; i++){
		// calcola sotto-matrici remote
                // non parallelizzare -> errori run-time su linux
                // #pragma omp parallel for num_threads(omp_get_num_procs()) schedule(dynamic)
		for(int j = 1; j < np; j++){
			int info[] = {(j+i+1) % np, (j-i-1+np) % np};
			MPI_Send((void*)info, 2, MPI_INT, j, TAG_TRANSFER_INFO,
					MPI_COMM_WORLD);
		}
                
		// calcola sotto-matrice locale
		mat[i+1] = mpi_compute_matrix(desc_array, local_nel, nel, np, myrank,
				(i+1) % np, (np-i-1) % np);
	}
	// se il numero di terminali e' pari fare un altro ciclo di chiusura
	if(np % 2 == 0){
		// calcola sotto-matrici remote
		for(unsigned int j = 0; j < (unsigned int)(np / 2); j++){
			// sola ricezione
			if(j != 0){
				int info1[] = {np/2 +j, -1};
				MPI_Send((void*)info1, 2, MPI_INT, j, TAG_TRANSFER_INFO,
						MPI_COMM_WORLD);
			}
			// sola trasmissione
			int info2[] = {-1, j};
			MPI_Send((void*)info2, 2, MPI_INT, np/2 + j, TAG_TRANSFER_INFO,
					MPI_COMM_WORLD);
		}
		// calcola sotto-matrice locale
		mat[*nel_parts] = mpi_compute_matrix(desc_array, local_nel, nel, np, 0,
				np/2, -1);
	}
	*matrix_parts = mat;
}

void slave_compute_matrix(const int myrank, const unsigned int nel, const unsigned int np, 
		SIFTs** desc_array, unsigned int local_nel, sim_metric ***matrix_parts, unsigned int *nel_parts)
{
	MPI_Status state;
	int info[2];
	unsigned int num_cycle = (np - 1) / 2;
	*nel_parts = get_nel_parts(myrank, np);
	// init matrix_parts
	sim_metric **mat = new sim_metric*[*nel_parts + 1];
	// elabora la matrice triangolare
	mat[0] = compute_matrix_dist(desc_array,local_nel);
	for(unsigned int i = 0; i < num_cycle; i++){
		// ricevi le informazioni di scambio dati
		MPI_Recv(info, 2, MPI_INT, 0, TAG_TRANSFER_INFO, MPI_COMM_WORLD, &state);
		// elabora la sotto matrice (myrank, myrank + i % np)
		mat[i+1] = mpi_compute_matrix(desc_array, local_nel, nel, np, myrank, info[0], info[1]);
	}
	if(np % 2 == 0){
		MPI_Recv(info, 2, MPI_INT, 0, TAG_TRANSFER_INFO, MPI_COMM_WORLD, &state);
		sim_metric* temp = mpi_compute_matrix(desc_array, local_nel, nel, np, myrank, info[0], info[1]);
		if(info[1] == -1){
			mat[*nel_parts] = temp;
		}
	}
	*matrix_parts = mat;
}

sim_metric* mpi_compute_matrix(SIFTs **desc, const unsigned int local_nel, const unsigned int nel, 
		const unsigned int np, const int myrank, const int mit, const int dest)
{
	MPI_Request req[2];
	MPI_Status state[2];

	// numero di descrittori da spedire
	unsigned int send_nel = local_nel + 1;
	// numero di descrittori da ricevere
	unsigned int recv_nel = get_nel_by_rank(mit, nel/np, nel%np, np) + 1;
	
	// crea vettore dimensioni descrittori da spedire
	unsigned int *send_array_nel = new unsigned int[send_nel];
	unsigned int sum = 0;

        #pragma omp parallel for num_threads(omp_get_num_procs()) schedule(static)
        for(unsigned int j = 1; j < send_nel; j++){
            send_array_nel[j] = desc[j-1]->nel;
            #pragma omp critical
            {
                sum += send_array_nel[j];
            }
        }
	send_array_nel[0] = sum; // il primo elemento è la somma totale

	// crea vettore dimensioni descrittori da ricevere
	unsigned int *recv_array_nel = new unsigned int[recv_nel];

	// scambio vettori delle dimensioni
	if(mit == -1){
		MPI_Send((void*)send_array_nel, send_nel, MPI_UNSIGNED, dest, TAG_ARRAY_NEL_DISTS, MPI_COMM_WORLD);
	}else if(dest == -1){
		MPI_Recv(recv_array_nel, recv_nel, MPI_UNSIGNED, mit, TAG_ARRAY_NEL_DISTS, MPI_COMM_WORLD,&state[0]);
	}else{
		MPI_Irecv(recv_array_nel, recv_nel, MPI_UNSIGNED, mit, TAG_ARRAY_NEL_DISTS, MPI_COMM_WORLD, &req[0]);
		MPI_Isend((void*)send_array_nel, send_nel, MPI_UNSIGNED, dest, TAG_ARRAY_NEL_DISTS, MPI_COMM_WORLD, &req[1]);
		MPI_Waitall(2,req,state);
	}

	unsigned char* send_array_sift = NULL;
	unsigned char* recv_array_sift = NULL;

	if(mit == -1 || dest != -1){
		// crea vettore globale sift
		send_array_sift = new unsigned char[send_array_nel[0]*SIFT_SIZE];
		unsigned int begin_desc = 0;
		for(unsigned int i=0; i < local_nel; i++){
			for(unsigned int j=0; j < send_array_nel[i+1]; j++){
                                #pragma omp parallel for num_threads(omp_get_num_procs()) schedule(static)
				for(unsigned int k=0; k<SIFT_SIZE; k++){
					send_array_sift[(begin_desc+j)*SIFT_SIZE+k] = get_sift(desc[i],j)[k];
				}
			}
			begin_desc += send_array_nel[i+1];
		}
	}
        
        if(mit != -1){
            recv_array_sift = new unsigned char[recv_array_nel[0]*SIFT_SIZE];
        }

	// scambio vettori sift globali
	if(mit == -1){
		MPI_Send((void*)send_array_sift, send_array_nel[0]*SIFT_SIZE, MPI_UNSIGNED_CHAR, dest, TAG_ARRAY_SIFT, MPI_COMM_WORLD);
		return NULL; // risparmia sul calcolo della matrice
	}else if(dest == -1){
		MPI_Recv(recv_array_sift, recv_array_nel[0]*SIFT_SIZE, MPI_UNSIGNED_CHAR, mit, TAG_ARRAY_SIFT, MPI_COMM_WORLD, &state[0]);
	}else{
		MPI_Irecv(recv_array_sift, recv_array_nel[0]*SIFT_SIZE, MPI_UNSIGNED_CHAR, mit, TAG_ARRAY_SIFT, MPI_COMM_WORLD, &req[0]);
		MPI_Isend((void*)send_array_sift, send_array_nel[0]*SIFT_SIZE, MPI_UNSIGNED_CHAR, dest, TAG_ARRAY_SIFT, MPI_COMM_WORLD, &req[1]);
		MPI_Waitall(2,req,state);		
	}
	
	// struttura i dati coi descrittori
	SIFTs** recv_desc_array = get_array_desc(recv_array_sift, recv_array_nel, recv_nel);
	sim_metric* dist;
	
	if(mit < myrank){
		dist = compute_submatrix_dist(recv_desc_array, recv_nel-1, desc, local_nel);
	}else{
		dist = compute_submatrix_dist(desc, local_nel, recv_desc_array, recv_nel-1);
	}

	return dist;
}

unsigned int get_nel_parts(int myrank, unsigned int np){
	unsigned int matrix_nel = np*(np-1)/2;
	if(np % 2 != 0){
		return matrix_nel / np;
	}else if(myrank < (int)np/2){
		return (matrix_nel / np) + 1;
	}else{
		return matrix_nel / np;
	}
}

sim_metric* compute_submatrix_dist(SIFTs** desc1, const unsigned int nel1, SIFTs** desc2, const unsigned int nel2){
	unsigned int nel = nel1*nel2;
	sim_metric* dist = new sim_metric[nel];
	for(unsigned int i = 0; i < nel; i++){
		unsigned int row = i % nel2; // indice riga
		unsigned int col = i / nel2; // indice colonna
		dist[i] = sim_degree(desc1[col], desc2[row]);
	}
	return dist;
}

void local_max_reduce(sim_metric** parts, const unsigned int nel_parts,
		const int myrank, const unsigned int np, const unsigned int nel,
		max_info * maxinf, const int *mask)
{
	const unsigned int q = nel/np;
	const unsigned int r = nel%np;
	const unsigned int local_nel = get_nel_by_rank(myrank, q, r, np);
	unsigned int index_part = 0;
	sim_metric max = 0;
	unsigned int c1, c2;
        
        // prima sotto-matrice triangolare
        findlink_tri(parts[0], get_mask_part_tri(mask, myrank), local_nel, &c1, &c2,
                        &max);
        // successive sotto-matrici quadrate
        //#pragma omp parallel for num_threads(omp_get_num_threads()) schedule(dynamic)
        for(unsigned int i = 1; i < nel_parts + 1; i++){
            // rango della seconda coordinata
            int rank2 = (myrank + i) % np;
            unsigned int tmp_c1, tmp_c2;
            sim_metric tmp_max;
            if(myrank < rank2){
                    findlink_quad(parts[i], get_mask_row_quad(mask, myrank, i ,np),
                                    local_nel, get_mask_col_quad(mask, myrank, i ,np),
                                    get_nel_by_rank(rank2, q, r, np), &tmp_c1, &tmp_c2,
                                    &tmp_max);
            }else{
                    findlink_quad(parts[i], get_mask_row_quad(mask, myrank, i ,np),
                                    get_nel_by_rank(rank2, q, r, np),
                                    get_mask_col_quad(mask, myrank, i ,np), local_nel, &tmp_c1,
                                    &tmp_c2, &tmp_max);
            }

            //#pragma omp critical
            if(tmp_max > max){
                    max = tmp_max;
                    index_part = i;
                    c1 = tmp_c1;
                    c2 = tmp_c2;
            }
        }
        
	maxinf->rank = myrank;
	maxinf->val = max;
	maxinf->row = c1;
	maxinf->col = c2;
	maxinf->index_part = index_part;
}

void print_matrix_parts(sim_metric** parts, const unsigned int nel_parts, const int myrank, 
	const unsigned int np, const unsigned int nel)
{
	const unsigned int q = nel/np;
	const unsigned int r = nel%np;
	const unsigned int local_nel = get_nel_by_rank(myrank, q, r, np);
	// prima sotto-matrice triangolare
	print_matrix(parts[0], local_nel);
	// successive sotto-matrici quadrate
	for(unsigned int i = 1; i < nel_parts+1; i++){
		// rango della seconda coordinata
		int rank2 = (myrank + i) % np;
		if(myrank < rank2){
			print_submatrix(parts[i], local_nel, get_nel_by_rank(rank2, q, r, np));
		}else{
			print_submatrix(parts[i], get_nel_by_rank(rank2, q, r, np), local_nel);
		}
	}
}

void print_submatrix(const sim_metric* dist, const unsigned int row, const unsigned int col){
	for(unsigned int i=0; i<row; i++){
		for(unsigned int j=0; j<col; j++){
			cout << dist[i*col+j] << ' ';
		}
		cout << endl;
	}
	cout << endl;
}

SIFTs** get_array_desc(unsigned char* array_sift, unsigned int* array_nel, unsigned int nel){
	SIFTs** array_desc = new SIFTs*[nel-1];
	unsigned int begin_desc = 0;
	for(unsigned int i = 0; i < nel-1; i++){
		array_desc[i] = new SIFTs;
		array_desc[i]->nel = array_nel[i+1];
		array_desc[i]->sift_array = &array_sift[begin_desc*SIFT_SIZE];
		begin_desc += array_nel[i+1];
	}
	return array_desc;
}

int* init_matrix_map(const int np){
 	return new int[np*(np-1)/2];
}

void print_matrix_map(const int* map, const unsigned int nel){
	unsigned int k = 0;
	for(unsigned int i=0; i<nel; i++){
		for(unsigned int j=0; j<nel; j++){
			if(i<j){
				cout << map[k] << ' ';
				k++;
			}else{
				cout << "_ ";
			}
		}
		cout << endl;
	}
}

void destroy_matrix_map(const int* map){
	delete [] map;
}

void set_map(int* map, const unsigned int nel, const unsigned int row, 
		const unsigned int col, const int val)
{
	if(col != row){
		if(row > col){
			map[(nel-2)*col - (col-1)*col/2 + row -1] = val;
		}else{
			map[(nel-2)*row - (row-1)*row/2 + col -1] = val;
		} 
	}else{
		cerr << "Errore set_map(i,i)" << endl;
	}
}

int get_map(const int* dist, const unsigned int nel, 
		const unsigned int row, const unsigned int col)
{
	if(row == col){
		return row;
	}
	if(row > col){
		return dist[(nel-2)*col - (col-1)*col/2 + row -1];
	}else{
		return dist[(nel-2)*row - (row-1)*row/2 + col -1];
	}
}

unsigned int get_nel_by_rank(const int rank, const int q, const int r, const int np)
{
	if(rank >= np-r)
		return q+1;
	return q;
}

unsigned int get_nel_by_rank(const int rank, const unsigned int nel, const int np)
{
	return get_nel_by_rank(rank, nel/np, nel%np, np);
}

const char* dest_path(const int rank, const unsigned int index){
	char *filename = new char[FILENAME_MAX_LEN];
	sprintf(filename, "images_%d/img_%d.jpg", rank, index);
//	ostringstream osrank, osindex;
//	osrank << rank;
//	osindex << index;
//	string str = (string(IMG_FOLDER) + string("_") + osrank.str() + string("/img_") + 
//		osindex.str() + string(".jpg"));
	return filename;
}

void copy_local_file(const char* sourcename, const char* destname){
	system((string("cp ")+string(sourcename)+ string(" ") +string(destname)).c_str());
}

int* get_img_map(unsigned int nel, int np){
	// np1 = np - r : numero di terminali con q elementi
	// np2 = r : numero di terminali con q+1 elementi	
	np--; // escludi il master
	int* map = new int[nel]; // mappa immagini su terminali
	int q = nel / np; // quoziente intero
	int r = nel % np;	// resto
	int np1 = np - r; // numero partizioni q
	for(int i = 0; i < q+1; i++){ // cicla sull'offset
		int k = i;
		if(i < q){
			for(int j = 0; j < np; j++, k+=q){ // cicla su le partizioni q
				map[k] = j+1;
			}				
		}else{
			// fai avanzare l'indice all'ultimo elemento della prima partizione q+1
			k = i + q * np1;
		}
		for(int j = np1; j < np; j++, k += q+1){ // cicla su le partizioni q+1
			map[k] = j+1;
		}
	}
	return map;
}

void send_file(const char* filename, int dest)
{	
	ifstream file(filename, ios::in|ios::binary);
	char *buffer;
	unsigned int length;
	// calcola la lunghezza del file
  	file.seekg(0, ios::end);
  	length = file.tellg();
	// alloca il buffer
	buffer = new char[length];
	// resetta il puntatore
  	file.seekg(0, ios::beg);
	// lettura del file
	file.read(buffer,length);
	// spedisci il file
	MPI_Send((void*)buffer, length, MPI_BYTE, dest, dest, MPI_COMM_WORLD);
	file.close();
	// dealloca il buffer
	delete[] buffer;
}

void receive_file(const char* filename, int src, int dest)
{

	MPI_Status state;
	int real_size;
	char* buffer = new char[IMG_MAX_LEN];
	ofstream file(filename, ios::out|ios::binary);
	// ricevi il file
	MPI_Recv(buffer, IMG_MAX_LEN, MPI_BYTE, src, dest, MPI_COMM_WORLD, &state);
	MPI_Get_count(&state, MPI_CHAR, &real_size);
	// salva il file
	file.write(buffer, real_size);
	// chiudi file
	file.close();
	// dealloca buffer
	delete[] buffer;
}

void send_mask(int *mask, int np){
	MPI_Bcast(mask,np,MPI_INT,0,MPI_COMM_WORLD);
}

int* receive_mask(int np){
	int *mask = new int[np];
	MPI_Bcast(mask,np,MPI_INT,0,MPI_COMM_WORLD);
	return mask;
}

void master_print_global_matrix(sim_metric ***matrix_parts,
		int *map, const int nel, const int np){
	MPI_Barrier(MPI_COMM_WORLD);
	const int q = nel/np;
	const int r = nel%np;
        cout << endl;
	for(int row = 0; row < nel; row++){
		for(int col = 0; col < nel; col++){

			if(row >= col){
				cout << " -";
				continue;
			}
			int map_row, map_col;
			int rank;
			matrix_to_map_index(row, col, nel, &map_row, &map_col, np);
			rank = get_map(map, np, map_row, map_col);

			if(rank == 0){
				// locale
				int local_col;
				if(r == 0){
					local_col = col - q * map_col;
					if(map_col == 0){
                                            printf(" %d",get((*matrix_parts)[map_col],
								q, row, local_col));
					}else{
                                            printf(" %d",(*matrix_parts)
								[map_col][row * q + local_col]);
					}
                                }else{
					local_col = col - ((q+1) * map_col - (MIN(map_col,np-r)));
					if(map_col == 0){
                                            printf(" %d",get((*matrix_parts)[map_col],
								map_col < np-r ? q : q+1, row, local_col));
					}else{
                                            printf(" %d",(*matrix_parts)
								[map_col]
								[row * (map_col < np-r ? q : q+1) + local_col]);
					}
				}

			}else{
				// remoto
				sim_metric element;
				MPI_Status state;
				MPI_Recv(&element, sizeof(sim_metric), MPI_BYTE, rank,
						TAG_PRINT, MPI_COMM_WORLD, &state);
                                printf(" %d", element);
			}
		}
                printf("\n");
	}
	cout << endl;
	MPI_Barrier(MPI_COMM_WORLD);
}

void slave_print_global_matrix(const int myrank,
		sim_metric ***matrix_parts, int *map, const int nel,
		const int np){
	MPI_Barrier(MPI_COMM_WORLD);
	const int q = nel / np;
	const int r = nel % np;

	for(int row = 0; row < nel; row++){
		for(int col = 0; col < nel; col++){

			if(row >= col){
				continue;
			}
			int map_row, map_col;
			int rank;

			matrix_to_map_index(row, col, nel, &map_row, &map_col, np);
			rank = get_map(map, np, map_row, map_col);

			// cerca solo i valori nelle sezioni del proprio rank
			if(rank == myrank){

				// il master e' in attesa dell'elemento
				int index_part;
				if(map_row == map_col){
					index_part = 0;
				}else if(map_row == rank && map_col > rank){
					index_part = map_col - rank;
				}else if(map_col == rank && map_row < rank){
					index_part = np - map_col + map_row;
				}else{
					// caso non previsto
					index_part = -1;
					cerr << "Errore slave_print_global_matrix: "
                                                << "indici mappa errati" << endl;
				}

				int local_row;
				int local_col;
				sim_metric val;
				if(r == 0){
					local_col = col - (q * map_col);
					local_row = row - (q * map_row);
					if(index_part == 0){
						val = get((*matrix_parts)[index_part], q, local_row ,local_col);
					}else{
						val = (*matrix_parts)[index_part][local_row * q + local_col];
					}
				}else{
					local_col = col - ((q+1) * map_col - (MIN(map_col,np-r)));
					local_row = row - ((q+1) * map_row - (MIN(map_row,np-r)));

					if(index_part == 0){
						val = get((*matrix_parts)[index_part],
								(map_col < np-r ? q : q+1), local_row ,local_col);
					}else{
						val = (*matrix_parts)
								[index_part]
								[local_row * (map_col < np-r ? q : q+1) + local_col];
					}
				}

				// spedisci singolo valore
				MPI_Send(&val, sizeof(sim_metric), MPI_BYTE, 0,
						TAG_PRINT, MPI_COMM_WORLD);
			}
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
}

void print_global_mask(const int* mask, const int nel, const int np){
	const int q = nel/np;
	const int r = nel%np;
	cout << endl;
	for(int i = 0; i < np; i++){
		const int local_nel = (r == 0) ? q : (i < np-r) ? q : q+1;
		for(int j = 0; j < local_nel ; j++){
                    printf("%d",((mask[i] >> j) & 0x01));
			
		}
		printf(" ");
	}
	printf("\n");
}

void get_global_index(const unsigned int myrank, const unsigned int np, 
        const unsigned int nel, const unsigned int index_part, 
        unsigned int l_row, unsigned int l_col, unsigned int &g_row, 
        unsigned int &g_col)
{
    const int q = nel/np;
    const int r = nel%np;
    const unsigned int np_r = np-r;
    const unsigned int part = myrank + index_part;
    
    // calcolo indici di riga e colonna globali
    
    if(part < np){
        if(myrank < np_r){
            g_row = myrank * q + l_row;
        }else{
            g_row = np_r * q + (myrank - np_r) * (q+1) + l_row;
        }
        if(part  < np_r){
            g_col = part * q + l_col;
        }else{
            g_col = np_r * q + (part - np_r) * (q+1) + l_col;
        }
    }else{
        const unsigned int part_np = part - np;
        if(part_np < np_r){
            g_row = part_np * q + l_row;
        }else{
            g_row = np_r * q + (part_np - np_r) * (q+1) + l_row;
        }
        if(myrank < np_r){
            g_col = myrank * q + l_col;
        }else{
            g_col = np_r * q + (myrank - np_r) * (q+1) + l_col;
        }
    }
        
}
