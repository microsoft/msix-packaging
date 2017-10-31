#pragma once
#include <memory> 
#include <atomic>

#include "Exceptions.hpp"
#include "AppxPackaging.hpp"

namespace xPlat {

    template <typename ...Interfaces>
    struct QIHelper;

    // Specialization
    template <typename I0, typename ...Interfaces>
    struct QIHelper<I0, Interfaces...> : I0, QIHelper<Interfaces...>
    {
        bool IsIIDAMatch(REFIID riid)
        {
            if (riid == UuidOfImpl<I0>::iid)
            {
                return true;
            }
            return QIHelper<Interfaces...>::IsIIDAMatch(riid);
        }
    };

    // Terminal case, always returns false
    template <>
    struct QIHelper<>
    {
        bool IsIIDAMatch(REFIID /*riid*/)
        {
            return false;
        }
    };

    template <class Derived, typename... Interfaces>
    class ComClass : public QIHelper<Interfaces...>
    {
    public:
        virtual ~ComClass() { }

        virtual ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }

        virtual ULONG STDMETHODCALLTYPE Release() override
        {
            if (--m_ref == 0)
            {
                delete this;
                return 0;
            }
            return m_ref;
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
        {
            *ppvObject = nullptr;
            if (riid == UuidOfImpl<IUnknown>::iid || QIHelper<Interfaces...>::IsIIDAMatch(riid))
            {
                *ppvObject = static_cast<void*>(this);
                AddRef();
                return S_OK;
            }
            xPlat::Exception e = xPlat::Exception(xPlat::Error::NoInterface);
            return e.Code();
        }

    protected:
        std::atomic<std::uint32_t> m_ref;

        ComClass() : m_ref(1) {}
    };

    struct ComDeleter
    {
        operator()(IUnknown* unknown)
        {
            unknown->Release();
        }
    };
    
    template <T>
    class ComPtr : public std::unique_ptr<T, ComDeleter>
    {
    public:
        template <U>
        HRESULT As (xPlatComPtr<U>& qi)
        {
            UuidOfImpl<U> uuid;
            qi.release();
            return this->QueryInterface(uuid.idd, &qi.get());
        }
    };
}