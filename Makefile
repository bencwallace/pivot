CXX := clang++
CXXFLAGS := -std=c++17 -stdlib=libstdc++ -O3

test: test.o utils.o walk_tree.o Makefile
	$(CXX) $(CXXFLAGS) -o test test.o utils.o walk_tree.o -lgvc -lcgraph

pivot: main.o utils.o walk.o Makefile
	$(CXX) $(CXXFLAGS) -o pivot main.o utils.o walk.o -lboost_program_options

test.o: test.cpp Makefile
	$(CXX) $(CXXFLAGS) -c test.cpp -I/usr/include/graphviz

main.o: main.cpp Makefile
	$(CXX) $(CXXFLAGS) -c main.cpp

walk.o: walk.h walk.cpp Makefile
	$(CXX) $(CXXFLAGS) -c walk.cpp

walk_tree.o: walk_tree.h walk_tree.cpp Makefile
	$(CXX) $(CXXFLAGS) -c walk_tree.cpp -I/usr/include/graphviz

utils.o: utils.h utils.cpp Makefile
	$(CXX) $(CXXFLAGS) -c utils.cpp

clean:
	rm -f test pivot test.o main.o utils.o walk.o

.PHONY: clean
