//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once

 #include <jni.h>
 #include <memory>
 #include <Exceptions.hpp>

extern JavaVM* g_JavaVM;
std::string GetStringFromJString(jstring jvalue);

class Jni
{
private:
    Jni () = default;
    Jni(const Jni&) = delete;
    Jni(Jni&&) = delete;
    Jni& operator=(const Jni&) =delete;
    Jni& operator=(Jni&&) = delete;

    bool m_isThreadAttached = false;
    JNIEnv* m_env = nullptr;

    ~Jni()
    {
        if (m_isThreadAttached)
        {
            g_JavaVM->DetachCurrentThread();
        }
    }

public:

    static Jni* Instance()
    {
        static Jni jni;
        return &jni;
    }

    JNIEnv* GetEnv()
    {
        if (!m_env)
        {
            int result = g_JavaVM->GetEnv(reinterpret_cast<void**>(&m_env), JNI_VERSION_1_6);
            if (result < 0)
            {   // Native thread. This should always be the case.
                result = g_JavaVM->AttachCurrentThread(&m_env, nullptr);
                ThrowErrorIf(MSIX::Error::Unexpected, result < 0, "Failed attaching the thread.");
                m_isThreadAttached = true;
            }
        }
        return m_env;
    }
};

struct JObjectDeleter {
  void operator()(jobject obj)
  {
      Jni::Instance()->GetEnv()->DeleteLocalRef(obj);
  }
};