all:
	g++ -DUNIT_TEST -DMUDUO_STD_STRING --std=c++0x stanalone_test.cpp ../core_src/core_ip_manager.cpp ../utility/cypher.cpp ../utility/random.cpp ../utility/hash.cpp ../utility/url.cpp ../utility/json.cpp ../utility/mttytime.cpp -I ../3rdparty/include/ -I ../common/ -I ../utility/ -I ../ -lcryptopp -ltbb -DNDEBUG  -o a.out
