
#include <stdlib.h>
#include <random>
#include <array>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <string.h>
#include "../utility/utility.h"

using namespace std;
using namespace adservice::utility::cypher;

#define VERBOSE_DEBUG	1

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
	time(&beginTime);
	for(int i=0;i<times;i++){
		CypherResult128 cypherCookies;
		DecodeResult64 decodeResult64;
		makeCookies(cypherCookies);
		cookiesDecode(cypherCookies,decodeResult64);
	}
	time_t endTime;
	time(&endTime);
	cout<<"time cost:"<<(endTime-beginTime)<<endl;
	cout<<"test cookies pressure end"<<endl;
}

int main(int argc,char** argv){
	cookies_hard_test();
	return 0;
}
