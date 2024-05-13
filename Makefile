CXX := clang++
CXXFLAGS := -std=c++17 -stdlib=libstdc++ -O3

pivot: main.o utils.o walk.o walk_tree.o Makefile
	$(CXX) $(CXXFLAGS) -o pivot main.o utils.o walk.o walk_tree.o -lboost_program_options -lgvc -lcgraph

main.o: main.cpp Makefile
	$(CXX) $(CXXFLAGS) -c main.cpp -I/usr/include/graphviz

walk.o: walk.h walk.cpp Makefile
	$(CXX) $(CXXFLAGS) -c walk.cpp

walk_tree.o: walk_tree.h walk_tree.cpp Makefile
	$(CXX) $(CXXFLAGS) -c walk_tree.cpp -I/usr/include/graphviz

utils.o: utils.h utils.cpp Makefile
	$(CXX) $(CXXFLAGS) -c utils.cpp

clean:
	rm -f pivot main.o utils.o walk.o

.PHONY: clean
