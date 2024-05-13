CXX := clang++
CXXFLAGS := -std=c++17 -stdlib=libstdc++ -O3

pivot: main.o utils.o walk.o Makefile
	$(CXX) $(CXXFLAGS) -o pivot main.o utils.o walk.o -lboost_program_options

main.o: main.cpp Makefile
	$(CXX) $(CXXFLAGS) -c main.cpp

walk.o: walk.h walk.cpp Makefile
	$(CXX) $(CXXFLAGS) -c walk.cpp

utils.o: utils.h utils.cpp Makefile
	$(CXX) $(CXXFLAGS) -c utils.cpp

clean:
	rm -f pivot main.o utils.o

.PHONY: clean
