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
        typedef std::unique_ptr<Derived> XplatAppxPtr;

        virtual HRESULT Initialize() { return S_OK; }

        inline static HRESULT Make(Derived** result)
        {
            *result = nullptr;
            HRESULT hr = S_OK;
            XplatAppxPtr item;
            hr = xPlat::ResultOf(item, [&]() { item = std::make_unique<Derived>(); });
            if (SUCCEDED(hr))
            {
                hr = item->Initialize();
            }
            *result = item.release();
            return hr;
        }

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
}