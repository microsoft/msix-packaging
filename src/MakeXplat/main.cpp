#include "xPlatAppx.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    return UnpackAppx(
        xPlatPackUnpackOptions::xPlatPackUnpackOptionsNone,
        xPlatValidationOptions::xPlatValidationOptionSkipAppxManifestValidation,
        argv[1],
        argv[2]
    );

    //return ValidateAppxSignature(argv[1]);
}