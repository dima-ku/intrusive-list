#pragma once

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <utility>

namespace intrusive {

namespace details {
class default_tag;

struct list_element_base {
  list_element_base() noexcept;

  list_element_base([[maybe_unused]] const list_element_base&) noexcept;

  list_element_base& operator=(const list_element_base& other) noexcept;

  bool is_linked() const noexcept;

  list_element_base& operator=(list_element_base&& other);

  list_element_base(list_element_base&& other);

  static void link(list_element_base* a, list_element_base* b);

  void unlink() noexcept;

  ~list_element_base();

  list_element_base* prev;
  list_element_base* next;
};
} // namespace details

template <typename Tag = details::default_tag>
struct list_element : private details::list_element_base {
  template <typename T, typename AnyTag>
  friend class list;
};

template <typename T, typename Tag = details::default_tag>
struct list {
  static_assert(std::is_base_of_v<list_element<Tag>, T>, "T must derive from list_element");

public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using difference_type = std::ptrdiff_t;

private:
  using elem = list_element<Tag>;
  using base_elem = details::list_element_base;

  static elem& as_elem(reference value) {
    return static_cast<elem&>(value);
  }

  static base_elem* as_base(reference val) {
    return static_cast<base_elem*>(static_cast<elem*>(&val));
  }

private:
  template <class S>
  struct basic_iterator {
    friend list;

    using value_type = T;
    using reference = S&;
    using pointer = S*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    basic_iterator() noexcept : current(nullptr) {}

    basic_iterator& operator++() {
      current = current->next;
      return *this;
    }

    basic_iterator operator++(int) {
      basic_iterator res = *this;
      operator++();
      return res;
    }

    basic_iterator& operator--() {
      current = current->prev;
      return *this;
    }

    basic_iterator operator--(int) {
      basic_iterator res = *this;
      operator--();
      return res;
    }

    reference operator*() const {
      return *static_cast<pointer>(static_cast<elem*>(current));
    }

    pointer operator->() const {
      return static_cast<pointer>(static_cast<elem*>(current));
    }

    friend bool operator==(const basic_iterator lhs, const basic_iterator rhs) noexcept {
      return lhs.current == rhs.current;
    }

    friend bool operator!=(const basic_iterator lhs, const basic_iterator rhs) noexcept {
      return lhs.current != rhs.current;
    }

    operator basic_iterator<const S>() const {
      return basic_iterator<const S>{current};
    }

  private:
    explicit basic_iterator(const base_elem* smth) noexcept : current(const_cast<base_elem*>(smth)) {}

    elem& get_elem() {
      return *static_cast<elem*>(current);
    }

    base_elem* get_base() {
      return current;
    }

    base_elem* current;
  };

public:
  using iterator = basic_iterator<value_type>;
  using const_iterator = basic_iterator<const value_type>;

  // O(1)
  list() noexcept = default;

  // O(1)
  ~list() = default;

  list(const list&) = delete;
  list& operator=(const list&) = delete;

  // O(1)
  list(list&& other) noexcept = default;

  // O(1)
  list& operator=(list&& other) noexcept = default;

  // O(1)
  bool empty() const noexcept {
    return !dummy.is_linked();
  }

  // O(n)
  std::size_t size() const noexcept {
    return std::distance(begin(), end());
  }

  // O(1)
  reference front() noexcept {
    return *begin();
  }

  // O(1)
  const_reference front() const noexcept {
    return *begin();
  }

  // O(1)
  reference back() noexcept {
    return *(std::prev(end()));
  }

  // O(1)
  const_reference back() const noexcept {
    return *(std::prev(end()));
  }

  // O(1)
  void push_front(reference value) noexcept {
    insert(begin(), value);
  }

  // O(1)
  void push_back(reference value) noexcept {
    insert(end(), value);
  }

  // O(1)
  void pop_front() noexcept {
    erase(begin());
  }

  // O(1)
  void pop_back() noexcept {
    erase(std::prev(end()));
  }

  // O(1)
  void clear() noexcept {
    dummy.unlink();
  }

  // O(1)
  iterator begin() noexcept {
    return iterator(dummy.next);
  }

  // O(1)
  const_iterator begin() const noexcept {
    return const_iterator(dummy.next);
  }

  // O(1)
  iterator end() noexcept {
    return iterator(static_cast<base_elem*>(&dummy));
  }

  // O(1)
  const_iterator end() const noexcept {
    return const_iterator(static_cast<const base_elem*>(&dummy));
  }

  // O(1)
  iterator insert(const_iterator pos, reference value) noexcept {
    if (pos == const_iterator(as_base(value))) {
      return iterator(pos.get_base());
    }
    as_elem(value).unlink();
    base_elem::link(pos.get_base()->prev, as_base(value));
    base_elem::link(as_base(value), pos.get_base());
    return iterator(as_base(value));
  }

  // O(1)
  iterator erase(const_iterator pos) noexcept {
    auto nxt = pos.get_base()->next;
    pos.get_elem().unlink();
    return iterator(nxt);
  }

  // O(1)
  void splice(const_iterator pos, [[maybe_unused]] list& other, const_iterator first,
              const_iterator last_exclusive) noexcept {
    if (first == last_exclusive) {
      return;
    }
    const_iterator last = std::prev(last_exclusive);
    auto before_pos = pos.get_base()->prev;
    auto before_first = first.get_base()->prev;
    auto after_last = last.get_base()->next;
    base_elem::link(before_pos, first.get_base());
    base_elem::link(last.get_base(), pos.get_base());
    base_elem::link(before_first, after_last);
  }

private:
  elem dummy;
};

} // namespace intrusive
