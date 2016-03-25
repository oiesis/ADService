all:
	g++ --std=c++0x stanalone_test.cpp ../utility/cypher.cpp -I ../3rdparty/include/ -I ../common/ -I ../utility/ -I ../ -o a.out
