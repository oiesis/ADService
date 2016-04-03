all:
	g++ -DUNIT_TEST --std=c++0x stanalone_test.cpp ../utility/json.cpp -I ../3rdparty/include/ -I ../common/ -I ../utility/ -I ../ -o a.out
