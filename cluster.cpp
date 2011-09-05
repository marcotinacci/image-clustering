#include "cluster.h"

cluster* init_cluster(){
 	return new cluster();
}

void destroy_cluster(cluster* clusters){
	clusters->clear();
	delete clusters;
}

void print_clusters(cluster* clusters){
	cout << "clusters:" << clusters->size() << endl;
	cluster::iterator itm;
	for (itm = clusters->begin(); itm != clusters->end(); itm++){
		cout << "- elements:";
		list<unsigned int>::iterator itl;
		for(itl = itm->second.begin(); itl != itm->second.end(); itl++){
	  		cout << " " << (*itl);
		}
		cout << endl;
	}
	cout << endl;
}