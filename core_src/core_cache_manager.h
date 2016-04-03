//
// Created by guoze.lin on 16/3/25.
//

#ifndef ADCORE_CORE_CACHE_MANAGER_H
#define ADCORE_CORE_CACHE_MANAGER_H

#include <unistd.h>
#include <string>
#include <functional>
#include "muduo/base/Mutex.h"
#include "spinlock.h"
#include "utility/mttytime.h"

namespace adservice{
    namespace cache{

        using namespace std;
        using namespace adservice::utility::time;
        using namespace muduo;

        static const        int MEMORY_ALLOC_UNIT               = 2048;
        static const        int MEMORY_ALLOC_UNIT_CNT           = 1024;
        static constexpr    int DEFAULT_MEMORY_ALLOC_SIZE       = MEMORY_ALLOC_UNIT * ((MEMORY_ALLOC_UNIT_CNT<<1)+1);
        static constexpr    int DEFAULT_POOL_UNIT_CNT           = (MEMORY_ALLOC_UNIT_CNT<<1)+1;
        static const        int64_t EXPIRE_TIME_INFINITE        = 0x7FFFFFFFFFFFFFFF;
        static const        int CACHE_MAX_LEVEL                 = 3;
        static const        int CACHE_KEY_LENGTH                = 56;

        struct CacheResult{
            char key[CACHE_KEY_LENGTH];
            uchar_t* data;
            int32_t size;
            int64_t expireTime;
            CacheResult* next;
            CacheResult(){
                expireTime = EXPIRE_TIME_INFINITE;
                next = NULL;
                data = NULL;
                size = 0;
            }
            void setKey(const char* k){
                strncpy(key,k,CACHE_KEY_LENGTH);
            }
            bool expired(){
                return expireTime <= getCurrentTimeStamp();
            }
            bool expired(int64_t time){
                return expireTime <= time;
            }
            void setExpire(int64_t expTime){
                expireTime = expTime;
            }
        };

        struct CacheSlot{
            struct CacheResult* caches[1024];
            MutexLock mutex[1024];
            CacheSlot(){
                for(int i=0;i<1024;i++){
                    caches[i] = NULL;
                }
            }
            ~CacheSlot(){
            }
        };

        class MemoryManageStategy{
        public:
            enum MemoryManageMethod{
                METHOD_LINKLIST
            };
        public:
            MemoryManageStategy(MemoryManageMethod m,int n):method(m),nodeCnt(n){

            }
            void setAux(int* a){
                aux = a;
            }
            void init(){
                aux[nodeCnt-1] = -1;
                for(int i=nodeCnt-2;i>=0;i--){
                    aux[i] = i+1;
                }
                head = 0;
                spinlock_init(&lock);
            }
            ~MemoryManageStategy(){
                spinlock_destroy(&lock);
            }
            int alloc();
            void free(int id);
        private:
            MemoryManageMethod method;
            int* aux;
            int nodeCnt;
            int head;
            spinlock lock;
        };

        class MemoryPool {
        public:
            MemoryPool():memory(NULL),memStategy(MemoryManageStategy::METHOD_LINKLIST,DEFAULT_POOL_UNIT_CNT){
            }
            void init(int size = DEFAULT_MEMORY_ALLOC_SIZE,int unit = MEMORY_ALLOC_UNIT){
                memory = new uchar_t[size];
                unitSize = unit;
                memStategy.setAux(aux);
                memStategy.init();
            }
            ~MemoryPool(){
                if(memory!=NULL) {
                    delete[] memory;
                    memory = NULL;
                }
            }
            void* alloc();
            void free(void* p);
            int getUnitSize(){
                return unitSize;
            }
        private:
            int unitSize;
            uchar_t* memory;
            int aux[DEFAULT_POOL_UNIT_CNT];
            MemoryManageStategy memStategy;
        };

        typedef std::function<bool (CacheResult&)> CacheAbsentCallback;

        typedef std::function<int (const char*)> HashFunc;

        class CacheManager{
        public:
            CacheManager(){
                //init memory pool
                int allocSize = DEFAULT_MEMORY_ALLOC_SIZE;
                int unitSize = MEMORY_ALLOC_UNIT;
                for(int i=0;i<CACHE_MAX_LEVEL;i++){
                    memPools[i].init(allocSize,unitSize);
                    allocSize+=DEFAULT_MEMORY_ALLOC_SIZE;
                    unitSize+=MEMORY_ALLOC_UNIT;
                }
                cacheResultSpare.init(10240*sizeof(CacheResult),sizeof(CacheResult));
            }
            void destroy(){
            }

            void setHashMethod(const HashFunc& hFunc){
                hashFunc = hFunc;
            }

            inline int hash(const std::string& key){
                return hash(key.c_str());
            }

            int hash(const char* key);

            /**
             * 设置缓存,成功返回true,失败返回false
             */
            bool set(const char* key,void* value,int size,int64_t expireTime = EXPIRE_TIME_INFINITE);

            /**
             * 设置缓存,成功返回true,失败返回false
             */
            inline bool set(const std::string& key,void* value,int size,int64_t expireTime = EXPIRE_TIME_INFINITE){
                return set(key.c_str(),value,size,expireTime);
            }


            /**
             * 获取缓存结果,失败返回NULL
             */
            CacheResult* get(const char* key,int dataSize = -1,const CacheAbsentCallback& cb = NULL);

            /**
             * 获取缓存结果,失败返回NULL
             * dataSize:为预计的data大小
             */
            inline CacheResult* get(const std::string& key,int dataSize,const CacheAbsentCallback& cb = NULL){
                return get(key.c_str(),dataSize,cb);
            }


        private:
            //多级缓存,每一级容量依次增加
            CacheSlot slots[CACHE_MAX_LEVEL];
            MemoryPool memPools[CACHE_MAX_LEVEL];
            MemoryPool cacheResultSpare;
            HashFunc hashFunc;
        };

    }
}

#endif //ADCORE_CORE_CACHE_MANAGER_H
