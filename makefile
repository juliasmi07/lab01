#Makefile
CXX=g++

helloWorld: helloWorld.o
	${CXX} helloWorld.o -o helloWorld

lab01Test: lab01Test.o tddFuncs.o arrayFuncs.o
	${CXX} lab01Test.o tddFuncs.o arrayFuncs.o -o lab01Test

clean:
	
	/bin/rm -f *.o helloWorld lab01Test
