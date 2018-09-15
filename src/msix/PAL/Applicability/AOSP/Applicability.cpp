//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include <vector>
#include <algorithm>
#include <iterator>

#include "Applicability.hpp"
#include "Exceptions.hpp"
#include "JniHelper.hpp"

namespace MSIX {

    MSIX_PLATFORMS Applicability::GetPlatform() { return MSIX_PLATFORM_AOSP; }

    std::vector<Bcp47Tag> Applicability::GetLanguages()
    {
        std::vector<Bcp47Tag> languages;
        JNIEnv* env = Jni::Instance()->GetEnv();
        std::unique_ptr<_jclass, JObjectDeleter> javaClass(env->FindClass("com/microsoft/msix/Language"));
        ThrowErrorIf(Error::Unexpected, !javaClass.get(), "Failed looking for our java class.");
        jmethodID languageFunc = env->GetStaticMethodID(javaClass.get(), "getLanguages", "()[Ljava/lang/String;");
        ThrowErrorIf(Error::Unexpected, !languageFunc, "Failed calling getLanguages().");
        std::unique_ptr<_jobjectArray, JObjectDeleter> javaLanguages(reinterpret_cast<jobjectArray>(env->CallStaticObjectMethod(javaClass.get(), languageFunc)));
        for(int i = 0; i < env->GetArrayLength(javaLanguages.get()); i++)
        {
            //std::unique_ptr<_jobject, JObjectDeleter> arrayElement (env->GetObjectArrayElement(javaLanguages.get(), i));
            std::unique_ptr<_jstring, JObjectDeleter>  javaLanguage (reinterpret_cast<jstring>(env->GetObjectArrayElement(javaLanguages.get(), i)));

            // BCP47 format
            std::string bcp47(GetStringFromJString(javaLanguage.get()));
            std::replace(bcp47.begin(), bcp47.end(), '_', '-');
            languages.push_back(Bcp47Tag(bcp47));
        }
       
        return languages;
    }
}
