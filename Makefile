build:
	g++ -std=c++17 src/* -I include -o main.out

run:
	valgrind ./main.out
