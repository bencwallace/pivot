pivot: main.o utils.o walk.o Makefile
	g++ -o pivot main.o utils.o walk.o -std=c++17 -lboost_program_options

main.o: main.cpp Makefile
	g++ -c main.cpp -std=c++17

walk.o: walk.cpp Makefile
	g++ -c walk.cpp -std=c++17

utils.o: utils.cpp Makefile
	g++ -c utils.cpp -std=c++17

clean:
	rm -f pivot main.o utils.o

.PHONY: clean
