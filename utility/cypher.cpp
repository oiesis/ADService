//
// Created by guoze.lin on 16/1/22.
//

#include "utility.h"
#include <random>
#include <array>
#include <algorithm>
#include <cassert>

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
            char* toHex(const uchar_t* input,int32_t size,INOUT char* hexResult){
                static char hexMap[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
                for(int i=0,j=0;i<size;i++,j+=2){
                    hexResult[j] = hexMap[input[i]&0x0F];
                    hexResult[j+1] = hexMap[input[i]>>4];
                }
                return hexResult;
            }

            /**
             * fromHex: 从十六进制串得到二进制串
             * hexInput : 十六进制字符串
             * size : 串大小
             * result : 输入的用于存放结果的二进制串
             * @return : 返回结果的首地址
             */
            uchar_t* fromHex(const char* hexInput,int32_t size,INOUT uchar_t* result){
                static uchar_t mask[2] = {0x0F,0xF0};
                for(int i=0;i<size;i++){
                    int j = i >> 1;
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
                while(l<h){
                    int mid = l+((h-l)>>1);
                    if(key<array[mid])
                        h = mid-1;
                    else
                        l = mid+1;
                }
                return l;
            }

            void cookiesEncode(const uchar_t *input, const int32_t size,CypherResult128 & result) {
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

            void cookiesDecode(const CypherResult128 & input,uchar_t* & output,int32_t & size){
                assert(size >= 8);
                static uchar_t mask[2] = {0x0F,0xF0};
                const uchar_t* bytes = input.bytes;
                for(int i=0;i<16;i++){
                    int j=i>>1;
                    int idx = bSearch<const char>(cypherSortMap[i&0x03],16,bytes[i]);
                    assert(idx>=0 && idx<16);
                    output[j]|=(cypherPosMap[i&0x03][idx]<<((i&0x01)?4:0))&mask[i&0x01];
                }
                size = 8;
                return;
            }

        }
    }
}
