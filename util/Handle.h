#pragma once

#pragma warning (disable : 4521)

#include <memory>

#define template_convertible_y template<typename Y, typename = typename std::enable_if<!std::is_same<Y,T>::value && std::is_convertible<Y *, T *>::value>::type>
#define template_downcast_y    template<typename Y, typename = typename std::enable_if<!std::is_same<Y,T>::value && !std::is_convertible<Y *, T *>::value && std::is_polymorphic<Y>::value && (std::is_base_of<T, Y>::value || std::is_base_of<Y, T>::value)>::type, typename = void>


namespace cpp
{

    template<typename T>
    class Handle 
        : public std::shared_ptr<T>
    {
    public:
        typedef T Type;
        typedef std::shared_ptr<T> ptr_t;
        typedef std::weak_ptr<T> weak_ptr_t;

        //  nullptr initializer
        Handle(std::nullptr_t) 
            : ptr_t(nullptr) {}

        //  copy shared_ptr
        Handle(ptr_t & copy)
            : ptr_t(copy) {}
        Handle(const ptr_t & copy)
            : ptr_t(copy) {}
        template_convertible_y Handle(std::shared_ptr<Y> & copy)
            : ptr_t(copy) {}
        template_downcast_y Handle(std::shared_ptr<Y> & copy)
            : ptr_t(std::dynamic_pointer_cast<T>(copy)) {}
        template_convertible_y Handle(const std::shared_ptr<Y> & copy)
            : ptr_t(copy) {}
        template_downcast_y Handle(const std::shared_ptr<Y> & copy)
            : ptr_t(std::dynamic_pointer_cast<T>(copy)) {}

        //  copy handle
        Handle(Handle<T> & copy)
            : ptr_t(copy.ptr()) {}
        Handle(const Handle<T> & copy)
            : ptr_t(copy.ptr()) {}
        template_convertible_y Handle(Handle<Y> & copy)
            : ptr_t(copy.ptr()) {}
        template_downcast_y Handle(Handle<Y> & copy)
            : ptr_t(std::dynamic_pointer_cast<T>(copy.ptr())) {}
        template_convertible_y Handle(const Handle<Y> & copy)
            : ptr_t(copy.ptr()) {}
        template_downcast_y Handle(const Handle<Y> & copy)
            : ptr_t(std::dynamic_pointer_cast<T>(copy.ptr())) {}

        //  move shared ptr
        Handle(ptr_t && move)
            : ptr_t(std::move(move)) {}
        template_convertible_y Handle(std::shared_ptr<Y> && move)
            : ptr_t(std::move(move)) {}
        template_downcast_y Handle(std::shared_ptr<Y> && move)
            : ptr_t(std::dynamic_pointer_cast<T>(move)) {}

        //  move handle
        Handle(Handle<T> && move)
            : ptr_t(std::move(move.ptr())) {}
        template_convertible_y Handle(Handle<Y> && move)
            : ptr_t(std::move(move.ptr())) {}
        template_downcast_y Handle(Handle<Y> && move)
            : ptr_t(std::dynamic_pointer_cast<T>(move)) {}

        //  make object
        template<class... argtypes>
        Handle(argtypes&&... args)
            : ptr_t(std::make_shared<T>(std::forward<argtypes>(args)...)) {}


        //  assign nullptr
        Handle<T> & operator=(std::nullptr_t)
            { reset(); return *this; }
        
        //  assign copy
        Handle<T> & operator=(const ptr_t & copy)
            { ptr_t::operator=(copy); return *this; }
        template_convertible_y Handle<T> & operator=(const std::shared_ptr<Y> & copy)
            { ptr_t::operator=(copy); return *this; }
        template_downcast_y Handle<T> & operator=(const std::shared_ptr<Y> & copy)
            { ptr_t::operator=(std::dynamic_pointer_cast<T>(copy)); return *this; }
        Handle<T> & operator=(const Handle<T> & copy)
            { ptr_t::operator=(copy.ptr()); return *this; }
        template_convertible_y Handle<T> & operator=(const Handle<Y> & copy)
            { ptr_t::operator=(copy.ptr()); return *this; }
        template_downcast_y Handle<T> & operator=(const Handle<Y> & copy)
            { ptr_t::operator=(std::dynamic_pointer_cast<T>(copy.ptr())); return *this; }
        
        //  assign move
        Handle<T> & operator=(ptr_t && move)
            { ptr_t::operator=(std::move(move)); return *this; }
        template_convertible_y Handle<T> & operator=(std::shared_ptr<Y> && move)
            { ptr_t::operator=(std::move(move)); return *this; }
        Handle<T> & operator=(Handle<T> && move)
            { ptr_t::operator=(std::move(move.ptr())); return *this; }
        template_convertible_y Handle<T> & operator=(Handle<Y> && move)
            { ptr_t::operator=(std::move(move.ptr())); return *this; }

        ptr_t & ptr()
            { return *this; }
        const ptr_t & ptr() const
            { return *this; }

        T & object()
            { return *ptr(); }
        const T & object() const
            { return *ptr(); }

        template<typename Y> bool instanceof() const
            { return Y(*this) != nullptr; }
    };

}
