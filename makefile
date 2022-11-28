all:
	g++ -std=c++11 -O3 -Wall -fmessage-length=0 -o nogo nogo.cpp MCTS.cpp -fopenmp 
clean:
	rm nogo