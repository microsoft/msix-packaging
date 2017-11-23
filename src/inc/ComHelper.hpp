#pragma once
#include <cstddef>
#include <memory> 
#include <atomic>
#include <type_traits>
#include <utility>

#include "Exceptions.hpp"
#include "AppxPackaging.hpp"
#include "xercesc/util/XMLString.hpp"

namespace xPlat {

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
                *ppvObject = static_cast<void*>(static_cast<I0*>(this));
                return true;
            }
            return QIHelper<Interfaces...>::Matches(riid, ppvObject);
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

        ~ComPtr() { InternalRelease(); }

        inline T* operator->() const { return m_ptr; }
        inline T* Get() const { return m_ptr; }
        
        inline T* Detach() 
        {   T* temp = m_ptr;
            m_ptr = nullptr;
            return temp;
        }

        inline T** operator&()
        {   InternalRelease();
            return &m_ptr;
        }

        template <class U>
        inline ComPtr<U> As()
        {   
            ComPtr<U> out;
            ThrowHrIfFailed(m_ptr->QueryInterface(UuidOfImpl<U>::iid, reinterpret_cast<void**>(&out)));
            return out;
        }
    protected:
        T* m_ptr = nullptr;

        inline void InternalRelease()
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

        virtual ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
        virtual ULONG STDMETHODCALLTYPE Release() override
        {   
            if (--m_ref == 0)
            {   delete this;
                return 0;
            }
            return m_ref;
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter, (ppvObject == nullptr || *ppvObject != nullptr), "bad pointer");
                *ppvObject = nullptr;
                if (QIHelper<Interfaces...>::Matches(riid, ppvObject))
                {
                    AddRef();
                    return S_OK;
                }
                throw xPlat::Exception(xPlat::Error::NoInterface);
            });
        }

    protected:
        std::atomic<std::uint32_t> m_ref;
        ComClass() : m_ref(1) {}
    };

    template<class T>
    class XercesPtr
    {
    public:
        XercesPtr() : m_ptr(nullptr) {}
        XercesPtr(T* p)  : m_ptr(p) {}

        // move ctor
        XercesPtr(XercesPtr &&right) : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<XercesPtr*>(&reinterpret_cast<std::int8_t&>(right)))
            {   Swap(right);
            }
        } 

        virtual ~XercesPtr() { InternalRelease(); }

        void InternalRelease()
        {
            T* temp = m_ptr;
            if (temp)
            {
                m_ptr = nullptr;
                temp->release();
            }
        }
        
        XercesPtr& operator=(XercesPtr&& right)
        {   XercesPtr(std::move(right)).Swap(*this);
            return *this;
        }

        T* operator->() const { return m_ptr; }
        T* Get() const { return m_ptr; }
    protected:
        inline void Swap(XercesPtr& right ) { std::swap(m_ptr, right.m_ptr); }
        T* m_ptr = nullptr;
    };

    class XercesCharPtr
    {
    public:
        XercesCharPtr(char* c) : m_ptr(c) {};
        ~XercesCharPtr() { InternalRelease(); }

        // move ctor
        XercesCharPtr(XercesCharPtr &&right) : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<XercesCharPtr*>(&reinterpret_cast<std::int8_t&>(right)))
            {   Swap(right);
            }
        } 

        void InternalRelease()
        {   XERCES_CPP_NAMESPACE::XMLString::release(&m_ptr);
            m_ptr = nullptr;
        }

        XercesCharPtr& operator=(XercesCharPtr&& right)
        {   XercesCharPtr(std::move(right)).Swap(*this);
            return *this;
        }

        char* operator->() const { return m_ptr; }
        char* Get() const { return m_ptr; }
    protected:
        inline void Swap(XercesCharPtr& right ) { std::swap(m_ptr, right.m_ptr); }
        char* m_ptr = nullptr;        
    };

    class XercesXMLChPtr
    {
    public:
        XercesXMLChPtr(XMLCh* c) : m_ptr(c) {}
        ~XercesXMLChPtr() { InternalRelease(); }

        // move ctor
        XercesXMLChPtr(XercesXMLChPtr &&right) : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<XercesXMLChPtr*>(&reinterpret_cast<std::int8_t&>(right)))
            {   Swap(right);
            }
        }         

        void InternalRelease()
        {   XERCES_CPP_NAMESPACE::XMLString::release(&m_ptr);
            m_ptr = nullptr;            
        }

        XercesXMLChPtr& operator=(XercesXMLChPtr&& right)
        {   XercesXMLChPtr(std::move(right)).Swap(*this);
            return *this;
        }

        XMLCh* operator->() const { return m_ptr; }
        XMLCh* Get() const { return m_ptr; }
    protected:
        inline void Swap(XercesXMLChPtr& right ) { std::swap(m_ptr, right.m_ptr); }
        XMLCh* m_ptr = nullptr;              
    };

    class XercesXMLBytePtr
    {
    public:
        XercesXMLBytePtr(XMLByte* c) : m_ptr(c) {}
        ~XercesXMLBytePtr() { InternalRelease(); }

        // move ctor
        XercesXMLBytePtr(XercesXMLBytePtr &&right) : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<XercesXMLBytePtr*>(&reinterpret_cast<std::int8_t&>(right)))
            {   Swap(right);
            }
        }  

        void InternalRelease()
        {   delete(m_ptr);
            m_ptr = nullptr;
        }

        XercesXMLBytePtr& operator=(XercesXMLBytePtr&& right)
        {   XercesXMLBytePtr(std::move(right)).Swap(*this);
            return *this;
        }

        XMLByte* operator->() const { return m_ptr; }
        XMLByte* Get() const { return m_ptr; }
    protected:
        inline void Swap(XercesXMLBytePtr& right ) { std::swap(m_ptr, right.m_ptr); }
        XMLByte* m_ptr = nullptr;             
    };
}