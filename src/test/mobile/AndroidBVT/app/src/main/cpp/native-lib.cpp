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

#include "msixtest.hpp"

std::string GetStringPathFromJString(JNIEnv* env, jstring jFilePath)
{
    std::string path = std::string(env->GetStringUTFChars(jFilePath, JNI_FALSE));
    return  path  + "/" ;
}

void CreateSubDirectory(const std::string& path)
{
    if (-1 == mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
    {
        __android_log_print(ANDROID_LOG_ERROR, "MSIX", "Error creating directory: %s with %s", path.c_str(), strerror(errno));
    }
}

// Assets are not unpacked from the APK. Retrieve them and copy them for our test to read.
void CopyFilesFromAssets(JNIEnv* env, jobject assetManager, std::string filePath, const std::string& subDirectory)
{
    __android_log_print(ANDROID_LOG_INFO, "MSIX", "Start: %s", subDirectory.c_str());
    CreateSubDirectory(filePath + subDirectory);

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
    __android_log_print(ANDROID_LOG_INFO, "MSIX", "End: %s", subDirectory.c_str());
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_microsoft_androidbvt_MainActivity_RunTests(JNIEnv* env, jobject /* this */,
                                                          jobject assetManager, jstring jFilePath)
{
    std::string filePath = GetStringPathFromJString(env, jFilePath);

    // Create top level subdirectory
    CreateSubDirectory(filePath + "testData");

    CopyFilesFromAssets(env, assetManager, filePath, "testData/unpack");
    CopyFilesFromAssets(env, assetManager, filePath, "testData/unpack/BlockMap");
    CopyFilesFromAssets(env, assetManager, filePath, "testData/unpack/bundles");
    CopyFilesFromAssets(env, assetManager, filePath, "testData/unpack/flat");
    CopyFilesFromAssets(env, assetManager, filePath, "testData/unpack/महसुस");

    std::string outputFile = filePath + "TEST-MsixSDK-AOSP.xml";
    filePath = filePath.substr(0, filePath.size() - 1);

    char* arguments [6] = { "msix_test", "-s", "-r", "junit" , "-o", const_cast<char*>(outputFile.c_str()) };
    int result = msixtest_main(6, arguments, filePath.c_str());
    std::string output = "Tests: ";
    if (result == 0)
    {
        output += "PASSED\n";
    }
    else
    {
        output += "FAILED\n";
    }

    return env->NewStringUTF(output.c_str());
}
