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
        // default ctor
        ComPtr() : m_ptr(nullptr) {}

        // For use via ComPtr<T> t(new Foo(...)); where Foo : public T
        template<
            class U, 
            typename = typename std::enable_if<
                std::is_convertible<U,T>::value
            >::type
        >
        ComPtr(U* ptr) : m_ptr(ptr)
        {
            InternalAddRef();
        }

        // copy ctor
        ComPtr(const ComPtr& right) : m_ptr(right.m_ptr)
        {
            InternalAddRef();
        }

        // copy ctor that allows instantiation of class when U* is convertible to T*
        template<
            class U, 
            typename = typename std::enable_if<
                std::is_convertible<U,T>::value
            >::type
        >
        ComPtr(const ComPtr<U>& right) : m_ptr(right.m_ptr)
        {
            InternalAddRef();
        }

        // move ctor
        ComPtr(ComPtr &&right) : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<ComPtr*>(&reinterpret_cast<std::int8_t&>(right)))
            {
                Swap(right);
            }
        }

        // move ctor that allows instantiation of a class when U* is convertible to T*
        template<
            class U, 
            typename = typename std::enable_if<
                std::is_convertible<U,T>::value
            >::type
        >
        ComPtr(ComPtr &&right) : m_ptr(right.m_ptr)
        {
            right.m_ptr = nullptr;
        }

        // Assignment operator for = nullptr
        ComPtr& operator=(decltype(__nullptr))
        {
            InternalRelease();
            return *this;
        }

        // Assignment operator... VERY important.
        ComPtr& operator=(const ComPtr& right)
        {
            if (m_ptr != right.m_ptr)
            {
                ComPtr(right).Swap(*this);
            }          
            return *this;
        }

        // Assignment operator of T*
        ComPtr& operator=(T* right)
        {
            if (m_ptr != right)
            {
                ComPtr(right).Swap(*this);
            }
            return *this;
        }

        // Assignment operator when U* is convertible to T*
        template<
            class U, 
            typename = typename std::enable_if<
                std::is_convertible<U,T>::value
            >::type
        >
        ComPtr& operator=(U* right)
        {
            ComPtr(right).Swap(*this);
            return *this;
        }

        ComPtr& operator=(ComPtr &&right)
        {
            ComPtr(static_cast<ComPtr&&>(right)).Swap(*this);
            return *this;
        }

        ~ComPtr() { InternalRelease(); }

        inline T* operator->() const { return m_ptr; }
        inline T* Get() const { return m_ptr; }
        inline void Reset() { InternalRelease(); }

        inline T** AddressOf()
        {
            InternalRelease();
            return &m_ptr;
        }

        inline T* Detach()
        {
            T* ptr = m_ptr;
            m_ptr = nullptr;
            return ptr;
        }

        template <class U>
        inline ComPtr<U> As()
        {
            UuidOfImpl<U> uuid;
            ComPtr<U> out;
            ThrowHrIfFailed(m_ptr->QueryInterface(uuid.iid, reinterpret_cast<void**>(out.AddressOf())));
            return out;
        }
    protected:
        T* m_ptr = nullptr;

        inline void InternalRelease()
        {   T* temp = m_ptr;
            if (temp)
            {   m_ptr = nullptr;
                temp->Release();
            }
        }

        inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
        inline void Swap(ComPtr&& right) { std::swap(m_ptr, right.m_ptr); }
        inline void Swap(ComPtr& right ) { std::swap(m_ptr, right.m_ptr); }
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