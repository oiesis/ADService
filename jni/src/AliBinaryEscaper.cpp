//
// Created by guoze.lin on 16/2/24.
//


#include <string>
#include "AliBinaryEscaper.h"


namespace adservice{
    namespace utility{
        namespace escape{
            extern std::string encode4ali(const char* buf,int len);
            extern std::string decode4ali(const char* inputBuf,int len);
        }
    }
}


using namespace adservice::utility::escape;
using namespace std;

extern "C" JNIEXPORT jbyteArray JNICALL Java_com_mtty_cypher_AliBinaryEscaper_unescape
        (JNIEnv * env, jclass cls, jbyteArray input){
    // get byte[] from jvm
    unsigned char* buffer = (unsigned char*)env->GetByteArrayElements(input, NULL);
    int size = env->GetArrayLength(input);

    string output = decode4ali((const char*)buffer,size);

    jbyteArray result = env->NewByteArray(output.length());
    env->SetByteArrayRegion (result, 0, output.length(), reinterpret_cast<const jbyte*>(output.c_str()));
    // return byte[] back to jvm
    env->ReleaseByteArrayElements(input, (jbyte*)buffer, JNI_ABORT);
    return result;
}


extern "C" JNIEXPORT jbyteArray JNICALL Java_com_mtty_cypher_AliBinaryEscaper_escape
        (JNIEnv *env, jclass cls, jbyteArray input){
    // get byte[] from jvm
    unsigned char* buffer = (unsigned char*)env->GetByteArrayElements(input, NULL);
    int size = env->GetArrayLength(input);

    string output = encode4ali((const char*)buffer,size);

    jbyteArray result = env->NewByteArray(output.length());
    env->SetByteArrayRegion (result, 0, output.length(), reinterpret_cast<const jbyte*>(output.c_str()));
    // return byte[] back to jvm
    env->ReleaseByteArrayElements(input, (jbyte*)buffer, JNI_ABORT);
    return result;
}
