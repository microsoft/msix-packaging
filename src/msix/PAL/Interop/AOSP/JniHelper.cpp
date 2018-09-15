//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
 #include "JniHelper.hpp"
 #include "Exceptions.hpp"
 
 JavaVM* g_JavaVM = nullptr;
 
 JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
 {
     g_JavaVM = vm;
     JNIEnv *env;
     if (g_JavaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
     {
         return JNI_ERR;
     }
     return JNI_VERSION_1_6;
 }

std::string GetStringFromJString(jstring jvalue)
{
    JNIEnv* env = Jni::Instance()->GetEnv();
    const char* value = env->GetStringUTFChars(jvalue, nullptr);
    std::string returnVal (value);
    env->ReleaseStringUTFChars(jvalue, value);
    return returnVal;
}