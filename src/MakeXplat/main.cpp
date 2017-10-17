#include "xPlatAppx.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    return ValidateAppxSignature(argv[1]);
    //return UnpackAppx(argv[1], argv[2]);
}