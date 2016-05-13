all:
	g++ -DUNIT_TEST --std=c++0x stanalone_test.cpp ../protocol/tencent_gdt/tencent_gdt_price.cpp ../utility/cypher.cpp ../utility/random.cpp ../utility/hash.cpp ../utility/url.cpp ../utility/json.cpp ../utility/mttytime.cpp -I ../3rdparty/include/ -I ../common/ -I ../utility/ -I ../ -lcryptopp -DNDEBUG  -o a.out
