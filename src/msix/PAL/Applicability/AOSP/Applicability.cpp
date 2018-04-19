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

    struct Bcp47Entry
    {
        const char* icu;
        const char* bcp47;

        Bcp47Entry(const char* i, const char* b) : icu(i), bcp47(b) {}

        inline bool operator==(const char* otherIcu) const {
            return 0 == strcmp(icu, rhs.icu);
        }
    };

    // ICU locale were introduced in API level 24. Add here any inconsistencies.
    static const Bcp47Entry bcp47List[] = {
        Bcp47Entry("zh_CN", "zh-Hans-CN"),
        Bcp47Entry("zh_HK", "zh-Hant-HK"),
        Bcp47Entry("zh_TW", "zh-Hant-TW"),
    };

    MSIX_PLATFORM Applicability::GetPlatform() { return MSIX_PLATFORM_AOSP; }

    std::vector<std::string> Applicability::GetLanguages()
    {
        std::vector<std::string> languages;
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
        const auto& tag = std::find(std::begin(bcp47List), std::end(bcp47List), language);
        if (tag == std::end(bcp47List))
        {
            std::string bcp47(language);
            std::replace(bcp47.begin(), bcp47.end(), '_', '-');
            languages.push_back(bcp47);
        }
        else
        {
            languages.push_back((*tag).bcp47);

        }
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
