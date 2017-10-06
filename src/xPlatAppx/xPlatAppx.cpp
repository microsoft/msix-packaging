#include "xPlatAppx.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "ZipObject.hpp"
#include "DirectoryObject.hpp"

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
            throw xPlat::InvalidArgumentException();
        }
        lambda();
    }
    catch (xPlat::ExceptionBase& exception)
    {
        result = exception.Code();
    }

    return result;
}

XPLATAPPX_API unsigned int UnpackAppx(char* from, char* to)
{
    return ResultOf(from, to, [&]() {
        std::string source(from);
        std::string target(to);

        xPlat::DirectoryObject directory(std::move(target));

        auto rawFile = std::make_unique<xPlat::FileStream>(std::move(source), xPlat::FileStream::Mode::READ);

        {
            xPlat::ZipObject zip(rawFile.get());

            auto fileNames = zip.GetFileNames();
            for (const auto& fileName : fileNames)
            {
                auto sourceFile = zip.GetFile(fileName);
                auto targetFile = directory.OpenFile(fileName, xPlat::FileStream::Mode::WRITE_UPDATE);

                sourceFile->CopyTo(targetFile.get());
            }
        }
    });
}

XPLATAPPX_API unsigned int PackAppx  (char* from, char* to)
{
    return ResultOf(from, to, [&]() {
        std::string source(from);
        std::string target(to);

        xPlat::DirectoryObject directory(std::move(source));

        auto rawFile = std::make_unique<xPlat::FileStream>(std::move(target), xPlat::FileStream::Mode::WRITE);

        {
            xPlat::ZipObject zip(rawFile.get());

            auto fileNames = directory.GetFileNames();
            for (const auto& fileName : fileNames)
            {
                auto sourceFile = directory.GetFile(fileName);
                auto targetFile = zip.OpenFile(fileName, xPlat::FileStream::Mode::WRITE);

                sourceFile->CopyTo(targetFile.get());
            }
            zip.CommitChanges();
        }
    });
}
