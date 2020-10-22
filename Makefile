all: main.o utilityFunctions.o
	g++ main.o utilityFunctions.o -o executable
main.o: main.c
	g++ -c main.c
utilityFunctions.o: utilityFunctions.c utilityFunctions.h
	g++ -c utilityFunctions.c
clean:
	rm *.o executable
