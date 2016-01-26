
#include <stdlib.h>
#include <random>
#include <array>
#include <algorithm>
#include <iostream>
#include "../utility/utility.h"

using namespace std;
using namespace adservice::utility::cypher;

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

void coder_test(){

}

int main(int argc,char** argv){

	return 0;
}
