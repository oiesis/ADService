#include "../common/types.h"
#include <stdlib.h>
#include <random>
#include <array>
#include <algorithm>
#include <iostream>

using namespace std;

static const char cypherMap[4][16] = {{'e','S','U','s','K','n','M','O','[','C','l','-','Q','c','E','b'},
									  {'u','v','z','X','f','R','x','Y','V','+','_','@','M','L','B','m'},
									  {']','w','T','#','@','a','k','I','d','j','G','J','Z','q','N','o'},
									  {'[','A','p','t','=','F','P','r','|','_','H','i','g','y','h','D'}};
int indexMap[4][16];
int* current;
const char* currentCypher;

int cmp(const void* a,const void* b){
	return currentCypher[*(const int*)a] - currentCypher[*(const int*)b];
}

int main(int argc,char** argv){

	for(int i=0;i<4;i++){
		for(int j=0;j<16;j++){
			indexMap[i][j] = j;
		}
	}
	for(int i=0;i<4;i++){
		current = indexMap[i];
		currentCypher = cypherMap[i];
		qsort(current,16,sizeof(int),cmp);
	}
	printf("remap[4][16] = {");
	for(int i=0;i<4;i++){
		currentCypher = cypherMap[i];
		current = indexMap[i];
		printf("{");
		for(int j=0;j<16;j++){
			printf("\'%c\',",currentCypher[current[j]]);
		}
		printf("},\n");
	}
	printf("}\n");
	printf("posmap[4][16] = {");
	for(int i=0;i<4;i++){
		currentCypher = cypherMap[i];
		current = indexMap[i];
		printf("{");
		for(int i=0;i<16;i++){
			printf("%d,",current[i]);
		}
		printf("},\n");
	}
	return 0;
}
