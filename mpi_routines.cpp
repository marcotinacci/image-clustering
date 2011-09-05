#include "mpi_routines.h"

void master_clusterize(const max_info &maxinfo, const int *map, const unsigned int nel, 
	const unsigned int np, sim_metric ***matrix_parts, const unsigned int matrix_nel)
{
	/*
		TODO usare una struttura
	*/
	// info[0]: rank con cui comunicare (rank = np -> terminazione)
	// info[1]: index cluster
	// info[2]: index part
	// info[3]: 0 row, 1 column
	// info[4]: 0 mit, 1 dest
	unsigned int info1[5], info2[5]; // vettori informazioni
	unsigned int i_row, i_col; // coordinate sotto-matrice sulla mappa
	get_map_index(maxinfo.rank, maxinfo.index_part, np, i_row, i_col);
	for(unsigned int i = 0; i < np; i++){
		// elabora i dati da spedire
		info1[0] = get_map(map, np, i_row, i);
		info2[0] = get_map(map, np, i, i_col);
		info1[1] = maxinfo.row;
		info2[1] = maxinfo.col;
		info1[2] = get_index_part(info2[0], np, i, i_col);
		info2[2] = get_index_part(info1[0], np, i_row, i);
		info1[3] = (i_row < i) ? 1 : 0;
		info2[3] = (i < i_col) ? 1 : 0;
		// la cifra specifica il ruolo di chi riceve le info!
		info1[4] = 0;
		info2[4] = 1;
		printf("%d => %d\n", info2[0], info1[0]);

		/*
			TODO usare Isend? gli interleaving sono safe?
		*/
		// gestione comunicazione
		if(info1[0] == 0 && info2[0] == 0){ // master-master
			printf("master-master (todo)\n");
		}else{
			if(info1[0] == 0){ // master-slave
				printf("master-slave\n");
//				printf("0 before send info\n");
				MPI_Send(info1, 5, MPI_UNSIGNED, info2[0], TAG_CLUSTER_1, MPI_COMM_WORLD);
//				printf("0 after send info\n");
				sim_metric* buf = recv_stripe(0, np, nel, info2, matrix_parts);
				// aggiornare elementi locali
				unsigned int row, col, direction;
				get_part_dim(0, np, nel, info2[2], row, col);
				direction = (info2[2] == 0) ? 2 : info2[3];
				update_local_cluster((*matrix_parts)[info2[2]], buf, info2[1], row, col, direction);
			}else if(info2[0] == 0){ // slave-master
				printf("slave-master\n");
//				printf("0 before send info\n");
				MPI_Send(info2, 5, MPI_UNSIGNED, info1[0], TAG_CLUSTER_1, MPI_COMM_WORLD);
//				printf("0 after send info\n");
				send_stripe(0, np, nel, info1, *matrix_parts);
			}else{ // slave-slave
				printf("slave-slave\n");
//				printf("0 before send info1\n");
				MPI_Send(info1, 5, MPI_UNSIGNED, info2[0], TAG_CLUSTER_1, MPI_COMM_WORLD);
//				printf("0 after send info1\n");
//				printf("0 before send info2\n");
				MPI_Send(info2, 5, MPI_UNSIGNED, info1[0], TAG_CLUSTER_1, MPI_COMM_WORLD);
//				printf("0 after send info2\n");
			}
		}
	}
	printf("inizia ciclo di terminazione\n");
	// invia messaggi di terminazione fase (rank = -1)
	for(unsigned int i = 1; i < np; i++){
		info1[0] = np;
		MPI_Send(info1, 5, MPI_UNSIGNED, i, TAG_CLUSTER_1, MPI_COMM_WORLD);
	}
}

void update_local_cluster(sim_metric *part, sim_metric *buf, const unsigned int index_cluster, 
		const unsigned int row, const unsigned int col, const unsigned int direction)
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
//		print_submatrix(part, row, col);
		break;
		case 1: // colonna
		for(unsigned int i = 0; i < dim; i++){
			if(part[index_cluster+col*i] > buf[i]){
				part[index_cluster+col*i] = buf[i];
			}
		}
//		print_submatrix(part, row, col);
		break;
		case 2: // triangolare
		for(unsigned int i = 0; i < dim; i++){
			if(get(part,dim,i,index_cluster) > buf[i]){
				buf[i] = get(part,dim,i,index_cluster);
			}
		}
//		print_matrix(part, dim);
		break;
		default:
			cerr << "Errore: codice direction = " << direction << endl;
		break;
	}
	
}

void send_stripe(const unsigned int myrank, const unsigned int np, const unsigned int nel, unsigned int * info,
		sim_metric** matrix_parts)
{
//	printf("%d (send)info = [%d,%d,%d,%d,%d]\n", myrank, info[0], info[1], info[2], info[3], info[4]);
	unsigned int row, col, dim;
	get_part_dim(myrank, np, nel, info[2], row, col);
	// numero dati da spedire
	// se e' triangolare (info[2] == 0) col e row sono uguali
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
//	printf("%d before send buf\n",myrank);
	MPI_Send(buf, dim * sizeof(sim_metric), MPI_BYTE, info[0], TAG_CLUSTER_2, MPI_COMM_WORLD);
//	printf("%d after send buf\n",myrank);
}

sim_metric * recv_stripe(const unsigned int myrank, const unsigned int np, const unsigned int nel, unsigned int * info,
	sim_metric ***matrix_parts)
{
//	printf("%d (recv)info = [%d,%d,%d,%d,%d]\n", myrank, info[0], info[1], info[2], info[3], info[4]);
	MPI_Status state;
	unsigned int row, col, dim;
	get_part_dim(myrank, np, nel, info[2], row, col);
	// numero dati da ricevere
	dim = (info[3] == 0) ? col : row;
	sim_metric *buf = new sim_metric[dim];
//	printf("%d before recv buf\n",myrank);
	MPI_Recv(buf, dim * sizeof(sim_metric), MPI_BYTE, info[0], TAG_CLUSTER_2, MPI_COMM_WORLD, &state);
//	printf("%d after recv buf\n",myrank);
	// copia gli elementi della sotto-matrice
	if(info[2] == 0){ // triangolo
		for(unsigned int i = 0; i < dim; i++)
			set((*matrix_parts)[info[2]], dim, info[1], i, buf[i]);
	}else if(info[3] == 0){ // riga
		for(unsigned int i = 0; i < dim; i++)
			(*matrix_parts)[info[2]][info[1]*col+i] = buf[i];
	}else if(info[3] == 1){ // colonna
		for(unsigned int i = 0; i < dim; i++)
			(*matrix_parts)[info[2]][col*i+info[1]] = buf[i];
	}else{ // errore
		cerr << "Errore: valore di info[3] = " << info[3] << endl;
	}
	
	// DEBUG
	cout << "recv_buf[ ";
	for(unsigned int i = 0; i < dim; i++)
	{
		cout << buf[i] << ' ';
	}
	cout << "]" << endl;
	return buf;
}

void slave_clusterize(const unsigned int myrank, const unsigned int nel, const unsigned int np, 
	sim_metric ***matrix_parts, const unsigned int nel_parts)
{
	unsigned int info[5];
	MPI_Status state;
	while(true){
		// ricevi rank mittente/destinatario
//		printf("%d before recv info\n",myrank);
		MPI_Recv(info, 5, MPI_UNSIGNED, 0, TAG_CLUSTER_1, MPI_COMM_WORLD, &state);
//		printf("%d after recv info\n",myrank);
		if(info[0] == np){ // terminare fase
			break;
		}else if(myrank == info[0]){ // eseguire l'aggiornamento in locale
			/*
				TODO implementare
			*/
			unsigned int info2[5];
			// seconda ricezione
			MPI_Recv(info2, 5, MPI_UNSIGNED, 0, TAG_CLUSTER_1, MPI_COMM_WORLD, &state);
			printf("slave local %d (todo)\n", myrank);
		
		}else{ // comunicare e aggiornare i cluster
			/*
				TODO scegliere in base al bilanciamento del carico
			*/
			if(info[4] == 1){ // questo slave e' destinatario
				recv_stripe(myrank, np, nel, info, matrix_parts);
			}else{ // questo slave e' mittente
				send_stripe(myrank, np, nel, info, *matrix_parts);
			}
		}
	}
}

unsigned int get_index_part(const int myrank, const unsigned int np, const unsigned int row, 
	const unsigned int col)
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

void get_map_index(const int myrank, const unsigned int index_part, const unsigned int np,
	unsigned int &row, unsigned int &col)
{
	if(myrank + index_part < np){
		row = myrank;
		col = myrank + index_part;
	}else{
		row = (myrank + index_part) % np;
		col = myrank;
	}
}

void get_part_dim(const int myrank, const unsigned int np, const unsigned int nel,
	const unsigned int index_part, unsigned int &row, unsigned int &col)
{
	if(myrank + index_part < np){
		row = get_nel_by_rank(myrank, nel, np);
		col = get_nel_by_rank(myrank + index_part, nel, np);
	}else{
		row = get_nel_by_rank((myrank + index_part) % np, nel, np);
		col = get_nel_by_rank(myrank, nel, np);
	}
}

void master_max_reduce(max_info * local_max, max_info * global_max, const int myrank, const int np){
	max_info *max_vett = new max_info[np];
	/*
		TODO eseguire con MPI_AllReduce MAX_LOC
	*/
	MPI_Gather(local_max, sizeof(max_info), MPI_BYTE, max_vett, sizeof(max_info), MPI_BYTE,
	 	myrank, MPI_COMM_WORLD);

	printf("{ ");
	for(int i = 0; i < np; i++)
	{
		printf("(%d,%d,%d,%d,%d) ",max_vett[i].rank,max_vett[i].val,max_vett[i].row,max_vett[i].col, max_vett[i].index_part);
	}
	printf("}\n");

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
//	printf("local max slave (%d,%d,%d,%d,%d)\n",local_max->rank, local_max->val, local_max->row, local_max->col, local_max->index_part);
	MPI_Gather(local_max, sizeof(max_info), MPI_BYTE, NULL, 0, MPI_DATATYPE_NULL,
	 	0, MPI_COMM_WORLD);
}


void master_scatter_keys(const int myrank, const unsigned int nel, const unsigned int np, 
	string* img_names, unsigned int* local_nel, SIFTs*** desc_array)
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
					/*
						TODO fattorizzare e parallelizzare
					*/						
					char* local_filename = dest_path(0, i);
					// conversione jpg->pgm
					img2pgm(local_filename);
					// conversione pgm->key
					pgm2key(ext_pgm(local_filename));
					// leggi i sifts
					(*desc_array)[i] = get_sifts((string(local_filename) + string(".key")).c_str());
				}else{
					send_file(img_names[k].c_str(), j);
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

void slave_scatter_keys(const int myrank, const unsigned int nel, const unsigned int np, 
		unsigned int *local_nel, SIFTs ***desc_array)
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
	char* temp;
	for(unsigned int i = 0; i < *local_nel; i++){
		char* local_filename = dest_path(myrank, i);
		temp = new char[FILENAME_MAX_LEN];
		receive_file(dest_path(myrank, i), 0, myrank);
		// conversione jpg->pgm
		img2pgm(local_filename);
		// conversione pgm->key
		pgm2key(ext_pgm(local_filename));
		// leggi i sifts
		(*desc_array)[i] = get_sifts((string(local_filename) + suffix).c_str());
	}
}

void master_compute_map(const unsigned int np, int **map, unsigned int *nel_map){
	// inizializza mappa
	*map = init_matrix_map(np);
	// numero elementi mappa
	*nel_map = np*(np-1)/2;
	// calcola mappa
	for(unsigned int i = 0; i < *nel_map; i++){
		set_map(*map, np, i % np, (i + i/np + 1) % np, i % np);
	}
}

void master_compute_matrix(const int myrank, const unsigned int nel, const unsigned int np, 
		const unsigned int local_nel, SIFTs** desc_array, sim_metric ***matrix_parts, unsigned int *nel_parts)
{
	// matrice di assegnamento sezione
	unsigned int num_cycle = (np-1)/2;
	*nel_parts = get_nel_parts(myrank, np);
	sim_metric **mat = new sim_metric*[*nel_parts + 1];
	// elabora la matrice triangolare
	mat[0] = compute_matrix_dist(desc_array,local_nel);
	for(unsigned int i = 0; i < num_cycle; i++){
		// calcola sotto-matrici remote
		for(unsigned int j = 1; j < (unsigned int)np; j++){
			int info[] = {(j+i+1) % np, (j-i-1+np) % np};
			MPI_Send((void*)info, 2, MPI_INT, j, TAG_TRANSFER_INFO, MPI_COMM_WORLD);
		}
		// calcola sotto-matrice locale
		mat[i+1] = mpi_compute_matrix(desc_array, local_nel, nel, np, myrank, (i+1) % np, (np-i-1) % np);
	}
	// se il numero di terminali e' pari fare un altro ciclo di chiusura
	if(np % 2 == 0){
		// calcola sotto-matrici remote
		for(unsigned int j = 0; j < (unsigned int)(np / 2); j++){
			// sola ricezione
			if(j != 0){
				int info1[] = {np/2 +j, -1};
				MPI_Send((void*)info1, 2, MPI_INT, j, TAG_TRANSFER_INFO, MPI_COMM_WORLD);
			}
			// sola trasmissione
			int info2[] = {-1, j};
			MPI_Send((void*)info2, 2, MPI_INT, np/2 + j, TAG_TRANSFER_INFO, MPI_COMM_WORLD);
		}
		// calcola sotto-matrice locale
		mat[*nel_parts] = mpi_compute_matrix(desc_array, local_nel, nel, np, 0, np/2, -1);
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

	/*
		TODO parallelizzare
	*/
	for(unsigned int j = 1; j < send_nel; j++){
		send_array_nel[j] = desc[j-1]->nel;
		sum += desc[j-1]->nel;
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
	
	/*
		TODO calcolare solo una volta fuori dal ciclo
	*/
	// crea vettore globale sift
	unsigned char* send_array_sift = new unsigned char[send_array_nel[0]*SIFT_SIZE];
	unsigned int begin_desc = 0;
	for(unsigned int i=0; i < local_nel; i++){
		for(unsigned int j=0; j < send_array_nel[i+1]; j++){
			for(unsigned int k=0; k<SIFT_SIZE; k++){
				send_array_sift[(begin_desc+j)*SIFT_SIZE+k] = get_sift(desc[i],j)[k];
			}
		}
		begin_desc += send_array_nel[i+1];
	}
	
	unsigned char* recv_array_sift = new unsigned char[recv_array_nel[0]*SIFT_SIZE];
	
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

void local_max_reduce(sim_metric** parts, const unsigned int nel_parts, const int myrank,
	const unsigned int np, const unsigned int nel, max_info * maxinf)
{
	/*
		TODO mantenere i massimi di ogni part memorizzati per velocizzare la ricerca
	*/
	
	const unsigned int q = nel/np;
	const unsigned int r = nel%np;
	const unsigned int local_nel = get_nel_by_rank(myrank, q, r, np);
	unsigned int index_part = 0;
	sim_metric max = 0;
	unsigned int c1, c2;
	// prima sotto-matrice triangolare
	findlink_tri(parts[0], 0xFFFFFFFF, local_nel, &c1, &c2, &max);
	cout << "- reduced matrix" << endl;
	print_matrix(parts[0], local_nel);
//	printf("local max = (%d,%d) index = %d/%d\n", c1, c2, 0, nel_parts);
	// successive sotto-matrici quadrate
	for(unsigned int i = 1; i < nel_parts + 1; i++){
		// rango della seconda coordinata
		int rank2 = (myrank + i) % np;
		unsigned int tmp_c1, tmp_c2;
		sim_metric tmp_max;
		if(myrank < rank2){
//			print_submatrix(parts[i], local_nel, get_nel_by_rank(rank2, q, r, np));
			findlink_quad(parts[i], 0xFFFFFFFF, local_nel, 0xFFFFFFFF, 
				get_nel_by_rank(rank2, q, r, np), &tmp_c1, &tmp_c2, &tmp_max);
//			printf("local max = (%d,%d) index = %d/%d\n", tmp_c1, tmp_c2,i,nel_parts);
		}else{
//			print_submatrix(parts[i], get_nel_by_rank(rank2, q, r, np), local_nel);
			findlink_quad(parts[i], 0xFFFFFFFF, get_nel_by_rank(rank2, q, r, np), 
				0xFFFFFFFF, local_nel, &tmp_c1, &tmp_c2, &tmp_max);
//			printf("local max = (%d,%d) index = %d/%d\n", tmp_c1, tmp_c2,i,nel_parts);
		}
//		printf("temp %d > %d ? \n",tmp_max,max);
		if(tmp_max > max){
			max = tmp_max;
			index_part = i;
			c1 = tmp_c1;
			c2 = tmp_c2;
//			printf("update max = %d, index = %d (%d,%d)\n", max, index_part, c1, c2);
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

/*
	TODO implementare la matrice template per distanze e mappatura
*/

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

char* dest_path(const int rank, const unsigned int index){
	ostringstream osrank, osindex;
	osrank << rank;
	osindex << index;
	string str = (string(IMG_FOLDER) + string("_") + osrank.str() + string("/img_") + 
		osindex.str() + string(".jpg"));
	return (char*)str.c_str();
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
