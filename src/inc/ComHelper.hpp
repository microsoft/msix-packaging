//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once
#include <cstddef>
#include <memory>
#include <atomic>
#include <type_traits>
#include <utility>

#include "Exceptions.hpp"
#include "AppxPackaging.hpp"

namespace MSIX {

    template <typename I0, typename I1, typename ...Interfaces>
    struct ChainInterfaces
    {
        static_assert(std::is_base_of<I1, I0>::value, "Interface has to derive from I0");
        static bool Includes(REFIID riid)
        {
            return riid == UuidOfImpl<I0>::iid || ChainInterfaces<I0, Interfaces...>::Includes(riid);
        }
    };

    template <typename I0, typename I1>
    struct ChainInterfaces<I0, I1>
    {
        static_assert(std::is_base_of<I1, I0>::value, "Interface has to derive from I0");
        static bool Includes(REFIID riid)
        {
            return riid == UuidOfImpl<I1>::iid;
        }
    };

    template <typename ...Interfaces>
    struct QIHelper;

    // Specialization
    template <typename I0, typename ...Interfaces>
    struct QIHelper<I0, Interfaces...> : I0, QIHelper<Interfaces...>
    {
        bool Matches(REFIID riid, void** ppvObject)
        {
            if (riid == UuidOfImpl<I0>::iid)
            {
                *ppvObject = static_cast<I0*>(this);
                return true;
            }
            return QIHelper<Interfaces...>::Matches(riid, ppvObject);
        }
    };

    template <typename I0, typename ...Chain, typename ...Interfaces>
    struct QIHelper<ChainInterfaces<I0, Chain...>, Interfaces...> : QIHelper<I0, Interfaces...>
    {
        bool Matches(REFIID riid, void** ppvObject)
        {
            if(ChainInterfaces<I0, Chain...>::Includes(riid))
            {
                *ppvObject = reinterpret_cast<I0*>(this);
                return true;
            }
            return QIHelper<I0, Interfaces...>::Matches(riid, ppvObject);
        }
    };

    // Terminal case, always returns false
    template <>
    struct QIHelper<>
    {
        bool Matches(REFIID /*riid*/, void** ppvObject)
        {
            *ppvObject = nullptr;
            return false;
        }
    };

    template <class T>
    class ComPtr
    {
    public:
        // default ctor
        ComPtr() : m_ptr(nullptr) {}

        // For use instead of ComPtr<T> t(new Foo(...));
        template<class U, class... Args>
        static ComPtr<T> Make(Args&&... args)
        {
            ComPtr<T> result;
            result.m_ptr = new U(std::forward<Args>(args)...);
            return result;
        }

        template<
            class U,
            typename = typename std::enable_if<
                std::is_same<U,T>::value
            >::type
        >
        ComPtr(U* ptr) : m_ptr(ptr) { InternalAddRef(); }

        // copy ctor
        ComPtr(const ComPtr& right) : m_ptr(right.m_ptr) { InternalAddRef(); }

        // move ctor
        ComPtr(ComPtr &&right) : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<ComPtr*>(&reinterpret_cast<std::int8_t&>(right)))
            {   Swap(right);
            }
        }

        // Assignment operator for = nullptr
        ComPtr& operator=(std::nullptr_t)
        {
            InternalRelease();
            return *this;
        }

        ComPtr& operator=(ComPtr &&right)
        {
            ComPtr(std::move(right)).Swap(*this);
            return *this;
        }

        // Assignment operator...
        ComPtr& operator=(const ComPtr& right)
        {
            if (m_ptr != right.m_ptr) { ComPtr(right).Swap(*this); }
            return *this;
        }

        // conversion-to-bool to remove/avoid usage of nullptr == _x_.Get() checks.
        inline explicit operator bool() const { return m_ptr != nullptr; }

        ~ComPtr() { InternalRelease(); }

        inline T* operator->() const { return m_ptr; }
        inline T* Get() const { return m_ptr; }

        T* Detach() 
        {   T* temp = m_ptr;
            m_ptr = nullptr;
            return temp;
        }

        T** operator&()
        {   InternalRelease();
            return &m_ptr;
        }

        template <class U>
        ComPtr<U> As() const
        {
            ComPtr<U> out;
            ThrowHrIfFailed(m_ptr->QueryInterface(UuidOfImpl<U>::iid, reinterpret_cast<void**>(&out)));
            return out;
        }
    protected:
        T* m_ptr = nullptr;

        void InternalRelease()
        {   
            T* temp = m_ptr;
            if (temp)
            {   m_ptr = nullptr;
                temp->Release();
            }
        }

        inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
        inline void Swap(ComPtr& right ) { std::swap(m_ptr, right.m_ptr); }
    };

    template <class Derived, typename... Interfaces>
    class ComClass : public QIHelper<Interfaces...>
    {
    public:
        virtual ~ComClass() { }

        virtual ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++m_ref; }
        virtual ULONG STDMETHODCALLTYPE Release() noexcept override
        {
            if (--m_ref == 0)
            {   delete this;
                return 0;
            }
            return m_ref;
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) noexcept override
        {
            if (ppvObject == nullptr || *ppvObject != nullptr)
            {
                return static_cast<HRESULT>(Error::InvalidParameter);
            }
            //ThrowErrorIf(Error::InvalidParameter, (ppvObject == nullptr || *ppvObject != nullptr), "bad pointer");
            *ppvObject = nullptr;
            if (QIHelper<Interfaces...>::Matches(riid, ppvObject))
            {
                AddRef();
                return S_OK;
            }
            if (riid == UuidOfImpl<IUnknown>::iid)
            {
                *ppvObject = static_cast<void*>(reinterpret_cast<IUnknown*>(this));
                AddRef();
                return S_OK;
            }
            return static_cast<HRESULT>(MSIX::Error::NoInterface);
        }

    protected:
        std::atomic<std::uint32_t> m_ref;
        ComClass() : m_ref(1) {}
    };
}
