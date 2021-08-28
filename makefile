all: res5

res5: Inverse_Matr_parallel.o functions_parallel.o
	g++ Inverse_Matr_parallel.o functions_parallel.o  -lpthread -o res5 
	
Inverse_Matr.o: Inverse_Matr_parallel.cpp
	g++ -c Inverse_Matr_parallel.cpp

functions.o: functions_parallel.cpp
	g++ -c functions_parallel.cpp
	
clean:
	rm -rf *.o res5