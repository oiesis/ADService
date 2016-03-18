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
#include <fstream>
#include "types.h"
#include "functions.h"
#include "hash.h"
#include "cypher.h"
#include "json.h"
#include "url.h"
#include "mttytime.h"
#include "escape.h"

#include "google/protobuf/message.h"

namespace adservice{
   namespace utility{

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

       namespace file{

           inline void loadFile(char* buffer,const char* filePath){
               std::fstream fs(filePath,std::ios_base::in);
               if(!fs.good()){
                   std::cerr<<" can't open json file:"<<filePath<<std::endl;
                   return;
               }
               std::stringstream ss;
               do{
                   std::string str;
                   std::getline(fs,str,'\n');
                   ss << str;
               }while(!fs.eof());
               fs.close();
               std::string str = ss.str();
               memcpy(buffer,str.c_str(),str.length());
               buffer[str.length()]='\0';
           }

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
               encoderPtr->flush();
               //out->flush();
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
                   encoderPtr->flush();
                   out->flush();
               }

               void write(T* array,int size){
                   for(int i=0;i<size;i++) {
                       avro::codec_traits<T>::encode(*encoderPtr,array[i]);
                   }
                   encoderPtr->flush();
                   out->flush();
               }

               void write(const T& obj){
                   avro::codec_traits<T>::encode(*encoderPtr,obj);
                   encoderPtr->flush();
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
