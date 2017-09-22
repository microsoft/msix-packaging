
#include <exception>

namespace xPlat {

    class ExceptionBase : public std::exception
    {
    public:
        enum Facility : std::uint32_t {
            NONE = 0x0000,
            FILE = 0x1000,
            ZIP =  0x2000,
            CRC =  0x4000,
            MAX =  0x0FFF      // always last, always defines bytes reserved for forwarding errors
        };

        ExceptionBase() {}
        ExceptionBase(Facility facility) : facility(facility) {}

        void SetLastError(std::uint32_t error)
        {
            code = header + facility + (error & Facility::MAX);
        }

        uint32_t Code() { return code; }

    protected:
        Facility facility = Facility::NONE;
        std::uint32_t header = 0x8BAD0000;   // facility 2989
        std::uint32_t code   = 0xFFFFFFFF;   // by default, something very bad happened...
    };

    class NotImplementedException : public ExceptionBase { public: NotImplementedException() { SetLastError(1); } };
    class NotSupportedException : public ExceptionBase {   public: NotSupportedException()   { SetLastError(2); } };

    class FileException : public ExceptionBase
    {
    public:
        FileException(std::string message, std::uint32_t error = 0) :
            reason(message),
            ExceptionBase(ExceptionBase::Facility::FILE)
        {
            SetLastError(error);
        }

        std::string reason;
    };


}