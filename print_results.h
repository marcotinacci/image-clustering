#ifndef __PRINT_RESULTS_H__
#define	__PRINT_RESULTS_H__

#include <fstream>
#include <sstream>

#include "cluster.h"

#define FILENAME "output.html"
#define GALLERY_COLS 6

using namespace std;

void print_results(cluster* clusters, string* img_names, const double time, 
        const unsigned int np);
string get_header();
string get_footer();
string get_cluster_gallery(cluster* clusters, string* img_names);

#endif	/* PRINT_RESULTS_H */

