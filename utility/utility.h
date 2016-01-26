//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_UTILITY_H
#define ADCORE_UTILITY_H

#include "../common/types.h"
#include <array>
#include <ctime>


namespace adservice{
   namespace utility{
       namespace hash{

           /** hash functions **/

           uint64_t sax_hash(const void *key, int32_t len);

           uint64_t fnv_hash(const void *key, int32_t len);

           uint64_t oat_hash(const void *key, int32_t len);
       }

       namespace cypher{

           /**
             * 密码表生成器
             */
           class CypherMapGenerator{
           public:
               typedef const char (*CypherMapArrayPointer)[16];
               typedef const int (*CypherMapIndexArrayPointer)[16];

               CypherMapGenerator(bool isInit);

               std::array<char,64> randomCharSequence();

               void regenerate();

               void print();

               inline void setCypherMap( CypherMapArrayPointer array){
                   for(int i=0;i<4;i++){
                       for(int j=0;j<16;j++){
                           cypherMap[i][j] = array[i][j];
                       }
                   }
               }

               inline CypherMapArrayPointer getCypherMap() const{
                   return (CypherMapArrayPointer)cypherMap;
               }

               inline CypherMapArrayPointer getCypherSortMap() const{
                   return (CypherMapArrayPointer)cypherSortMap;
               }

               inline CypherMapIndexArrayPointer getCypherPosMap() const{
                   return (CypherMapIndexArrayPointer)cypherPosMap;
               }

               class InnerComparetor{
               public:
                   InnerComparetor(CypherMapGenerator * g):generator(g){}
                   bool operator()(const int& a,const int& b){
                       return generator->currentCypher[a]<generator->currentCypher[b];
                   }
               private:
                   CypherMapGenerator* generator;
               };
           private:
               char cypherMap[4][16];
               char cypherSortMap[4][16];
               int cypherPosMap[4][16];
               int indexMap[4][16];
               char* currentCypher;
               int* current;
           };

           typedef union {
               uchar_t bytes[16];
               int32_t words[4];
               int64_t dwords[2];
           } CypherResult128;

           void cookiesEncode(const uchar_t * input,const int32_t size,INOUT CypherResult128& result);

           void cookiesDecode(const CypherResult128& input,uchar_t* & output,int32_t& size);

       }

       namespace time{

           extern long getMttyTimeBegin();

           extern long getCurrentTimeStamp();

           extern int getCurrentTimeSinceMtty();
	   
	   extern long mttyTimeBegin();
       }
   }
}
 





#endif
