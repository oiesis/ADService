//
// Created by guoze.lin on 16/3/18.
//
#define UNIT_TEST
#include <string>
#include "iostream"
#include "utility/cypher.h"
#include "utility/mttytime.h"
#include "functions.h"
#include "platform.h"

using namespace adservice::platform;

void endium_test(){
    cout<<isLittleEndium()?"litle endium":"big endium"<<endl;
}


int main(int argc,char** argv){
    endium_test();
    return 0;
}