#pragma once
#include <type_traits>
#include <cstring>
#include <functional>

/*
    Inspired by Callback class from ARM mbed
*/

namespace hal
{
    namespace detail
    {
        template<typename>
        constexpr bool is_reference_wrapper_v = false;
        template<typename T>
        constexpr bool is_reference_wrapper_v<std::reference_wrapper<T> > = true;

        template<typename T, typename Type, typename T1, typename ... Args, typename std::enable_if_t<std::is_member_function_pointer<Type>::value, int > = 0 >
        constexpr decltype(auto) invoke_impl(Type T::* func, T1&& receiver, Args&&... args)
        {
            if constexpr(std::is_base_of<T, std::decay_t<T1> >::value)
                return (std::forward<T1>(receiver).*func)(std::forward<Args>(args)...);
            else if constexpr(is_reference_wrapper_v<std::decay_t<T1> >)
                return (receiver.get().*func)(std::forward<Args>(args)...);
            else
                return ((*std::forward<T1>(receiver)).*func)(std::forward<Args>(args)...);
        }

        template<typename F, typename ... Args>
        constexpr decltype(auto) invoke_impl(F&& func, Args&&... args)
        {
            return std::forward<F>(func)(std::forward<Args>(args)...);
        }

        /*template<typename R, typename F, typename ... Args>
        struct is_invocable_r :
            std::is_constructible<std::function<R(Args...)>, std::reference_wrapper<std::remove_reference_t<F> > >{};
        */

        template<typename ... Args>
        using is_invocable_r = std::is_invocable_r<Args...>;
    }

    template<typename F, typename ... Args>
    constexpr std::invoke_result_t<F(Args...)> invoke(F&& f, Args&&... args)
        noexcept(std::is_nothrow_invocable<F, Args...>::value)
    {
        return detail::invoke_impl(std::forward<F>(f), std::forward<Args>(args)...);
    }

    template<typename R, typename F, typename ... Args, typename std::enable_if_t< std::is_void<R>::value, int> = 0>
    R invoke_r(F&& f, Args&&... args)
    {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }

    template<typename R, typename F, typename ... Args, typename std::enable_if_t< !std::is_void<R>::value, int> = 0>
    R invoke_r(F&& f, Args&&... args)
    {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }

    template<typename Signature> class function;
    template<typename R, typename ... Args> class function<R(Args...)>;

    class function_base
    {
    public:
        struct holder
        {
            struct _class;
            void (_class::*func)(int);
            void *obj;
        };

        struct alignas(holder) [[gnu::may_alias]] store
        {
            char data[sizeof(holder)];
        };

        struct ops
        {
            void (*call)();
            void (*dtor)(store&);
            void (*copy)(store&, store&);
        };


        template<typename F, typename std::enable_if_t< std::is_trivially_copyable<F>::value, int> = 0 >
        constexpr static void target_copy(store& dest, store& src)
        {
            std::memcpy(&dest, &src, sizeof(store));
        }

        template<typename F, typename  std::enable_if_t< !std::is_trivially_copyable<F>::value, int > = 0 >
        constexpr static void target_copy(store& dest, store& src)
        {
            const F& f = reinterpret_cast<const F&>(src);
            new (&dest) F(f);
        }

        template<typename F, typename std::enable_if_t< std::is_trivially_destructible<F>::value, int> = 0 >
        constexpr static void target_dtor(store& s)
        {}

        template<typename F, typename std::enable_if_t< !std::is_trivially_destructible<F>::value, int> = 0 >
        constexpr static void target_dtor(store& s)
        {
            F &f = reinterpret_cast<F &>(s);
            f.~F();
        }

        const ops* m_ops;
        store m_storage;

        function_base(std::nullptr_t) noexcept : m_ops(nullptr){}
        function_base() = default;

        void clear() volatile
        {
            m_ops = nullptr;
        }

        void clear()
        {
            m_ops = nullptr;
        }

        void destroy() volatile
        {
            if(m_ops && m_ops->dtor)
                m_ops->dtor(const_cast<store&>(m_storage));
        }

        void destroy()
        {
            if(m_ops && m_ops->dtor)
                m_ops->dtor(m_storage);
        }

        void copy(function_base& other)
        {
            m_ops = other.m_ops;
            if(m_ops)
                m_ops->copy(m_storage, other.m_storage);
        }

        void copy(function_base& other) volatile
        {
            m_ops = other.m_ops;
            if(m_ops)
                m_ops->copy(const_cast<store&>(m_storage), other.m_storage);
        }
    };

    template<typename R, typename ... Args>
    class function<R(Args...)> : protected function_base
    {
        using call_type = R(const function_base*, Args...);
        using volatile_call_type = R(volatile function_base*, Args...);

        template<typename F>
        static R target_call(const function_base* base, Args ... args)
        {
            F& f = const_cast<F&>(reinterpret_cast<const F&>(base->m_storage));
            return invoke_r<R>(f, std::forward<Args>(args)...);
        }

        template<typename F>
        static R volatile_target_call(volatile function_base* base, Args ... args)
        {
            F& f = const_cast<F&>(reinterpret_cast<const F&>(base->m_storage));
            return invoke_r<R>(f, std::forward<Args>(args)...);
        }


        template <typename F, typename = std::enable_if_t<!std::is_lvalue_reference<F>::value>>
        void generate(F &&f)
        {
            static_assert(sizeof(function<R(Args...)>) == sizeof(function_base));

            static const ops _ops = {
                reinterpret_cast<void(*)()>(target_call<F>),
                target_dtor<F>,
                target_copy<F>
            };

            m_ops = &_ops;

            new (&m_storage) F(std::move(f));
        }

        template <typename F, typename = std::enable_if_t<!std::is_lvalue_reference<F>::value>>
        void generate(F &&f) volatile
        {
            static_assert(sizeof(function<R(Args...)>) == sizeof(function_base));

            static const ops _ops = {
                reinterpret_cast<void(*)()>(volatile_target_call<F>),
                function_base::target_dtor<F>,
                function_base::target_copy<F>
            };

            m_ops = &_ops;

            new (&m_storage) F(std::move(f));
        }

        

    public:

        function() noexcept : function_base(nullptr){}
        function(std::nullptr_t) noexcept : function(){}

        function(const function& other)
        {
            m_ops = other.m_ops;
            if(m_ops && m_ops->copy)
                m_ops->copy(m_storage, other.m_storage);
        }

        function(function&& other)
        {
            m_ops = std::move(other.m_ops);
            m_storage = std::move(other.m_storage);
        }


        template<typename Obj, typename Method, typename std::enable_if_t<std::is_invocable_r<R, Method, Obj, Args...>::value, int> = 0>
        function(Obj obj, Method m) : function_base()
        {
            generate([obj, m](Args... args){
                return invoke_r<R>(m, obj, std::forward<Args>(args)...);
            });
        }

        template<typename F, typename std::enable_if_t<
                std::is_invocable_r<R, F, Args...>::value &&
                !(std::is_pointer<F>::value && std::is_function<std::remove_pointer_t<F> >::value),
            int> = 0>
        function(F&& f)
        {
            static_assert(std::is_copy_constructible<F>::value);
            generate(std::move(f));
        }

        template<typename F, typename std::enable_if_t<
                std::is_invocable_r<R, F, Args...>::value &&
                std::is_pointer<F>::value && 
                std::is_function<std::remove_pointer_t<F> >::value,
            int> = 0>
        function(F f)
        {
            static_assert(std::is_copy_constructible<F>::value);
            if(f)
                generate(std::move(f));
            else
                clear();
        }

        ~function()
        {
            destroy();
        }
        R call(Args... args) volatile
        {
            auto op_call = reinterpret_cast<volatile_call_type*>(m_ops->call);

            if constexpr(std::is_integral<R>::value) // Allow empty callbacks with integral return types (returning 0)
                if(!m_ops || !op_call)
                    return static_cast<R>(0);

            if constexpr(std::is_void<R>::value) // Allow empty void(...) callbacks
            {
                if(m_ops && op_call)
                    op_call(this, args...);
                return;
            }    
            else
            {
                return op_call(this, args...);
            } 
        }

        R operator()(Args ... args)
        {
            return call(args...);
        }

        R operator()(Args ... args) volatile
        {
            return call(args...);
        }

        

        static R thunk(void* self, Args... args)
        {
            return static_cast<function*>(self)->call(args...);
        }

        void swap(function& that)
        {
            function temp(std::move(*this));
            *this = std::move(that);
            that = std::move(temp);
        }


        template<typename F, typename std::enable_if_t<
                std::is_invocable_r<R, F, Args...>::value &&
                !std::is_same<std::remove_reference_t<std::remove_cv_t<F>>, function>::value,
            int> = 0>
        function& operator=(F&& that)
        {
            function(std::forward<F>(that)).swap(*this);
            return *this;
        }

        template<typename F, typename std::enable_if_t<
                std::is_invocable_r<R, F, Args...>::value &&
                !std::is_same<std::remove_reference_t<std::remove_cv_t<F>>, function>::value,
            int> = 0>
        volatile function& operator=(F&& that) volatile
        {
            function(std::forward<F>(that)).swap(*this);
            return *this;
        }

        function& operator=(function&& that)
        {
            if(this != &that)
            {
                destroy();
                copy(that);
            }

            return *this;
        }

        volatile function& operator=(function&& that) volatile
        {
            if(this != &that)
            {
                destroy();
                copy(that);
            }

            return *this;
        }

        explicit operator bool()
        {
            return m_ops;
        }

        explicit operator bool() volatile
        {
            return m_ops;
        }


    };
}