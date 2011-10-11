#include "print_results.h"

void print_results(cluster* clusters, string* img_names, const double time, 
        const unsigned int np, const unsigned int nel){
    // apri file
    ofstream outfile(FILENAME);

    if (outfile.is_open()){
        // scrivi header
        outfile << get_header() << endl;
        // titolo pagina
        outfile << "<h1>Clustering Results</h1>" << endl;
        // configurazione
        outfile << "<h2>Configuration</h2>" << endl;
        outfile << "Numero immagini: " << nel << "<br>" << endl;
        outfile << "Numero terminali: " << np << "<br>" << endl;
        outfile << "Tempo impiegato: " << time << " secondi<br>" << endl;
        // scrivi la galleria
        outfile << get_cluster_gallery(clusters,img_names) << endl;
        // scrivi footer
        outfile << get_footer() << endl;
        // chiudi file
        outfile.close();
    }else{
        cerr << "Unable to open results file";
    }
}

string get_header(){
        return "<html><head><title>Clustering Results</title></head><body>";
}

string get_footer(){
        return "</body></html>";
}

string get_cluster_gallery(cluster* clusters, string* img_names){
    stringstream gallery;
    cluster::iterator itm;
    unsigned int i = 0;
    // scorri i clusters
    for (itm = clusters->begin(); itm != clusters->end(); itm++, i++){
        gallery << "<h2>Cluster " << i << "</h2>" << endl;
        gallery << "<table border=\"1\">" << endl;
                
        unsigned int j = 0;
        list<unsigned int>::iterator itl;
        // scorri la lista
        for(itl = itm->second.begin(); itl != itm->second.end(); itl++, j++){
            if(j % GALLERY_COLS == 0){
                if( j != 0 )
                    gallery << "</tr>" << endl;
                gallery << "<tr>" << endl;
            }
            gallery << "<td><img src=\"" << img_names[(*itl)] 
                    << "\" width=\"200px\" alt=\"" << img_names[(*itl)] 
                    << "\"></td>" << endl;
        }
        // chiusura tabella
        while(j % GALLERY_COLS != 0){
            gallery << "<td width=\"200px\"></td>" << endl;
            j++;
        }
        gallery << "</tr></table>";
    }
    gallery << endl;
    return gallery.str();
}