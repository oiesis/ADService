//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_UTILITY_H
#define ADCORE_UTILITY_H


#include <array>
#include <ctime>
#include <random>
#include <stddef.h>
#include <map>
#include <vector>
#include <iostream>
#include <strings.h>
#include <cstring>
#include <sstream>
#include <tuple>
#include "google/protobuf/message.h"
#include "types.h"
#include "functions.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "protocol/click/click.h"

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

           char_t* toHex(const uchar_t* input,int32_t size,INOUT char_t* hexResult);

           char_t* toHexReadable(const uchar_t* input ,int32_t size,INOUT char_t* hexResult);

           char_t* hexToReadable(const char_t* hexString,int32_t size,INOUT char_t* readableResult);

           uchar_t* fromHex(const char_t* hexInput,int32_t size,INOUT uchar_t* result);

           uchar_t * fromLittleEndiumHex(const char* hexInput,int32_t size,INOUT uchar_t* result,INOUT int32_t & outSize,bool capital=true);

           /**
            * 用于调试输入输出,解析十六进制字符流
            */
           class HexResolver final{
           public:
               explicit HexResolver(int stringSize){
                   resultSize = (stringSize>>1)+2;
                   result = new uchar_t[resultSize];
               }
               void resolve(const char_t* input,int size){
                   fromLittleEndiumHex(input,size,result,resultSize);
               }
               typedef void (*SHOWHANDLER)(uchar_t*,int32_t);

               void show(SHOWHANDLER handler = nullptr){
                   if(handler!= nullptr)
                       handler(result,resultSize);
                   else
                       printResult();
               }
               ~ HexResolver(){
                   if(result!= nullptr)
                       delete[] result;
               }
           private:
               void printResult(){
                   result[resultSize]='\0';
                   printf("%s",result);
               }

               uchar_t* result;
               int resultSize;
           };

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
               char_t char_bytes[20];
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
           inline void cookiesDecode(const CypherResult128& input,INOUT DecodeResult64& output){
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

           //假定服务器在东八区

           static const int64_t MTTY_SERVICE_TIME_BEGIN = 1443669071L;//mttyTimeBegin();
           static const int32_t timeZone = 8;
           static constexpr int32_t UTC_TIME_DIFF_SEC = timeZone * 3600;

           inline int64_t getMttyTimeBegin(){
               return MTTY_SERVICE_TIME_BEGIN;
           }

           inline int64_t getCurrentTimeStamp(){
               time_t currentTime;
               ::time(&currentTime);
               return (int64_t)currentTime;
           }

           inline int32_t getCurrentTimeSinceMtty(){
               long currentTime = getCurrentTimeStamp();
               return (int32_t)(currentTime - MTTY_SERVICE_TIME_BEGIN);
           }

           /**
            * mtty时间到本地Unix时间戳
            */
           inline int64_t mttyTimeToUnixTimeStamp(int32_t mttyTime){
               return mttyTime+MTTY_SERVICE_TIME_BEGIN;
           }

           /**
            * 本地时间戳到UTC时间戳
            */
           inline int64_t localTimeStamptoUtc(int64_t unixTimeStamp){
               return unixTimeStamp - UTC_TIME_DIFF_SEC;
           }

           /**
            * 用于离线计算mtty基准时间
            */
           int64_t mttyTimeBegin();

           /**
            * 获取当前时间偏离当日零点的偏移秒数
            */
           int32_t getTimeSecondOfToday();

           /**
            * 获取当日零点Unix时间戳
            */
           int64_t getTodayStartTime();

       }

       namespace memory{


#ifndef OPTIMIZED_MALLOC

//不作内存分配优化,使用库函数

           inline void* adservice_malloc(size_t size){
               return malloc(size);
           }

           inline void* adservice_calloc(size_t num, size_t size){
               return calloc(num,size);
           }


           inline void* adservice_realloc(void* ptr, size_t size){
               return realloc(ptr,size);
           }

           inline void adservice_free(void* ptr){
               free(ptr);
           }

           template<typename T>
           inline T* adservice_new(size_t num = 0){
               if(num == 0)
                   return new T;
               else
                   return new T[num];
           }

//           template<typename T>
//           inline T* adservice_new(void* placement,size_t num = 0){
//               if(num == 0){
//                   return new(placement) T;
//               }else{
//                   return new(placement) T[num];
//               }
//           }

           template<typename T>
           inline void adservice_delete(T* ptr,size_t num = 0){
                if(num>0){
                   delete[] ptr;
                }else{
                    delete ptr;
                }
           }

#else

           //作内存分配优化,重新实现内存管理函数

void* adservice_malloc(size_t size);
void* adservice_calloc(size_t nmemb,size_t size);
void* adservice_realloc(void *ptr,size_t size);
void adservice_free(void* ptr);

#endif

           inline char* strdup(const char* str){
               size_t size = strlen(str);
               char* ret = (char*)adservice_malloc(size+1);
               memcpy(ret,str,size+1);
               return ret;
           }
       }

       namespace json{

           class MessageWraper{
           public:
               MessageWraper(){
               }
               MessageWraper(std::vector<std::string> keys){
                    for(auto& s : keys){
                        messages[s] = "";
                    }
               }
               bool isFieldEmpty(const std::string& key){
                   std::string& values = messages[key];
                   return values.empty();
               }

               std::map<std::string,std::string>& getMessages(){
                   return messages;
               }

               int32_t getInt(const std::string& key,int32_t defaultValue = 0){
                   std::string& values = messages[key];
                   if(values.empty()){
                       return defaultValue;
                   }
                   int32_t ret = std::stoi(values);
                   return ret;
               }

               double getDouble(const std::string& key,double defaultValue = 0.0){
                   std::string& values = messages[key];
                   if(values.empty()){
                       return defaultValue;
                   }
                   double ret = std::stof(values);
                   return ret;
               }

               bool getBoolean(const std::string& key,bool defaultValue = false){
                   std::string& values = messages[key];
                   if(values.empty()){
                       return defaultValue;
                   }
                   return strcasecmp("true",values.c_str()) == 0;
               }

               const char* getRawString(const std::string& key){
                   std::string& values = messages[key];
                   return values.c_str();
               }

               const std::string& getString(const std::string& key,const std::string& defaultValue){
                   std::string& values = messages[key];
                   if(values.empty()){
                       return defaultValue;
                   }
                   return values;
               }
           private:
               std::map<std::string,std::string> messages;
           };

           class JsonParseHandler
                   : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, JsonParseHandler> {
           public:
               typedef std::map<std::string,std::string> MessageMap;

               JsonParseHandler() : messages_(), state_(kExpectObjectStart), name_() {}

               bool StartObject() {
                   switch (state_) {
                       case kExpectObjectStart:
                           state_ = kExpectNameOrObjectEnd;
                           return true;
                       default:
                           return false;
                   }
               }

               bool String(const char* str, rapidjson::SizeType length, bool) {
                   switch (state_) {
                       case kExpectNameOrObjectEnd:
                           name_ = std::string(str, length);
                           state_ = kExpectValue;
                           return true;
                       case kExpectValue:
                           messages_.insert(MessageMap::value_type(name_, std::string(str, length)));
                           state_ = kExpectNameOrObjectEnd;
                           return true;
                       default:
                           return false;
                   }
               }

               bool EndObject(rapidjson::SizeType) { return state_ == kExpectNameOrObjectEnd; }

               bool Default() { return false; } // All other events are invalid.

               MessageMap& getMessageMap(){
                   return messages_;
               }
           private:
               MessageMap messages_;
               enum State {
                   kExpectObjectStart,
                   kExpectNameOrObjectEnd,
                   kExpectValue
               }state_;
               std::string name_;
           };



           bool parseJson(const char* json,MessageWraper& mw);

           bool parseJson(const char* json,rapidjson::Document& doc);

           bool parseJsonFile(const char* filePath, MessageWraper& mw);

           bool parseJsonFile(const char* filePath, rapidjson::Document & doc);

       }

       namespace serialize{
           //support of protobuf object
           using google::protobuf::Message;


           template<typename T>
           inline T& getProtoBufObject(T& obj,std::stringstream& stream){
               obj.ParseFromIstream(&stream);
               return obj;
           }

           template<typename T>
           inline void writeProtoBufObject(T& obj,std::stringstream& stream){
               obj.SerializeToOstream(&stream);
           }

           /**
            * 本方法从avro字节流中获取对象
            */
           template<typename T>
           inline T& getAvroObject(T& obj,const uint8_t* bytes,int size){
               std::unique_ptr<avro::InputStream> in = avro::memoryInputStream(bytes,size); //需要考虑对象重用
               avro::DecoderPtr decoderPtr = avro::binaryDecoder();
               decoderPtr->init(*in);
               avro::codec_traits<T>::decode(*decoderPtr,obj);
               return obj;
           }

           template<typename T>
           inline std::unique_ptr<avro::OutputStream> writeAvroObject(T& obj){
               std::unique_ptr<avro::OutputStream> out = avro::memoryOutputStream();
               avro::EncoderPtr encoderPtr = avro::binaryEncoder();
               encoderPtr->init(*out);
               avro::codec_traits<T>::encode(*encoderPtr,obj);
               return out;
           }

           /**
            * 本方法从给定对象获取avro二进制串
            */
           template<typename T>
           inline void writeAvroObject(T& obj,adservice::types::string& output){
               std::unique_ptr<avro::OutputStream> out = writeAvroObject(obj);
               std::unique_ptr<avro::InputStream> is = avro::memoryInputStream(*out);
               const uint8_t* data;
               size_t size;
               while(is->next(&data,&size)){
                    output.append((const char*)data,size);
               }
           }

           template<typename T,int BUFFSIZE>
           class AvroObjectReader{
           public:
               AvroObjectReader(){
                   in = avro::memoryInputStream(buffer,BUFFSIZE);
                   decoderPtr = avro::binaryDecoder();
                   decoderPtr->init(*in);
               }
               std::tuple<uint8_t*,int> getBuffer(){
                   std::make_tuple<uint8_t*,int>(buffer,BUFFSIZE);
               }
               void read(T& obj){
                   avro::codec_traits<T>::decode(*decoderPtr,obj);
               }
               void rewind(){
                   //in.release();
                   //in = avro::memoryInputStream(buffer,BUFFSIZE);
                   decoderPtr->init(*in);
               }

           private:
               uint8_t buffer[BUFFSIZE];
               std::unique_ptr<avro::InputStream> in;
               avro::DecoderPtr decoderPtr;
           };

           template<typename T>
           class AvroObjectWriter{
           public:
               typedef std::function<bool(const uint8_t*,int)> WriteDataCallback;
               AvroObjectWriter(){
                   out = avro::memoryOutputStream();
                   encoderPtr = avro::binaryEncoder();
                   encoderPtr->init(*out);
               }
               void write(std::vector<T>& v){
                   using Iter = typename std::vector<T>::iterator;
                   for(Iter iter = v.begin();iter!=v.end();iter++){
                       avro::codec_traits<T>::encode(*encoderPtr,*iter);
                   }
                   out->flush();
               }

               void write(T* array,int size){
                   for(int i=0;i<size;i++) {
                       avro::codec_traits<T>::encode(*encoderPtr,array[i]);
                   }
                   out->flush();
               }

               void write(const T& obj){
                   avro::codec_traits<T>::encode(*encoderPtr,obj);
                   out->flush();
               }

               void commit(WriteDataCallback cb){
                   std::unique_ptr<avro::InputStream> is = avro::memoryInputStream(*out);
                   const uint8_t* nextBuffer = NULL;
                   size_t bufferSize = 0;
                   while(is->next(&nextBuffer,&bufferSize)){
                        cb(nextBuffer,bufferSize);
                   }
               }

           private:
               std::unique_ptr<avro::OutputStream> out;
               avro::EncoderPtr encoderPtr;
           };

       }

   }
}
 





#endif
