//
// Created by guoze.lin on 16/2/5.
//

#ifndef ADCORE_FUNCTIONS_H
#define ADCORE_FUNCTIONS_H

#include <iostream>

#ifdef UNIT_TEST



template<typename T>
void DebugMessage(T obj){
    std::cout<<obj<<std::endl;
}

template<typename T,typename ...Args>
void DebugMessage(T obj,Args... args){
   std::cout<<obj;
   DebugMessage(args...);
};

#else
#define DebugMessage(x) {}
#endif

#endif //ADCORE_FUNCTIONS_H
