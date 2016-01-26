//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_UTILITY_H
#define ADCORE_UTILITY_H

#include "../common/types.h"
#include <array>
#include <ctime>
#include <random>

namespace adservice{
   namespace utility{
       namespace hash{

           /** hash functions **/

           uint64_t sax_hash(const void *key, int32_t len);

           uint64_t fnv_hash(const void *key, int32_t len);

           uint64_t oat_hash(const void *key, int32_t len);
       }

       namespace rng{
           class MTTYRandom{
           private:
               static std::mt19937 generator;
           public:
               MTTYRandom(){
                   generator.seed(std::random_device()());
               }
               int32_t get(){
                   return generator();
               }
           };
       }

       namespace cypher{

           char* toHex(const uchar_t* input,int32_t size,INOUT char_t* hexResult);

           char* toHexReadable(const uchar_t* input ,int32_t size,INOUT char_t* hexResult);

           char* hexToReadable(const char_t* hexString,int32_t size,INOUT char_t* readableResult);

           uchar_t* fromHex(const char_t* hexInput,int32_t size,INOUT uchar_t* result);

           /**
             * 密码表生成器,用于离线计算密码表
             */
           class CypherMapGenerator{
           public:
               typedef const char (*CypherMapArrayPointer)[16];
               typedef const int (*CypherMapIndexArrayPointer)[16];

               /**
                * 构造器
                * @param isInit:是否需要自动初始化,自动初始化将自动生成结果
                */
               CypherMapGenerator(bool isInit);

               /**
                * 使用给定字符集产生一个乱码表
                */
               std::array<char,64> randomCharSequence();

               /**
                * 生成码表的索引
                */
               void regenerate();

               /**
                * 输出结果
                */
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

           typedef union {
               uchar_t bytes[8];
               int32_t words[2];
               int64_t dword;
           } DecodeResult64;

           /**
            * 将给定字符串进行加密,字符串不超过8字节
            * @param input
            * @param size: input的大小
            * @param result:用于存放结果
            */
           void cookiesEncode(const uchar_t * input,const int32_t size,INOUT CypherResult128& result);

           /**
            * 将给定加密字符串进行解密
            * @param input
            * @param output : 输出结果
            * @param size: 存放结果大小
            */
           void cookiesDecode(const CypherResult128& input,INOUT uchar_t* output,INOUT int32_t& size);

           /**
            * 将给定加密字符串进行解密
            * @param input
            * @param output : 输出结果
            */
           static inline void cookiesDecode(const CypherResult128& input,INOUT DecodeResult64& output){
               int size = 8;
               cookiesDecode(input,output.bytes,size);
           }

           /**
            * 生成一个明文cookies串
            * @param result:用于存放结果
            * @param size:进入时指定result缓存的大小,返回时指出结果的长度,以字节为单位
            */
           void makeCookiesPublic(INOUT char_t* result,INOUT int32_t & size);

           /**
            * 生成一个加密的cookies串
            * @param result:用于存放结果
            */
           void makeCookies(INOUT CypherResult128& result);

       }

       namespace time{

           static constexpr long MTTY_SERVICE_TIME_BEGIN = 1443669071L;//mttyTimeBegin();

           static inline long getMttyTimeBegin(){
               return MTTY_SERVICE_TIME_BEGIN;
           }

           static inline long getCurrentTimeStamp(){
               time_t currentTime;
               ::time(&currentTime);
               return (long)currentTime;
           }

           static inline int getCurrentTimeSinceMtty(){
               long currentTime = getCurrentTimeStamp();
               return (int)(currentTime - MTTY_SERVICE_TIME_BEGIN);
           }

           /**
            * 用于离线计算mtty基准时间
            */
           long mttyTimeBegin();

           /**
            * 获取当前时间偏离当日零点的偏移秒数
            */
           int getTimeSecondOfToday();

       }
   }
}
 





#endif
