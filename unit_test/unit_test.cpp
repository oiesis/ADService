
#include <stdlib.h>
#include <random>
#include <array>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <string.h>
#include "utility/utility.h"
#include "common/types.h"

using namespace std;
using namespace adservice::utility::hash;
using namespace adservice::utility::time;
using namespace adservice::utility::cypher;
using namespace adservice::utility::json;
using namespace adservice::utility::serialize;
using namespace adservice::utility::url;
using namespace adservice::utility::escape;

#define VERBOSE_DEBUG	1
#define UNIT_TEST	1

void cypher_generator_tester(){
	const char cypherMap[4][16] = {{'e','S','U','s','K','n','M','O','[','C','l','-','Q','c','E','b'},
								   {'u','v','z','X','f','R','x','Y','V','+','_','@','M','L','B','m'},
								   {']','w','T','#','@','a','k','I','d','j','G','J','Z','q','N','o'},
								   {'[','A','p','t','=','F','P','r','|','_','H','i','g','y','h','D'}};
	cout<<"test cypher generator:"<<endl;
	CypherMapGenerator generator(false);
	generator.setCypherMap(cypherMap);
	generator.regenerate();
	generator.print();
	cout<<"test cypher generator end"<<endl;
}

void coder_hex_test(){
	uchar_t bytes[8] = {0x7F,0x56,0x99,0xEF,0x12,0x34,0x56,0x78};
	char_t result[17],result2[17];
	uchar_t origin[8];
	cout<<"test hex string"<<endl;
	toHex(bytes,sizeof(bytes),result);
	cout<<result<<endl;
	fromHex(result,sizeof(bytes)*2,origin);
	for(int i=0;i<8;i++){
		assert(origin[i]==bytes[i]);
	}
	toHexReadable(origin,sizeof(origin),result2);
	cout<<result2<<endl;
	toHexReadable(bytes,sizeof(bytes),result);
	cout<<result<<endl;
	cout<<"test hex string end"<<endl;
}

void printCypherResult(CypherResult128& result){
	char display[33];
	memcpy((void*)display,(void*)result.bytes,16);
	display[16]='\0';
	cout<<display<<endl;
}

void printDecodeResult(DecodeResult64& result){
	printf("time:%p random:%p\n",result.words[0],result.words[1]);
}

void cookies_test(){
	cout<<"test cookies"<<endl;
	char_t result[17],readableResult[35];
	int size=sizeof(result);
	makeCookiesPublic(result,size);
	cout<<result<<endl;
	hexToReadable(result,size,readableResult);
	cout<<readableResult<<endl;
	CypherResult128 cypherCookies;
	makeCookies(cypherCookies);
	printCypherResult(cypherCookies);
	DecodeResult64 decodeResult64;
	cookiesDecode(cypherCookies,decodeResult64);
	cookiesEncode(decodeResult64.bytes,8,cypherCookies);
	printCypherResult(cypherCookies);
	printDecodeResult(decodeResult64);
	cout<<"test cookies end"<<endl;
}

void cookies_hard_test(){
	cout<<"test cookies pressure"<<endl;
	int times=1000000;
	time_t beginTime;
	::time(&beginTime);
	for(int i=0;i<times;i++){
		CypherResult128 cypherCookies;
		DecodeResult64 decodeResult64;
		makeCookies(cypherCookies);
		cookiesDecode(cypherCookies,decodeResult64);
	}
	time_t endTime;
	::time(&endTime);
	cout<<"time cost:"<<(endTime-beginTime)<<endl;
	cout<<"test cookies pressure end"<<endl;
}

void time_util_test(){
	cout<<"time test"<<endl;
	cout<<getMttyTimeBegin()<<endl;
	cout<<getCurrentTimeUtcString()<<endl;
	cout<<getCurrentTimeString()<<endl;
	cout<<"time tets end"<<endl;
}

void whatever_test(){
	cout<<"whatever test"<<endl;
	char_t test[] = "504f5354202f6265"
			"7362696420485454502f312e310d0a48"
			"6f73743a206269642d62616964752e6d"
			"7474792e636f6d0d0a436f6e74656e74"
			"2d4c656e6774683a2034370d0a557365"
			"722d4167656e743a20707974686f6e2d"
			"72657175657374732f322e392e310d0a"
			"436f6e6e656374696f6e3a206b656570"
			"2d616c6976650d0a4163636570743a20"
			"2a2f2a0d0a4163636570742d456e636f"
			"64696e673a20677a69702c206465666c"
			"6174650d0a0d0a0a0831383730343930"
			"37120c3139322e3136382e312e3131a2"
			"011408fe89ef80f0ffffff7f180020ac"
			"0228fa013001";
	int testSize = sizeof(test)-1;
	HexResolver resolver(testSize);
	resolver.resolve(test,testSize);
	resolver.show();
	cout<<"whatever test end"<<endl;
}

void json_test(){
	cout<<"json test"<<endl;
	MessageWraper mw;
	parseJsonFile("../conf/test.conf",mw);
	cout<<"port:"<<mw.getInt("port",8808)<<endl;
	cout<<"isDaemon:"<<mw.getBoolean("isDaemon",true)<<endl;
	cout<<"json test end"<<endl;
}

#include "core.h"

void server_test(){
	launch_service();
}



void url_test(){
	cout<<"url test"<<endl;
	std::string url="http://click.mtty.com/test?s=144105&x=99&r=0490337e-d6ce-11e5-8300-00163e08015d&d=12&t=144105&e=38&c=273&f=http%3A%2F%2Fwww.mtty.com%2Fdemo%2Fpcdd.html&h=000&a=0086-0010-0010&url=http%3A%2F%2Fm.rong360.com%2Fexpress%3Ffrom%3Dsem22%26utm_source%3Dmtwj%26utm_medium%3Dmtwj_dk";
	adservice::types::string url2=url.c_str();
	adservice::types::string output;
	char buffer[1024];
	urlDecode_f(url2,output,buffer);
	cout<<output<<endl;
	time_t beginTime;
	time_t endTime;
	cout<<"origin version:"<<endl;
	::time(&beginTime);
	for(int i=0;i<1000000;i++){
		std::string decodedUrl=urlDecode(url);
	}
	::time(&endTime);
	cout<<"time cost:"<<(endTime-beginTime)<<endl;
	cout<<"modified version:"<<endl;
	::time(&beginTime);
	for(int i=0;i<1000000;i++){
		urlDecode_f(url2,output,buffer);
	}
	::time(&endTime);
	cout<<"time cost:"<<(endTime-beginTime)<<endl; //modified version is 5 times faster than old version
	cout<<"url test end"<<endl;
}

void url_param_test(){
	cout<<"url param test"<<endl;
	std::string url="s=144105&x=99&r=0490337e-d6ce-11e5-8300-00163e08015d&d=12&t=144105&e=38&c=273&f=http%3A%2F%2Fwww.mtty.com%2Fdemo%2Fpcdd.html&h=000&a=0086-0010-0010&url=http%3A%2F%2Fm.rong360.com%2Fexpress%3Ffrom%3Dsem22%26utm_source%3Dmtwj%26utm_medium%3Dmtwj_dk";
	adservice::utility::url::ParamMap paramMap;
	getParam(paramMap,url.c_str());
	typedef typename ParamMap::iterator Iter;
	for(Iter iter = paramMap.begin();iter!=paramMap.end();iter++){
		cout<<iter->first<<":"<<iter->second<<endl;
	}
	time_t beginTime;
	time_t endTime;
	cout<<"getparam pressure test:"<<endl;
	::time(&beginTime);
	for(int i=0;i<1000000;i++){
		paramMap.erase(paramMap.cbegin(),paramMap.cend());
		getParam(paramMap,url.c_str());
	}
	::time(&endTime);
	cout<<"getparam pressure test,cost time:"<<(endTime-beginTime)<<endl;
	cout<<"url param test end"<<endl;
}

void ali_shit_test(){
	cout<<"ali shit test"<<endl;
	uint8_t tmp[56];
	uint8_t* tmpbuf = tmp;
	numberEncode(300,tmpbuf);
	for(uint8_t* p=tmp;p<tmpbuf;p++){
		cout<<(int)*p<<" ";
	}
	cout<<endl;
	uint8_t input[256];
	for(int i=0;i<200;i++){
		input[i]=i;
	}
	input[1]=0;
	input[5]=0;
	input[6]=0;
	input[30]=0;
	input[40]=0;
	input[200]='\0';
	std::string test((char*)input,(char*)(input+200));
	std::string encoded=encode4ali(test);
	std::string decoded=decode4ali(encoded);
	if(test.length()==decoded.length()){
		bool isSame = true;
		for(int i=0;i<test.length();i++){
			if(test[i]!=decoded[i]){
				isSame = false;
				break;
			}
		}
		if(isSame){
			cout<<"decoded string equal origin"<<endl;
		}else{
			cout<<"decoded string not equal origin"<<endl;
		}
	}else{
		cout<<"length not equal"<<endl;
	}
	cout<<"ali shit test end"<<endl;
}

void hash_test(){
	cout<<"begin hash test"<<endl;
	time_t beginTime;
	time_t endTime;
	const char* m = "mm_27818366_4246688_14420528";
	int len = strlen(m);
	cout<<"answer:"<<fnv_hash(m,len)<<endl;
	::time(&beginTime);
	for(int i=0;i<1000000;i++){
		long number = fnv_hash(m,len);
	}
	::time(&endTime);
	cout<<"time elapsed:"<<(endTime-beginTime)<<endl;
	cout<<"end of hash test"<<endl;
}

#include "protocol/baidu/baidu_price.h"
#include "protocol/tanx/tanx_price.h"

void price_decode_test(){
	cout<<"begin price decode test"<<endl;
	const char* baiduPriceStr = "VpM2NwAB_RB7jEpgW5IA8hCvJAhKYEz1mEmAng";
	int baiduPrice=baidu_price_decode(baiduPriceStr);
	cout<<"baiduPrice:"<<baiduPrice<<endl;
	const char* tanxPriceStr = "AQpnIAgAAlbw10bNJgdnB3dTFDSxPXxTxg%3D%3D";
	int tanxPrice = tanx_price_decode(tanxPriceStr);
	cout<<"tanxPrice:"<<tanxPrice<<endl;
	cout<<"end of price decode test"<<endl;
}

void adselect_test(){
	using namespace adservice::adselect;
	using namespace adservice::server;
	cout<<"adselect test"<<endl;
	ConfigManager::init();
	AdSelectManager& manager = AdSelectManager::getInstance();
	rapidjson::Document result;
	rapidjson::Value& v=manager.queryCreativeById(0,"7",result);
	cout<<v["bgid"].GetInt()<<" "<<v["ctr"].GetDouble()<<endl;
	AdSelectManager::release();
	ConfigManager::exit();
	cout<<"end of adselect test"<<endl;
}



int main(int argc,char** argv){
	server_test();
	return 0;
}
