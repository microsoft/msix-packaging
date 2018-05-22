//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "ComHelper.hpp"
#include "MSIXFactory.hpp"

#include <vector>
#include <string>

namespace MSIX {

    // Helper class for implementing any IAppx*Enumerator interfaces that has a COM interface as an 
    // out parameter for their GetCurrent method.
    template<typename EnumeratorInterface, typename ObjectType>
    class EnumeratorCom final : public MSIX::ComClass<EnumeratorCom<EnumeratorInterface, ObjectType>, EnumeratorInterface>
    {
    public:
        EnumeratorCom(std::vector<ComPtr<ObjectType>>& objects) :
            m_objects(objects)
        {}

        // AppxBundleManifestPackageInfoEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(ObjectType** object) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (object == nullptr || *object != nullptr), "bad pointer");
            auto obj = m_objects.at(m_cursor);
            *object = obj.Detach();
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) noexcept override try
        {   
            ThrowErrorIfNot(Error::InvalidParameter, (hasCurrent), "bad pointer");
            *hasCurrent = (m_cursor != m_objects.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) noexcept override try
        {
            ThrowErrorIfNot(Error::InvalidParameter, (hasNext), "bad pointer");
            *hasNext = (++m_cursor != m_objects.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    protected:
        std::vector<ComPtr<ObjectType>> m_objects;
        std::size_t m_cursor = 0;
    };
}
