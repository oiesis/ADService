//
// Created by guoze.lin on 16/1/28.
//

#ifndef ADCORE_SERIALIZE_H
#define ADCORE_SERIALIZE_H

#include <cstring>
#include <sstream>
#include "google/protobuf/message.h"

namespace adservice{
    namespace serialize{

        using google::protobuf::Message;

        template<typename T>
        inline T& getObject(T& obj,stringstream& stream){
            obj.ParseFromIstream(&stream);
        }

        template<typename T>
        inline void writeObject(T& obj,stringstream& stream){
            obj.SerializeToOstream(&stream);
        }

    }
}

#endif //ADCORE_SERIALIZE_H
