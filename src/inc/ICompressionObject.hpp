//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <cstdlib>
#include <cstdint>
#include <memory>

namespace MSIX {
    enum class CompressionOperation
    {
        Inflate,
        Deflate
    };

    enum class CompressionStatus
    {
        Ok,
        Error,
        End,
        NeedDictionary
    };

    class ICompressionObject
    {
        public:
            virtual CompressionStatus Initialize(CompressionOperation operation) = 0;
            virtual CompressionStatus Inflate() = 0;
            virtual CompressionStatus Cleanup() = 0;
            virtual std::size_t GetAvailableSourceSize() = 0;
            virtual std::size_t GetAvailableDestinationSize() = 0;
            virtual void SetInput(std::uint8_t* buffer, std::size_t size) = 0;
            virtual void SetOutput(std::uint8_t* buffer, std::size_t size) = 0;
            virtual ~ICompressionObject() = default;
    };

    std::unique_ptr<ICompressionObject> CreateCompressionObject();
}