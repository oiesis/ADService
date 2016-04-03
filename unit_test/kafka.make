
THIRD_LIB_PATH:=$(shell pwd)
THIRD_LIB_PATH:=$(THIRD_LIB_PATH)/../3rdparty/lib/


kafka_checker:
	g++ --std=c++0x kafka_checker.cpp ../utility/mttytime.cpp ../utility/url.cpp ../utility/cypher.cpp ../protocol/log/logstring.cpp -I ../3rdparty/include/ -I ../utility/ -I ../common/ -I ../ -lrt -lpthread -L$(THIRD_LIB_PATH) -lrdkafka -lavrocpp -Wl,-rpath,$(THIRD_LIB_PATH) -o kafka_checker

clean:
	rm kafka_checker
