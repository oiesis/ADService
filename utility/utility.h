//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_UTILITY_H
#define ADCORE_UTILITY_H

#include "../common/types.h"
#include <ctime>

#define IN
#define OUT
#define INOUT

namespace adservice{
   namespace utility{
       namespace hash{

           /** hash functions **/

           uint64_t sax_hash(const void *key, int32_t len);

           uint64_t fnv_hash(const void *key, int32_t len);

           uint64_t oat_hash(const void *key, int32_t len);
       }

       namespace cypher{

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
