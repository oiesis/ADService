all:
	g++ -DUNIT_TEST --std=c++0x stanalone_test.cpp ../utility/hash.cpp ../utility/url.cpp ../utility/json.cpp ../utility/mttytime.cpp -I ../3rdparty/include/ -I ../common/ -I ../utility/ -I ../ -DNDEBUG  -o a.out
