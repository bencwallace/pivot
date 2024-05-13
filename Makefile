pivot: main.cpp
	g++ -o pivot main.cpp -std=c++17 -pthread -lboost_program_options

clean:
	rm -f pivot

.PHONY: clean
