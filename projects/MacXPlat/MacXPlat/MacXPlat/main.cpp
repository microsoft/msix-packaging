//
//  main.cpp
//  MacXPlat
//
//  Created by Phil Smith on 9/14/17.
//  Copyright © 2017 Phil Smith. All rights reserved.
//

#include <iostream>
#include <string>
#include <exception>
#include <cstdio>

namespace xPlat {
    class NotImplementedException : public std::exception { };
    class NotSupportedException : public std::exception { };
    
    class StreamBase
    {
    public:
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };
        
        virtual ~StreamBase() {}
        
        // just like fwrite
        virtual void Write(std::size_t size, const std::uint8_t* bytes)
        { throw NotImplementedException(); }
        
        // just like freed
        virtual std::size_t Read(std::size_t size, const std::uint8_t* bytes)
        { throw NotImplementedException(); }
        
        // just like fseek
        virtual void Seek(long offset, Reference where )
        { throw NotImplementedException(); }
        
        virtual void CopyTo(StreamBase& to)
        {
            std::uint8_t buffer[1024];  // 1k at a time ought to be sufficient
            std::size_t bytes = Read(sizeof(buffer), buffer);
            while (bytes !=0)
            {
                to.Write(bytes, buffer);
                bytes = Read(sizeof(buffer), buffer);
            }
        }
        
        virtual void Close()
        { throw NotImplementedException(); }
    };
    
    // used to calculate offsets
    class OffsetStream : public StreamBase
    {
    public:
        void CopyTo(StreamBase& to) { throw NotImplementedException(); }
        virtual void Write(std::size_t size, const std::uint8_t* bytes)
        {
            count += size;
        }
        
        std::size_t Offset() { return count; }
    protected:
        
        std::size_t count = 0;
        
    };
    
    class FileException : public std::exception
    {
    public:
        FileException(std::string& message, int error = 0) : reason(message), code(error) {}
        
        std::string reason;
        int code;
    };
    
    // TODO: turns out we DO need some sort of abstraction over storage -- investigate using BOOST?
    class FileStream : public StreamBase
    {
    public:
        enum Mode { READ = 0, WRITE, APPEND, READ_UPDATE, WRITE_UPDATE, APPEND_UPDATE };
        
        FileStream(std::string& path, Mode& mode) : name(path)
        {
            static char* modes[] = { "r", "w", "a", "r+", "w+", "a+" };
            file = std::fopen(path.c_str(), modes[mode]);
            if (!file)
            {
                throw FileException(path);
            }
        }
        
        virtual ~FileStream() {
            Close();
        }
        
        void Close()
        {
            if (file) {
                int rc = std::fclose(file);
                if (rc != 0) { throw FileException(name, rc); }
                file = nullptr;
            }
        }
        
        void Seek(long offset, Reference where)
        {
            int rc = std::fseek(file, offset, where);
            if (rc != 0) { throw FileException(name, rc); }
        }
        
        std::size_t Read(std::size_t size, const std::uint8_t* bytes)
        {
            std::size_t bytesRead = std::fread(
                static_cast<void*>(const_cast<std::uint8_t*>(bytes)), 1, size, file
                );
            if (bytesRead < size && !std::feof(file))
            {
                throw FileException(name, std::ferror(file));
            }
            return bytesRead;
        }
        
        void Write(std::size_t size, const std::uint8_t* bytes)
        {
            std::size_t bytesWritten = std::fwrite(
                static_cast<void*>(const_cast<std::uint8_t*>(bytes)), 1, size, file
                );
            if (bytesWritten != size)
            {
                throw FileException(name, std::ferror(file));
            }
        }
        
    protected:
        std::string name;
        FILE* file;
    };
    
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
