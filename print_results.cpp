#include "print_results.h"

void print_results(cluster* clusters, string* img_names){
    // apri file
    ofstream outfile(FILENAME);

    if (outfile.is_open()){
        // scrivi header
        outfile << get_header() << endl;
        // TODO scrivi le prestazioni!
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
    // titolo pagina
    gallery << "<h1>Clustering Results</h1>" << endl;
    // scorri i clusters
    for (itm = clusters->begin(); itm != clusters->end(); itm++, i++){
        gallery << "<h2>Cluster " << i << "</h2>" << endl;
        gallery << "<table border=\"1\">" << endl;
                
        unsigned int j = 0;
        list<unsigned int>::iterator itl;
        // scorri la lista
        for(itl = itm->second.begin(); itl != itm->second.end(); itl++, j++){
            if(j % GALLERY_COLS == 0) gallery << "<tr>" << endl;
            
            gallery << "<td><img src=\"" << img_names[(*itl)] 
                    << "\" width=\"200px\"></td>" << endl;
            if(j % GALLERY_COLS == 0 && j != 0) gallery << "</tr>" << endl;
        }
        // chiusura tabella
        while(j % GALLERY_COLS != 0){
            gallery << "<td></td>" << endl;
            j++;
            if(j % GALLERY_COLS == 0) gallery << "</tr>" << endl;
        }
        gallery << "</table>";
    }
    gallery << endl;
    return gallery.str();
}