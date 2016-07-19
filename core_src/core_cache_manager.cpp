//
// Created by guoze.lin on 16/3/25.
//

#include "core_cache_manager.h"
#include "common/functions.h"
#include "utility/hash.h"

namespace adservice{
    namespace cache{

        using namespace adservice::utility::hash;


        //内存分配算法思路:因为一开始预先分配了一大块内存,而且每次分配的内存都是固定大小的,希望每次总是分配出最低地址的空闲内存块,
        //希望每次找到空闲内存和归还内存的花销都足够小.
        //1)如果最低空闲地址不是必须的,使用链表索引可以得到O(1)的分配和回收时间复杂度,O(n)的空间复杂度
        //2)如果最低空闲地址是必须的,使用并查集可以得到均摊时间复杂度为log(n)的均摊时间复杂度,O(n)的空间复杂度

        int MemoryManageStategy::alloc() {
            if(head==-1)
                return -1;
            spinlock_lock(&lock);
            int memId = head;
            head = aux[head];
            spinlock_unlock(&lock);
            return memId;
        }

        void MemoryManageStategy::free(int id) {
            //DebugMessageWithTime("free memory pool unit ",id);
            spinlock_lock(&lock);
            aux[id]=head;
            head = id;
            spinlock_unlock(&lock);
        }

        void* MemoryPool::alloc() {
            int memId = memStategy.alloc();
            if(memId==-1){
                DebugMessageWithTime("memory pool ran out!!");
                return NULL;
            }
            int retAddr = memId*unitSize;
//            if(retAddr<0||retAddr>=totalSize||memId<0||memId>=DEFAULT_POOL_UNIT_CNT){
//                DebugMessageWithTime("MemoryPool::alloc failed,finalAddr:,",(memory+retAddr),"retAddr:",retAddr,",memory:",
//                                     reinterpret_cast<long>(memory),",memId:",memId,",unitSize:",unitSize);
//            }
            return memory+retAddr;
        }

        void MemoryPool::free(void* p){
//            if(!isValidAddr(p)){
//                DebugMessageWithTime("MemoryPool::free invalid addr", reinterpret_cast<long>(p));
//            }
            int memId = ((long)p-(long)memory)/unitSize;
//            if(memId>=DEFAULT_POOL_UNIT_CNT){
//                DebugMessageWithTime("MemoryPool::free invalid memId:",memId,",addr:", reinterpret_cast<long>(p),",memory:",
//                                     reinterpret_cast<long>(memory),",unitSize:",unitSize);
//            }
            memStategy.free(memId);
        }

        inline int getCacheLevel(int size){
            return std::min(size/MEMORY_ALLOC_UNIT,CACHE_MAX_LEVEL-1);
        }


        uint64_t CacheManager::hash(const char* key){
            if(hashFunc){
                return hashFunc(key);
            }else{
                return fnv_hash(key,strlen(key));
            }
        }

        bool CacheManager::set(const char* key,void* value,int size,int64_t expireTime) {
            int level = getCacheLevel(size);
            MemoryPool &pool = memPools[level];
            CacheSlot &slot = slots[level];
            uchar_t *data = (uchar_t *) pool.alloc();
            if (data == NULL)
                return false;
            memcpy(data, value, size);
            int h = std::abs(hash(key) % MEMORY_ALLOC_UNIT_CNT);
            CacheResult *newCache = (CacheResult *) cacheResultSpare.alloc();
            if (newCache == NULL) {
                pool.free(data);
                return false;
            }
            newCache->data = data;
            newCache->size = size;
            newCache->setKey(key);
            newCache->expireTime = expireTime;
            //加锁
            {
                //todo:增加version
                MutexLockGuard lockGuard(slot.mutex[h]);
                CacheResult *result = slot.caches[h];
                if (result) {
                    newCache->next = result;
                }
                slot.caches[h] = newCache;
            }
            return true;
        }


        CacheResult* CacheManager::get(const char* key,int dataSize,const CacheAbsentCallback& cb){
            int h = std::abs(hash(key)%MEMORY_ALLOC_UNIT_CNT);
            int64_t currentTime = getCurrentTimeStamp();
            if(dataSize!=-1) {
                int level = getCacheLevel(dataSize);
                CacheSlot& slot = slots[level];
                CacheResult* result;
                CacheResult** pre;
                {
                    // 加锁
                    MutexLockGuard lockGuard(slot.mutex[h]);
                    result = slot.caches[h];
                    pre = &(slot.caches[h]);
                    while (result) {
//                        if(!cacheResultSpare.isValidAddr(result)){
//                            DebugMessageWithTime("invalid cacheresult address:", reinterpret_cast<long>(result));
//                        }
                        if (result->expired(currentTime)) { //过期
                            *pre = result->next;
                            if(result->data!=NULL) {
                                memPools[level].free(result->data);
                            }
                            cacheResultSpare.free(result);
                            result = *pre;
                        } else if (strcmp(result->key, key) == 0) { //cache hit
                            //todo:latent risk:如果在hit传出内存后马上过期释放,可能引发未定义的行为.解决办法是将内存复制出去
                            return result;
                        } else {
                            pre = &(result->next);
                            result = result->next;
                        }
                    }
                    //cache missing
                    //DebugMessageWithTime("cache level ",level,",key:",key," cache miss");
                    if (cb) {
                        CacheResult *newCache = (CacheResult *) cacheResultSpare.alloc();
                        if (!newCache)
                            return NULL;
                        newCache->init();
                        newCache->data = (uchar_t *) memPools[level].alloc();
                        newCache->size = memPools[level].getUnitSize();
                        if (newCache->data && cb(*newCache)) { //成功
                            newCache->next = slot.caches[h];
                            newCache->setKey(key);
                            slot.caches[h] = newCache;
                            return newCache;
                        } //失败
                        if(newCache->data!=NULL)
                            memPools[level].free(newCache->data);
                        cacheResultSpare.free(newCache);
                    }
                }
                return result;
            }else{
                int size = MEMORY_ALLOC_UNIT-1;
                for(int i=0;i<CACHE_MAX_LEVEL;i++){
                    CacheResult* result = get(key,size,cb);
                    if(result!=NULL)
                        return result;
                    size+=MEMORY_ALLOC_UNIT;
                }
            }
            return NULL;
        }

    }
}