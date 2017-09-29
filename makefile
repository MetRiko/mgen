INC = -ID:\Dev\CORE\boost_1_65_1
LIB = -LD:\Dev\CORE\boost_1_65_1\stage\lib -lboost_filesystem-mgw61-mt-1_65_1 -lboost_system-mgw61-mt-1_65_1

all:
	g++ -c main.cpp $(INC)
	g++ -o mgen.exe main.o $(LIB)
