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

        // IAppx*Enumerator
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

    // Helper class for implementing any IAppx*Enumerator that has an string as an out parameter
    // for their GetCurrent method.
    template<typename EnumeratorInterface>
    class EnumeratorString final : public MSIX::ComClass<EnumeratorString<EnumeratorInterface>, EnumeratorInterface>
    {
    public:
        EnumeratorString(IMsixFactory* factory, std::vector<std::string>& values) :
            m_factory(factory), m_values(values)
        {}

        // IAppx*Enumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(LPWSTR* value) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (value == nullptr || *value != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_values.at(m_cursor), value);
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) noexcept override try
        {   
            ThrowErrorIfNot(Error::InvalidParameter, (hasCurrent), "bad pointer");
            *hasCurrent = (m_cursor != m_values.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) noexcept override try
        {
            ThrowErrorIfNot(Error::InvalidParameter, (hasNext), "bad pointer");
            *hasNext = (++m_cursor != m_values.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    protected:
        IMsixFactory* m_factory;
        std::vector<std::string> m_values;
        std::size_t m_cursor = 0;
    };
}
