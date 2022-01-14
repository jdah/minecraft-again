// MODIFIED BY jdh
// adds default constructor
// makes deleter an std::optional
//
// UniqueResource
// Copyright (c) 2015 okdshin
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
// This implementation is based on C++ standards committee paper N4189.
#ifndef UNIQUE_RESOURCE_H_
#define UNIQUE_RESOURCE_H_
#include <type_traits>
#include <utility>
#include <optional>
// workaround for GCC
#if defined(__GNUC__)
#ifdef UNIQUE_RESOURCE_ALLOW_DELETER_CALL_THROWING_EXCEPTION
#define UNIQUE_RESOURCE_NOEXCEPT
#else
#define UNIQUE_RESOURCE_NOEXCEPT noexcept
#endif
#define UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_RESET UNIQUE_RESOURCE_NOEXCEPT
#define UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_DELETER_CALL                    \
  UNIQUE_RESOURCE_NOEXCEPT
#else
#define UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_RESET                           \
  noexcept(noexcept(this->reset()))
#define UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_DELETER_CALL                    \
  noexcept(noexcept(this->get_deleter()(resource)))
#endif
namespace util {
template <typename R, typename D> class UniqueResource {
  R resource;
  std::optional<D> deleter;
  bool execute_on_destruction; // exposition only
  UniqueResource &operator=(UniqueResource const &) = delete;
  UniqueResource(UniqueResource const &) = delete; // no copies!
public:
  template <typename S = R>
      requires std::is_default_constructible<S>::value
  UniqueResource() noexcept
    : resource(R()),
      deleter(std::nullopt),
      execute_on_destruction(false) {}

  // construction
  explicit UniqueResource(R &&resource_, D &&deleter_,
                           bool shouldrun = true) noexcept
      : resource(std::move(resource_)),
        deleter(std::move(deleter_)),
        execute_on_destruction{shouldrun} {}
  // move
  UniqueResource(UniqueResource &&other) noexcept
      : resource(std::move(other.resource)),
        deleter(std::move(other.deleter)),
        execute_on_destruction{other.execute_on_destruction} {
    other.release();
  }
  UniqueResource &operator=(UniqueResource &&other)
      UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_RESET {
    this->reset();
    this->deleter = std::move(other.deleter);
    this->resource = std::move(other.resource);
    this->execute_on_destruction = other.execute_on_destruction;
    other.release();
    return *this;
  }
  // resource release
  ~UniqueResource() UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_RESET {
    this->reset();
  }
  void reset() UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_DELETER_CALL {
    if (execute_on_destruction) {
      this->execute_on_destruction = false;
      this->get_deleter()(resource);
    }
  }
  void reset(R &&newresource) UNIQUE_RESOURCE_NOEXCEPT_NOEXCEPT_THIS_RESET {
    this->reset();
    this->resource = std::move(newresource);
    this->execute_on_destruction = true;
  }
  R const &release() noexcept {
    this->execute_on_destruction = false;
    return this->get();
  }
  // resource access
  R const &get() const noexcept { return this->resource; }
  operator R const &() const noexcept { return this->resource; }
  R operator->() const noexcept { return this->resource; }
  typename std::add_lvalue_reference<
      typename std::remove_pointer<R>::type>::type
  operator*() const {
    return *this->resource;
  }
  // deleter access
  const D &get_deleter() const noexcept { return this->deleter.value(); }
};
// factories
template <typename R, typename D>
UniqueResource<R, typename std::remove_reference<D>::type>
make_unique_resource(R &&r, D &&d) noexcept {
  return UniqueResource<R, typename std::remove_reference<D>::type>(
      std::move(r), std::forward<typename std::remove_reference<D>::type>(d),
      true);
}
template <typename R, typename D>
UniqueResource<R, D> make_unique_resource_checked(R r, R invalid,
                                                   D d) noexcept {
  bool shouldrun = not bool(r == invalid);
  return UniqueResource<R, D>(std::move(r), std::move(d), shouldrun);
}

template <typename T>
using Deleter = std::function<void(T)>;

template <typename T>
using RDUniqueResource = UniqueResource<T, util::Deleter<T&>>;

template <typename T, typename D>
RDUniqueResource<T> make_rd_unique_resource(T &&t, D d) {
    return RDUniqueResource<T>(
        std::move(t),
        std::move(static_cast<util::Deleter<T&>>(d)));
}

}
#endif /* UNIQUE_RESOURCE_H_ */
