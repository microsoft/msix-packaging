//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
#include "ApiTests.hpp"

#include <string>
#include <iostream>

void Help()
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << "\tapitests [-f <file>] [-d <output directory>] " << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "\tValidates MSIX SDK APIs" << std::endl;
    std::cout << "\t\t-f <file>      : takes the specified file as input. By default, uses console input" << std::endl;
    std::cout << "\t\t-d <directory> : outputs the result in <directory>/testApiResults.txt. By default, print to the console." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    char* source = nullptr;
    char* target = nullptr;

    for (int i = 1; i < argc; i++)
    {
        auto option = std::string(argv[i]);
        if (option == "-f")
        {
            if (++i == argc)
            {
                Help();
                return 1;
            }
            else
            {
                source = argv[i];
            }
        }
        else if (option == "-d")
        {
            if (++i == argc)
            {
                Help();
                return 1;
            }
            else
            {
                target = argv[i];
            }
        }
        else
        {
            Help();
            return 1;
        }
    }

    RunApiTest(source, target, nullptr);
    return 0;
}