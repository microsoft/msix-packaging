#include "xPlatAppx.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "ZipStream.hpp"

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

XPLATAPPX_API unsigned int UnpackAppx(char* source, char* destination)
{
    return ResultOf(source, destination, [&]() {
        std::string appxFileName(source);
        xPlat::ZipStream zip(std::move(std::make_unique<xPlat::FileStream>(
            std::move(appxFileName),
            xPlat::FileStream::Mode::READ)));
        zip.Read();
    });
}

XPLATAPPX_API unsigned int PackAppx  (char* source, char* destination)
{
    return ResultOf(source, destination, []() {
        // TODO: implement here
    });
}
