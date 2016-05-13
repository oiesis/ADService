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
inline void DebugMessage(const T& obj){
    std::cout<<obj<<std::endl;
}

template<typename T,typename ...Args>
inline void DebugMessage(const T& obj,Args... args){
   std::cout<<obj;
   DebugMessage(args...);
};

#include "utility/mttytime.h"

template<typename T>
inline void DebugMessageWithTime(const T& obj){
    using namespace adservice::utility::time;
    std::cout<<getCurrentTimeUtcString()<<" "<<obj<<std::endl;
}

template<typename T,typename ...Args>
inline void DebugMessageWithTime(const T& obj,Args... args){
    using namespace adservice::utility::time;
    std::cout<<getCurrentTimeUtcString()<<" "<<obj;
    DebugMessage(args...);
};

#else
template<typename T>
inline void DebugMessage(T& obj){}

template<typename T,typename ...Args>
inline void DebugMessage(T& obj,Args... args){ }

template<typename T>
inline void DebugMessageWithTime(T& obj){}

template<typename T,typename ...Args>
inline void DebugMessageWithTime(T& obj,Args... args){}

#endif
#endif //ADCORE_FUNCTIONS_H
