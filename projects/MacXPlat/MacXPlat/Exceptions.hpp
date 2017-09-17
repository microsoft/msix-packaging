
#include <exception>

namespace xPlat {

    class ExceptionBase : public std::exception
    {
    public:
        enum Facility : public uint32_t {
            NONE = 0x0000,
            FILE = 0x1000,
            CRC  = 0x2000,
            MAX =  0x0FFF      // always last, always defines bytes reserved for forwarding errors
        } facility = Facility::NONE;

        void SetLastError(uint32_t error)
        {
            code = Header + facility + (error & Facility::MAX);
        }

        uint32_t Code() { return code; }

    protected:
        uint32_t Header = 0x8BAD0000;   // facility 2989
        uint32_t code   = 0xFFFFFFFF;   // by default, something very bad happened...
    };

    class NotImplementedException : public ExceptionBase { public: NotImplementedException { SetLastError(1); } };
    class NotSupportedException : public ExceptionBase {   public: NotSupportedException   { SetLastError(2); } };

    class FileException : public ExceptionBase
    {
    public:
        FileException(std::string& message, int error = 0) : reason(message), facility(Facility::FILE)
        {
            SetLastError(error);
        }

        std::string reason;
    };


}