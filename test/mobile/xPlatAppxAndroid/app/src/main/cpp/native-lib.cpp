#include <jni.h>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include "MobileTests.hpp"
#include "AppxWindows.hpp"

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
        {   __android_log_print(ANDROID_LOG_ERROR, "xPlatAppx", "Error creating directory: %s", strerror(errno));
        }
    }

    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
    AAssetDir* assetDir = AAssetManager_openDir(manager, subDirectory.c_str());
    const char* fileName;
    int  count = 0;
    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr)
    {   std::string appxFileName;
        // fileName is relative to the opened dir. AAssetManager_open needs the relative
        // path from the assets directory.
        if(!subDirectory.empty())
        {   appxFileName = subDirectory + "/" + std::string(fileName);
        }
        else
        {   appxFileName = std::string(fileName);
        }

        // Get asset and copy it.
        AAsset* appxAsset = AAssetManager_open(manager, appxFileName.c_str(), AASSET_MODE_STREAMING);
        int bytesRead = 0;
        char buffer[BUFSIZ];
        std::string fileToCreate = filePath + std::string(appxFileName);
        __android_log_print(ANDROID_LOG_INFO, "xPlatAppx", "File: %s", fileToCreate.c_str());
        FILE* appxFile = fopen(fileToCreate.c_str(), "w");
        if(appxFile == nullptr)
        {   __android_log_print(ANDROID_LOG_ERROR, "xPlatAppx", "File: %s Error: %s", fileToCreate.c_str(), strerror(errno));
        }
        else
        {   while ((bytesRead = AAsset_read(appxAsset, buffer, BUFSIZ)) > 0)
            {   fwrite(buffer, bytesRead, 1, appxFile);
            }
            fclose(appxFile);
        }
        AAsset_close(appxAsset);
    }
    AAssetDir_close(assetDir);
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_microsoft_xplatappxandroid_MainActivity_RunTests(JNIEnv* env, jobject /* this */,
                                                          jobject assetManager, jstring jFilePath)
{
    std::string output = "Test failure";
    std::string filePath = GetStringPathFromJString(env, jFilePath);
    CopyFilesFromAssets(env, assetManager, filePath, "");
    CopyFilesFromAssets(env, assetManager, filePath, "BlockMap");
    HRESULT hr = RunTests(filePath);
    if(hr == S_OK)
    {
        output = "Finished running tests";
    }
    return env->NewStringUTF(output.c_str());
}
