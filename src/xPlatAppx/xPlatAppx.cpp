#include "xPlatAppx.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "ZipObject.hpp"
#include "DirectoryObject.hpp"
#include "AppxPackageObject.hpp"

#include <string>
#include <memory>
#include <functional>

// on apple platforms, compile with -fvisibility=hidden
#ifdef PLATFORM_APPLE
#undef XPLATAPPX_API
#define XPLATAPPX_API __attribute__((visibility("default")))

// Initializer.
__attribute__((constructor))
static void initializer(void) {                             // 2
    printf("[%s] initializer()\n", __FILE__);
}

// Finalizer.
__attribute__((destructor))
static void finalizer(void) {                               // 3
    printf("[%s] finalizer()\n", __FILE__);
}

#endif

// Provides an ABI exception boundary with parameter validation
using Lambda = std::function<void()>;

unsigned int ResultOf(char* source, char* destination, Lambda lambda)
{
    unsigned int result = 0;
    try
    {
        if (source == nullptr || destination == nullptr)
        {
            throw xPlat::Exception(xPlat::Error::InvalidParameter);
        }
        lambda();
    }
    catch (xPlat::Exception& e)
    {
        result = e.Code();
    }

    return result;
}

XPLATAPPX_API unsigned int UnpackAppx(
    xPlatPackUnpackOptions packUnpackOptions,
    xPlatValidationOptions validationOptions,
    char* source,
    char* destination)
{
    return ResultOf(source, destination, [&]() {
        // TODO: what if source and destination are something OTHER than a file paths?
        xPlat::AppxPackageObject appx(validationOptions,
            std::make_unique<xPlat::ZipObject>(
                std::make_unique<xPlat::FileStream>(
                    source, xPlat::FileStream::Mode::READ
                    )));
        
        xPlat::DirectoryObject to(destination);
        appx.Unpack(packUnpackOptions, to);
    });
}

XPLATAPPX_API unsigned int PackAppx(
    xPlatPackUnpackOptions packUnpackOptions,
    xPlatValidationOptions validationOptions,
    char* source,
    char* certFile,
    char* destination)
{
    return ResultOf(source, destination, [&]() {
        // TODO: what if source and destination are something OTHER than a file paths?
        xPlat::AppxPackageObject appx(validationOptions, std::move(
            std::make_unique<xPlat::ZipObject>(std::move(
                std::make_unique<xPlat::FileStream>(destination, xPlat::FileStream::Mode::WRITE_UPDATE)
            ))
        ));

        xPlat::DirectoryObject from(source);
        appx.Pack(packUnpackOptions, certFile, from);
        appx.CommitChanges();
    });
}
