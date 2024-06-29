#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
#include <numeric>
#include <unordered_set>

template <class T> struct memory_pool {

  struct node {
    T *data;
    node *next{nullptr};
  };

public:
  memory_pool(size_t chunk_size, size_t max_chunks = std::numeric_limits<size_t>::max())
      : chunk_size_(chunk_size), max_chunks_(max_chunks) {
    free_ = new node;
    free_->data = new T[chunk_size];
  }

  ~memory_pool() {
    while (free_ != nullptr) {
      auto next = free_->next;
      delete[] free_->data;
      delete free_;
      free_ = next;
    }
  }

  T *allocate(size_t n) {
    if (n != chunk_size_) {
      throw std::bad_array_new_length();
    }
    if (free_ == nullptr) {
      if (allocated_size_ == max_chunks_) {
        throw std::bad_array_new_length();
      }
      size_t new_size = std::min(2 * allocated_size_, max_chunks_);
      for (size_t i = allocated_size_; i < new_size; ++i) {
        auto new_node = new node;
        new_node->data = new T[chunk_size_];
        if (free_ == nullptr) {
          free_ = new_node;
        } else {
          new_node->next = free_;
          free_ = new_node;
        }
      }
    }
    auto p = free_->data;
    auto tmp = free_;
    free_ = free_->next;
    tmp->next = allocated_;
    allocated_ = tmp;
    ++allocated_size_;
    return p;
  }

  void deallocate(T *p, size_t n) noexcept {
    if (n != chunk_size_ || allocated_size_ == 0) {
      return;
    }
    auto tmp = allocated_;
    allocated_ = allocated_->next;
    tmp->next = free_;
    free_ = tmp;
    free_->data = p;
    --allocated_size_;
  }

private:
  size_t chunk_size_;
  size_t max_chunks_;
  node *free_{nullptr};
  node *allocated_{nullptr};
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
