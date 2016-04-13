
all:
	g++ -c -O2 -DUNIT_TEST --std=c++11 locallog_collector.cpp ../protocol/log/logstring.cpp -I /usr/include -I ../3rdparty/include/ -I ../common/ -I ../utility/ -I ../core_src/ -I ../
	
link:
	g++ -O2 --std=c++11 *.o -rdynamic /home/guoze.lin/Workspace/MacWorkspace/guoze.lin/ADCore/3rdparty/lib/libmuduo_base_cpp11.a -L../3rdparty/lib -ldl -lboost_serialization -lavrocpp -lonsclient4cpp -lpthread -lrt -lrdkafka++ -lcrypto -lcryptopp -Wl,-rpath,/home/guoze.lin/Workspace/MacWorkspace/guoze.lin/ADCore/3rdparty/lib/
