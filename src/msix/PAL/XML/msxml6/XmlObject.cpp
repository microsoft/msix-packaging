#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "IXml.hpp"

// schemas
#include "AppxBlockMapSchemas.hpp"
#include "ContentTypesSchemas.hpp"

namespace MSIX {

ComPtr<IXmlFactory> CreateXmlFactory()
{
    throw Exception(Error::NotImplemented);
}

} // namespace MSIX