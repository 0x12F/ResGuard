#ifndef RESGUARD_HPP
#define RESGUARD_HPP

#include <memory>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <cstddef>

namespace safe
{
    template <class T>
    class [[nodiscard]] unique_resource
    {
        static_assert(!std::is_pointer_v<T>, "raw pointer types are not allowed");

        T value_;
        bool valid_{true};

        using deleter_type = void (*)(T&) noexcept;

        deleter_type deleter_{nullptr};

        void release() noexcept
        {
            if (valid_ && deleter_)
            {
                deleter_(value_);
                valid_ = false;
            }
        }

    public:
        unique_resource() = delete;
        unique_resource(std::nullptr_t) = delete;

        template <class U>
        explicit unique_resource(U&& res, deleter_type d) noexcept(std::is_nothrow_constructible_v<T, U&&>)
            : value_{std::forward<U>(res)}, deleter_{d}
        {
            if (!d) std::terminate();
        }

        ~unique_resource() noexcept
        {
            release();
        }

        unique_resource(unique_resource&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
            : value_{std::move(other.value_)},
              valid_{std::exchange(other.valid_, false)},
              deleter_{other.deleter_}
        {
        }

        unique_resource& operator=(unique_resource&& other) noexcept(std::is_nothrow_move_assign_v<T>)
        {
            if (this != &other)
            {
                release();
                value_   = std::move(other.value_);
                valid_   = std::exchange(other.valid_, false);
                deleter_ = other.deleter_;
            }
            return *this;
        }

        unique_resource(const unique_resource&)            = delete;
        unique_resource& operator=(const unique_resource&) = delete;

        T& get() noexcept
        {
            return value_;
        }

        const T& get() const noexcept
        {
            return value_;
        }

        T* operator->() noexcept
        {
            return &value_;
        }

        const T* operator->() const noexcept
        {
            return &value_;
        }

        explicit operator bool() const noexcept
        {
            return valid_;
        }

        void reset() noexcept
        {
            release();
        }

        T release_ownership() noexcept
        {
            valid_ = false;
            return std::move(value_);
        }

        void swap(unique_resource& other) noexcept(std::is_nothrow_swappable_v<T>)
        {
            using std::swap;
            swap(value_, other.value_);
            swap(valid_, other.valid_);
            swap(deleter_, other.deleter_);
        }
    };

    template <class T, class D>
    [[nodiscard]] auto make_unique_resource(T&& res, D&& deleter) noexcept(noexcept(deleter(std::declval<T&>())))
    {
        using decayed_t = std::decay_t<T>;
        using deleter_fn = void (*)(decayed_t&) noexcept;

        deleter_fn fn = +[](decayed_t& v) noexcept
        {
            try
            {
                std::forward<D>(deleter)(v);
            }
            catch (...)
            {
                std::terminate();
            }
        };

        return unique_resource<decayed_t>{std::forward<T>(res), fn};
    }

    template <class T>
    void swap(unique_resource<T>& a, unique_resource<T>& b) noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }
}

#endif
