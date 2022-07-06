COMPILER		=	gcc
FLAGS 			= 	-Wall -pedantic -std=c99 -fsanitize=address
DEBUG_COMPILER 	= 	-g
EXECUTABLE		=	pedestrians
OBJECT_FILES 	= 	main.o PeatonesADT.o
QUERIES 		= 	query1.csv query2.csv query3.csv query4.csv

all: $(OBJECT_FILES)
	$(COMPILER) -o $(EXECUTABLE) $(OBJECT_FILES) $(FLAGS)
debug: COMPILER+=$(DEBUG_COMPILER)
debug: all
main.o: main.c
	$(COMPILER) -c main.c ($FLAGS)
PeatonesADT.o: PeatonesADT.c PeatonesADT.h
	$(COMPILER) -c PeatonesADT.c PeatonesADT.h $(FLAGS)
cleanAll: cleanQueries cleanObjects cleanExecutable
cleanQueries:
	rm -rf $(QUERIES)
cleanObjects:
	rm -rf $(OBJECT_FILES)
cleanExecutable:
	rm -rf $(EXECUTABLE)