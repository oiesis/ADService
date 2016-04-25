//
// Created by guoze.lin on 16/4/8.
//

#include <exception>
#include "youku_price.h"
#include "utility.h"
#include "functions.h"

using namespace adservice::utility::cypher;
using namespace adservice::utility::url;

const char* TOKEN = "9016dcdd08974c498c19c1802e2f332d";
//const char* TOKEN = "e48c06fe0da2403db2de26e2fcfe14d5";


class PriceDecoder{
public:
    PriceDecoder(const char* token){
        keySize = 32;
        fromLittleEndiumHex(token,strlen(token),key,keySize,false);
    }
    int getPrice(const std::string& input){
        std::string result;
        std::string base64decode;
        std::string paddingString = input+padding[input.length()%4];
        urlsafe_base64decode(paddingString,base64decode);
        aes_ecbdecode(key,base64decode,result);
        return std::stoi(result);
    }
private:
    uchar_t key[32];
    int keySize;
private:
    static std::string padding[4];
};

std::string PriceDecoder::padding[4]={"","===","==","="};

PriceDecoder decoder(TOKEN);

int64_t youku_price_decode(const std::string& input){
    int price = 0;
    try {
        char buffer[2048];
        std::string output;
        urlDecode_f(input,output,buffer);//Maby twice?
        price = decoder.getPrice(output);
    }catch(std::exception& e){
        DebugMessageWithTime("youku_price_decode failed,input:",input);
    }
    return price;
}

