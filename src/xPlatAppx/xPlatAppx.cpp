#include "Exceptions.hpp"
#include "xPlatAppx.hpp"

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
    return ResultOf(source, destination, []() {
        // TODO: implement here
    });
}

unsigned int PackAppx  (char* source, char* destination)
{
    return ResultOf(source, destination, []() {
        // TODO: implement here
    });
}
