//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <jni.h>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include "MobileTests.hpp"
#include "MSIXWindows.hpp"
#include "ApiTests.hpp"

std::string GetStringPathFromJString(JNIEnv* env, jstring jFilePath)
{
    std::string path = std::string(env->GetStringUTFChars(jFilePath, JNI_FALSE));
    return  path  + "/" ;
}

// Assets are not unpacked from the APK. Retrieve them and copy them for our test to read.
void CopyFilesFromAssets(JNIEnv* env, jobject assetManager, std::string filePath, std::string subDirectory)
{
    // Create subdirectory if needed
    if (!subDirectory.empty())
    {   std::string path = filePath + subDirectory;
        if (-1 == mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
        {   __android_log_print(ANDROID_LOG_ERROR, "MSIX", "Error creating directory: %s", strerror(errno));
        }
    }

    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
    AAssetDir* assetDir = AAssetManager_openDir(manager, subDirectory.c_str());
    const char* fileName;
    int  count = 0;
    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr)
    {   std::string relativeFile;
        // fileName is relative to the opened dir. AAssetManager_open needs the relative
        // path from the assets directory.
        if (!subDirectory.empty())
        {   relativeFile = subDirectory + "/" + std::string(fileName);
        }
        else
        {   relativeFile = std::string(fileName);
        }

        // Get asset and copy it.
        AAsset* asset = AAssetManager_open(manager, relativeFile.c_str(), AASSET_MODE_STREAMING);
        int bytesRead = 0;
        char buffer[BUFSIZ];
        std::string fileToCreate = filePath + std::string(relativeFile);
        __android_log_print(ANDROID_LOG_INFO, "MSIX", "File: %s", fileToCreate.c_str());
        FILE* newFile = fopen(fileToCreate.c_str(), "w");
        if (newFile == nullptr)
        {   __android_log_print(ANDROID_LOG_ERROR, "MSIX", "File: %s Error: %s", fileToCreate.c_str(), strerror(errno));
        }
        else
        {   while ((bytesRead = AAsset_read(asset, buffer, BUFSIZ)) > 0)
            {   fwrite(buffer, bytesRead, 1, newFile);
            }
            fclose(newFile);
        }
        AAsset_close(asset);
    }
    AAssetDir_close(assetDir);
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_microsoft_androidbvt_MainActivity_RunTests(JNIEnv* env, jobject /* this */,
                                                          jobject assetManager, jstring jFilePath)
{
    std::string output = "";
    std::string filePath = GetStringPathFromJString(env, jFilePath);
    CopyFilesFromAssets(env, assetManager, filePath, "");
    CopyFilesFromAssets(env, assetManager, filePath, "BlockMap");
    CopyFilesFromAssets(env, assetManager, filePath, "bundles");
    CopyFilesFromAssets(env, assetManager, filePath, "flat");
    CopyFilesFromAssets(env, assetManager, filePath, "महसुस");
    signed long hr = RunTests(const_cast<char*>(filePath.c_str()), const_cast<char*>(filePath.c_str()));
    output += "End-to-end tests: ";
    if (hr == 0)
    {
        output += "PASSED\n";
    }
    else
    {
        output += "FAILED\n";
    }
    std::string input = filePath + "apitest_test_1.txt";
    int apiResult = RunApiTest(const_cast<char*>(input.c_str()), const_cast<char*>(filePath.c_str()), const_cast<char*>(filePath.c_str()));
    output += "Api tests: ";
    if (apiResult == 0)
    {
        output += "PASSED\n";
    }
    else
    {
        output += "FAILED\n";
    }
    return env->NewStringUTF(output.c_str());
}
