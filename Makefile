build:
	g++ -std=c++17 -g src/* -I include -o main.out

run:
	valgrind ./main.out
