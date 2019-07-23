
#include <napi.h>
#include <cstdint>
#include <string>
#include <iostream>

#include "MsixErrors.hpp"
#include "AppxPackaging.hpp"
#include "Log.hpp"

// Same arguments as UnpackPackage in AppxPackaging.hpp
//    MSIX_PACKUNPACK_OPTION packUnpackOptions,
//    MSIX_VALIDATION_OPTION validationOption,
//    char* utf8SourcePackage,
//    char* utf8Destination
// TODO: change this to return Napi::BigInt
Napi::Number Unpack(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 4 ||
        !info[0].IsNumber() ||
        !info[1].IsNumber() ||
        !info[2].IsString() ||
        !info[3].IsString())
    {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return Napi::Number::New(env, static_cast<double>(MSIX::Error::InvalidParameter)); 
    }

    std::uint32_t packUnpackOpt = info[0].As<Napi::Number>().Uint32Value();
    std::uint32_t validationOption = info[1].As<Napi::Number>().Uint32Value();
    std::string source = info[2].As<Napi::String>().Utf8Value();
    std::string destination = info[3].As<Napi::String>().Utf8Value();

    HRESULT hr = UnpackPackage(static_cast<MSIX_PACKUNPACK_OPTION>(packUnpackOpt),
                               static_cast<MSIX_VALIDATION_OPTION>(validationOption),
                               const_cast<char*>(source.c_str()),
                               const_cast<char*>(destination.c_str()));
    
    if (hr != S_OK)
    {
        std::cout << MSIX::Global::Log::Text() << std::endl;
    }
    
    return Napi::Number::New(env, static_cast<double>(hr));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "unpack"),
              Napi::Function::New(env, Unpack));
  return exports;
}

NODE_API_MODULE(msix, Init)