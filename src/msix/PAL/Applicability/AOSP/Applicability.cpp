//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include <vector>
#include <algorithm>
#include <iterator>

#include "Applicability.hpp"
#include "Exceptions.hpp"

// Android includes
#include <jni.h>

static JavaVM* g_JavaVM = nullptr;

namespace MSIX {

    MSIX_PLATFORMS Applicability::GetPlatform() { return MSIX_PLATFORM_AOSP; }

    std::vector<Bcp47Tag> Applicability::GetLanguages()
    {
        std::vector<Bcp47Tag> languages;
        JNIEnv* env;
        bool isThreadAttached = false;
        int result = g_JavaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        if (result < 0)
        {   // Native thread. This should always be the case.
            result = g_JavaVM->AttachCurrentThread(&env, nullptr);
            ThrowErrorIf(Error::Unexpected, result < 0, "Failed attaching the thread.");
            isThreadAttached = true;
        }
        jclass javaClass = env->FindClass("com/microsoft/msix/JniHelper");
        ThrowErrorIf(Error::Unexpected, !javaClass, "Failed looking for our java class.");
        jmethodID languageFunc = env->GetStaticMethodID(javaClass, "getLanguage", "()Ljava/lang/String;");
        ThrowErrorIf(Error::Unexpected, !languageFunc, "Failed calling getLanguage().");
        jstring javaLanguage = reinterpret_cast<jstring>(env->CallStaticObjectMethod(javaClass, languageFunc));
        const char* language = env->GetStringUTFChars(javaLanguage, nullptr);
        ThrowErrorIf(Error::Unexpected, !language, "Failed getting langauge from the system.");
        // BCP47 format
        std::string bcp47(language);
        std::replace(bcp47.begin(), bcp47.end(), '_', '-');
        languages.push_back(Bcp47Tag(bcp47));
        env->ReleaseStringUTFChars(javaLanguage, language);
        if (isThreadAttached)
        {
            g_JavaVM->DetachCurrentThread();
        }
        return languages;
    }
}

__attribute__((visibility("default")))
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    g_JavaVM = vm;
    JNIEnv* env;
    if (g_JavaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}
