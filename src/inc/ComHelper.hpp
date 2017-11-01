#pragma once
#include <memory> 
#include <atomic>
#include <type_traits>
#include <utility>

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

    template <class T>
    class ComPtr
    {
    public:
        ComPtr() : m_ptr(nullptr) {}
        ComPtr(ComPtr& right) : m_ptr(right.Detach()) { }
        ComPtr(const ComPtr& right)
        {
            m_ptr = right.m_ptr;
            m_ptr->AddRef();
        }

        template<
            class U, 
            typename = typename std::enable_if<
                std::is_convertible<U,T>::value ||
                std::is_same<U,T>::value
                >::type>
        ComPtr(U* ptr) : m_ptr(ptr) {}

        ~ComPtr() { InternalRelease(); }

        T* operator->() const { return m_ptr; }
        T* Get() const { return m_ptr; }
        void Reset() { InternalRelease(); }

        T** AddressOf()
        {
            InternalRelease();
            return &m_ptr;
        }

        T* Detach()
        {
            T* ptr = m_ptr;
            m_ptr = nullptr;
            return ptr;
        }

        template <class U>
        ComPtr<U> As()
        {
            UuidOfImpl<U> uuid;
            ComPtr<U> out;
            ThrowHrIfFailed(m_ptr->QueryInterface(uuid.iid, reinterpret_cast<void**>(out.AddressOf())));
            return out;
        }
    protected:
        T* m_ptr = nullptr;

        void InternalRelease()
        {   if (m_ptr)
            {   m_ptr->Release();
                m_ptr = nullptr;
            }
        }
    };

    template <class Derived, typename... Interfaces>
    class ComClass : public QIHelper<Interfaces...>
    {
    public:
        virtual ~ComClass() { }

        //template<class T, typename... Args>
        //static ComPtr<T> Make(Args&&... args)
        //{
        //    return ComPtr<T>(new Derived(std::forward<Args>(args...)));
        //}

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