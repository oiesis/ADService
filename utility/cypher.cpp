//
// Created by guoze.lin on 16/1/22.
//

#include "utility.h"
#include <random>
#include <array>
#include <algorithm>
#include <cassert>
#include <cstring>

namespace adservice {
    namespace utility {
        namespace cypher {

            using namespace std;

            /**
             * toHex 将二进制串转换成十六进制字符串
             * input : 输入串
             * size  : 串大小
             * hexResult : 输入的用于存放结果的十六进制字符串
             * @return : 返回结果的首地址
             */
            char_t* toHex(const uchar_t* input,int32_t size,INOUT char* hexResult){
                static char hexMap[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
                for(int i=0,j=0;i<size;i++,j+=2){
                    hexResult[j] = hexMap[input[i]&0x0F];
                    hexResult[j+1] = hexMap[input[i]>>4];
                }
                hexResult[size<<1]='\0';
                return hexResult;
            }

            /**
             * 因为toHex的结果是低地址低字节的,所以不符合人类阅读习惯
             * 使用toHexReadable将得到可读的十六进制字符串
             */
            char_t* toHexReadable(const uchar_t* input ,int32_t size,INOUT char* hexResult){
                toHex(input,size,hexResult);
                uint16_t* _hexResult = (uint16_t*)hexResult;
                for(int i=0,j=size-1;i<j;i++,j--){
                    _hexResult[i]^=_hexResult[j];
                    _hexResult[j]^=_hexResult[i];
                    _hexResult[i]^=_hexResult[j];
                }
                for(int i=(size<<1)-1;i>0;i-=2){
                    hexResult[i]^=hexResult[i-1];
                    hexResult[i-1]^=hexResult[i];
                    hexResult[i]^=hexResult[i-1];
                }
                return hexResult;
            }


            /**
             * 将toHex产生的结果转为可读字符串
             */
            char_t* hexToReadable(const char_t* hexString,int32_t size,INOUT char_t* readableResult){
                uint16_t *in = (uint16_t*)hexString;
                uint16_t *out= (uint16_t*)readableResult;
                size>>=1;
                for(int i=0,j=size-1;i<size&&j>=0;i++,j--){
                    out[j] = in[i];
                }
                size<<=1;
                for(int i=size-1;i>0;i-=2){
                    readableResult[i]^=readableResult[i-1];
                    readableResult[i-1]^=readableResult[i];
                    readableResult[i]^=readableResult[i-1];
                }
                readableResult[size]='\0';
                return readableResult;
            }

            /**
             * fromHex: 从十六进制串得到二进制串
             * hexInput : 十六进制字符串,从toHex得到的结果
             * size : 串大小
             * result : 输入的用于存放结果的二进制串
             * @return : 返回结果的首地址
             */
            uchar_t* fromHex(const char* hexInput,int32_t size,INOUT uchar_t* result){
                static uchar_t mask[2] = {0x0F,0xF0};
                for(int i=0;i<size;i++){
                    int j = i >> 1;
                    if(!(i&0x01))
                        result[j]=0;
                    if(hexInput[i]<'A'){
                        result[j]|=((hexInput[i]-'0')<<((i&0x01)?4:0))&mask[i&0x01];
                    }else{
                        result[j]|=((hexInput[i]-'A'+10)<<((i&0x01)?4:0))&mask[i&0x01];
                    }
                }
                return result;
            }


            CypherMapGenerator::CypherMapGenerator(bool isInit){
                if(!isInit){
                    return;
                }
                array<char,64> randomString = randomCharSequence();
                for(int i=0;i<4;i++) {
                    for (int j = 0; j < 16; j++) {
                        cypherMap[i][j] = randomString[(i << 4) + j];
                    }
                }
                regenerate();
            }

            /**
             * randomCharSequence: 生成固定字符集的随机字符顺序
             * @return : 结果
             */
            array<char,64> CypherMapGenerator::randomCharSequence(){
                array<char,64> keys= {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
                                      'A','B','C','D','M','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','X','Y','Z',
                                      '=','+','-','[',']','#','@','_','[','_','@','|'};
                std::shuffle(keys.begin(),keys.end(),std::random_device());
                return keys;
            }

            /**
             * 生成密码表索引
             */
            void CypherMapGenerator::regenerate(){
                for(int i=0;i<4;i++){
                    for(int j=0;j<16;j++){
                        indexMap[i][j] = j;
                    }
                }
                InnerComparetor comp(this);
                for(int i=0;i<4;i++){
                    current = indexMap[i];
                    currentCypher = cypherMap[i];
                    std::sort(current,current+16,comp);
                }
                for(int i=0;i<4;i++){
                    currentCypher = cypherMap[i];
                    current = indexMap[i];
                    for(int j=0;j<16;j++){
                        cypherSortMap[i][j]=currentCypher[current[j]];
                    }
                }
                for(int i=0;i<4;i++){
                    currentCypher = cypherMap[i];
                    current = indexMap[i];
                    for(int j=0;j<16;j++){
                        cypherPosMap[i][j] = current[j];
                    }
                }
            }

            void CypherMapGenerator::print(){
                printf("cypherMap:{");
                for(int i=0;i<4;i++){
                    printf("{");
                    for(int j=0;j<16;j++){
                        printf(" %c,",cypherMap[i][j]);
                    }
                    printf("},\n");
                }
                printf("}\n");
                printf("cypherSortMap={");
                for(int i=0;i<4;i++){
                    printf("{");
                    for(int j=0;j<16;j++){
                        printf(" %c,",cypherSortMap[i][j]);
                    }
                    printf("},\n");
                }
                printf("}\n");

                printf("cypherPosMap:{");
                for(int i=0;i<4;i++){
                    printf("{");
                    for(int j=0;j<16;j++){
                        printf(" %d,",cypherPosMap[i][j]);
                    }
                    printf("},\n");
                }
            }



            static const char cypherMap[4][16] = {{'e','S','U','s','K','n','M','O','[','C','l','-','Q','c','E','b'},
                                                  {'u','v','z','X','f','R','x','Y','V','+','_','@','M','L','B','m'},
                                                  {']','w','T','#','@','a','k','I','d','j','G','J','Z','q','N','o'},
                                                  {'[','A','p','t','=','F','P','r','|','_','H','i','g','y','h','D'}};
            static const char cypherSortMap[4][16] = {{'-','C','E','K','M','O','Q','S','U','[','b','c','e','l','n','s'},
                                              {'+','@','B','L','M','R','V','X','Y','_','f','m','u','v','x','z'},
                                              {'#','@','G','I','J','N','T','Z',']','a','d','j','k','o','q','w'},
                                              {'=','A','D','F','H','P','[','_','g','h','i','p','r','t','y','|'}};
            static const int cypherPosMap[4][16] = {{11,9,14,4,6,7,12,1,2,8,15,13,0,10,5,3},
                                                    {9,11,14,13,12,5,8,3,7,10,4,15,0,1,6,2},
                                                    {3,4,10,7,11,14,2,12,0,5,8,9,6,15,13,1},
                                                    {4,1,15,5,10,6,0,9,12,14,11,2,7,3,13,8}};
            template<typename T>
            int bSearch(T* array,int size,T key){
                int l=0,h=size-1;
                while(l<=h){
                    int mid = l+((h-l)>>1);
                    if(key<=array[mid])
                        h = mid-1;
                    else
                        l = mid+1;
                }
                assert(l>=0&&l<size&&array[l]==key);
                return l;
            }

            /**
            * 将给定字符串进行加密,字符串不超过8字节
            * @param input
            * @param size: input的大小
            * @param result:用于存放结果
            */
            void cookiesEncode(const uchar_t *input, const int32_t size,INOUT CypherResult128 & result) {
                assert(size<=8);
                uchar_t* output = result.bytes;
                for(int i=0,j=0;i<size;i++){
                    output[j] = cypherMap[j&0x03][input[i]&0x0F];
                    j++;
                    output[j] = cypherMap[j&0x03][(input[i]>>4)];
                    j++;
                }
                return ;
            }

            /**
            * 将给定加密字符串进行解密
            * @param input
            * @param output : 输出结果
            * @param size: 存放结果大小
            */
            void cookiesDecode(const CypherResult128 & input,INOUT uchar_t*  output,INOUT int32_t & size){
                assert(size >= 8);
                static uchar_t mask[2] = {0x0F,0xF0};
                const uchar_t* bytes = input.bytes;
                memset(output,0,8);
                for(int i=0;i<16;i++){
                    int j=i>>1;
                    int idx = bSearch<const char>(cypherSortMap[i&0x03],16,bytes[i]);
                    output[j]|=(cypherPosMap[i&0x03][idx]<<((i&0x01)?4:0))&mask[i&0x01];
                }
                size = 8;
                return;
            }


            /**
            * 生成一个明文cookies串
            * @param result:用于存放结果
            * @param size:进入时指定result缓存的大小,返回时指出结果的长度,以字节为单位
            */
            void makeCookiesPublic(INOUT char_t* result,INOUT int32_t & size){
                static std::random_device rd;
                static std::mt19937 randomGenerator(rd());
                union{
                    int value;
                    uchar_t bytes[4];
                } key1,key2;
                key1.value = time::getCurrentTimeSinceMtty();
                key2.value = randomGenerator();
                uchar_t midResult[8];
                uchar_t* p = midResult;
                for(int i=0;i<4;i++){
                    p[i]=key1.bytes[i];
                }
                p+=4;
                for(int i=0;i<4;i++){
                    p[i]=key2.bytes[i];
                }
                toHex(midResult,8,result);
                size = 16;
            }

            /**
             * 生成一个加密的cookies串
             * @param result:用于存放结果
             * @param size:进入时指定result缓存的大小,返回时指出结果的长度,以字节为单位
             */
            void makeCookies(INOUT CypherResult128& result){
                static std::random_device rd;
                static std::mt19937 randomGenerator(rd());
                union{
                    int value;
                    uchar_t bytes[4];
                } key1,key2;
                key1.value = time::getCurrentTimeSinceMtty();
                key2.value = randomGenerator();
#ifdef VERBOSE_DEBUG
                printf("create cookies time:%p random:%p\n",key1.value,key2.value);
#endif
                uchar_t midResult[8];
                uchar_t* p = (uchar_t*)midResult;
                for(int i=0;i<4;i++){
                    p[i]=key1.bytes[i];
                }
                p+=4;
                for(int i=0;i<4;i++){
                    p[i]=key2.bytes[i];
                }
                cookiesEncode(midResult,8,result);
            }

        }
    }
}
