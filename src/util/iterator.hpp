#ifndef UTIL_ITERATOR_HPP
#define UTIL_ITERATOR_HPP

#include <cstddef>
#include <optional>
#include <functional>
#include <unordered_map>
#include <variant>

#include "util/types.hpp"

namespace util {

// iterator type over arbitrary collections - do not instantiate directly,
// see helpers below class (util::iter, util::iter_keys, etc.)
template <typename T, typename V = typename std::add_pointer<T>::type>
class Iterator {
    using Provider = std::function<V(bool)>;

    Provider p;

    struct Wrapper {
        Iterator<T> iter;
        typename Iterator<T>::iterator it, end;

        Wrapper(Iterator<T> iter) {
            this->iter = iter;
            this->it = this->iter.begin();
            this->end = this->iter.end();
        }
    };
public:
    Iterator() : p([](bool) { return nullptr; }) {}
    Iterator(Provider p) : p(p) {}

    struct iterator {
        Provider p;
        V v;

        iterator()
            : p([](bool){ return nullptr; }), v(nullptr) {}

        iterator(V v)
            : p([](bool){ return nullptr; }), v(v) {}

        iterator(Provider q) : p(q) {
            this->v = this->p(false);
        }

        iterator operator++(int) { auto it = *this; ++*this; return it; }
        iterator &operator++() { v = p(true); return *this; }
        bool operator==(iterator other) const { return v == other.v; }
        bool operator!=(iterator other) const { return !(*this == other); }
        T &operator*() { return *v; }
        T *&raw() { return v; }
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::forward_iterator_tag;
    };

    auto begin() { return iterator(p); }
    auto end() { return iterator(nullptr); }

    template <typename U>
    inline auto map(std::function<U(T *)> f) {
        return Iterator<U>(
            [=,
             w = Wrapper(*this),
             u = std::optional<U>()](bool advance) mutable {
                if (advance) {
                    ++w.it;
                }

                return w.it != w.end ? &*(u = f(w.it.raw())) : nullptr;
            });
    }

    inline auto ptr() {
        return this->map<T*>([](T *t) -> T* { return t; });
    }

    inline auto ref() {
        return this->map<std::reference_wrapper<T>>(
            [](T *t) -> std::reference_wrapper<T> { return std::ref(*t); });
    }

    template <typename U = typename std::remove_pointer<T>::type>
        requires std::is_pointer<T>::value
    inline auto deref() {
        return this->map<U>([](U **u) -> U { return **u; });
    }

    template <typename C, typename E = typename C::value_type>
    static inline auto from_container(C &c) {
        return Iterator<E>(
            [it = c.begin(), end = c.end()](bool advance) mutable -> E* {
                if (advance) {
                    ++it;
                }

                return it != end ? &(*it) : nullptr;
            });
    }
};

template <
    typename V,
    typename T = typename V::value_type>
    requires is_vector<V>::value
inline auto iter(V &v) {
    return Iterator<T>::from_container(v);
}

template <
    typename M,
    typename K = typename M::key_type,
    typename P = typename M::value_type>
inline auto iter_keys(M &m) {
    return
        Iterator<P>::from_container(m)
            .template map<K*>([](P *p) -> K* { return &p->first; });
}

template <
    typename M,
    typename V = typename M::mapped_type,
    typename P = typename M::value_type>
inline auto iter_values(M &m) {
    return
        Iterator<P>::from_container(m)
            .template map<V*>([](P *p) -> V* { return &p->second; });
}

}

#endif
