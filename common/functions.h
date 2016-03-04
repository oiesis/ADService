//
// Created by guoze.lin on 16/2/5.
//

#ifndef ADCORE_FUNCTIONS_H
#define ADCORE_FUNCTIONS_H

#include <iostream>

#ifndef _GLIBCXX_USE_NOEXCEPT
#define  _GLIBCXX_USE_NOEXCEPT
#endif

#ifdef UNIT_TEST

template<typename T>
void DebugMessage(T& obj){
    std::cout<<obj<<std::endl;
}

template<typename T,typename ...Args>
void DebugMessage(T& obj,Args... args){
   std::cout<<obj;
   DebugMessage(args...);
};

#include "utility/mttytime.h"

template<typename T>
void DebugMessageWithTime(T& obj){
    using namespace adservice::utility::time;
    std::cout<<getCurrentTimeUtcString()<<" "<<obj<<std::endl;
}

template<typename T,typename ...Args>
void DebugMessageWithTime(T& obj,Args... args){
    using namespace adservice::utility::time;
    std::cout<<getCurrentTimeUtcString()<<" "<<obj;
    DebugMessage(args...);
};

#else
#define DebugMessage(a) {}
#define DebugMessage(a,b) {}
#define DebugMessage(a,b,c) {}
#define DebugMessage(a,b,c,d) {}
#define DebugMessage(a,b,c,d,e) {}
#define DebugMessage(a,b,c,d,e,f) {}
#define DebugMessage(a,b,c,d,e,f,g) {}
#define DebugMessageWithTime(a) {}
#define DebugMessageWithTime(a,b) {}
#define DebugMessageWithTime(a,b,c) {}
#define DebugMessageWithTime(a,b,c,d) {}
#define DebugMessageWithTime(a,b,c,d,e) {}
#define DebugMessageWithTime(a,b,c,d,e,f) {}
#define DebugMessageWithTime(a,b,c,d,e,f,g) {}
#endif
#endif //ADCORE_FUNCTIONS_H
