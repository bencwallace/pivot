#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
#include <numeric>
#include <unordered_set>

template <class T> struct memory_pool {

  struct node {
    node *next{nullptr};
  };

public:
  memory_pool(size_t chunk_size, size_t max_chunks = std::numeric_limits<size_t>::max())
      : chunk_size_(std::max(sizeof(node *), chunk_size)), max_chunks_(max_chunks) {}

  ~memory_pool() {
    while (free_ != nullptr) {
      auto next = free_->next;
      delete[] reinterpret_cast<char *>(free_);
      free_ = next;
    }
  }

  T *allocate(size_t n) {
    if (n > chunk_size_) {
      throw std::bad_array_new_length();
    }
    if (free_ == nullptr) {
      if (allocated_size_ == max_chunks_) {
        throw std::bad_array_new_length();
      }
      free_ = reinterpret_cast<node *>(new char[chunk_size_ * sizeof(T)]);
      free_->next = nullptr;
    }
    auto p = reinterpret_cast<T *>(free_);
    free_ = free_->next;
    ++allocated_size_;
    return p;
  }

  void deallocate(T *p, size_t n) noexcept {
    if (n > chunk_size_ || allocated_size_ == 0) {
      return;
    }
    auto tmp = reinterpret_cast<node *>(p);
    tmp->next = free_;
    free_ = tmp;
    --allocated_size_;
  }

private:
  size_t chunk_size_;
  size_t max_chunks_;
  node *free_{nullptr};
  size_t allocated_size_{0};
};

template <class T> struct pool_allocator {
  typedef T value_type;

  pool_allocator(size_t chunk_size) : chunk_size_(chunk_size) {}

  template <class U> constexpr pool_allocator(const pool_allocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    auto p = get_pool(chunk_size_).allocate(n);
    return p;

    throw std::bad_alloc();
  }

  void deallocate(T *p, std::size_t n) noexcept { get_pool(chunk_size_).deallocate(p, n); }

private:
  size_t chunk_size_;

  static memory_pool<T> &get_pool(size_t chunk_size) {
    static memory_pool<T> pool(chunk_size);
    return pool;
  }
};

template <class T, class U> bool operator==(const pool_allocator<T> &, const pool_allocator<U> &) { return true; }

template <class T, class U> bool operator!=(const pool_allocator<T> &, const pool_allocator<U> &) { return false; }
