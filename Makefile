# sift cluster makefile

CC=mpic++
CFLAGS= -c -Wall -O6 -fopenmp
# lista dei file sorgenti
SOURCES=main.cpp siftget.cpp siftcmp.cpp merge.cpp findlink.cpp distance_matrix.cpp cluster.cpp clusterize.cpp file_desc.cpp tests.cpp img2key.cpp mpi_routines.cpp mask.cpp print_results.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=app

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(OBJECTS) -fopenmp -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *~ *.bak $(EXECUTABLE)
