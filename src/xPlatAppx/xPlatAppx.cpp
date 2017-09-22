#include "xPlatAppx.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "ZipStream.hpp"

#include <string>
#include <memory>

// Provides an ABI exception boundary with parameter validation
template <class Lambda>
unsigned int ResultOf(char* source, char* destination, Lambda& lambda)
{
    unsigned long result = 0;
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

unsigned int UnpackAppx(char* source, char* destination)
{
    return ResultOf(source, destination, [&]() {
        std::string appxFileName(source);
        xPlat::ZipStream zip(std::move(std::make_unique<xPlat::FileStream>(
            std::move(appxFileName),
            xPlat::FileStream::Mode::READ)));

    });
}

unsigned int PackAppx  (char* source, char* destination)
{
    return ResultOf(source, destination, []() {
        // TODO: implement here
    });
}
