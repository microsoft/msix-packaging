
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <string>

namespace xPlat {

    // This represents a raw stream over a file contained in a .zip file.
    class ZipFileStream : public StreamBase
    {
    public:
        // TODO: define what streams to pass in on the .ctor
        ZipFileStream(std::string fileName) : fileName(fileName)
        {
        }

    protected:
        std::string fileName;
    };
}