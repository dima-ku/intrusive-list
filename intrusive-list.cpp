#include "intrusive-list.h"

namespace intrusive::details {

list_element_base::list_element_base() noexcept : prev(this), next(this) {}

list_element_base& list_element_base::operator=(const list_element_base& other) noexcept {
  if (this == &other) {
    return *this;
  }
  unlink();
  return *this;
}

bool list_element_base::is_linked() const noexcept {
  return prev != this;
}

list_element_base& list_element_base::operator=(list_element_base&& other) {
  if (this == &other) {
    return *this;
  }
  unlink();
  if (other.is_linked()) {
    prev = std::exchange(other.prev, &other);
    next = std::exchange(other.next, &other);
    prev->next = this;
    next->prev = this;
  }
  return *this;
}

list_element_base::list_element_base(list_element_base&& other) : list_element_base() {
  *this = std::move(other);
}

void list_element_base::link(list_element_base* a, list_element_base* b) {
  a->next = b;
  b->prev = a;
}

void list_element_base::unlink() noexcept {
  link(prev, next);
  prev = next = this;
}

list_element_base::list_element_base(const list_element_base&) noexcept : list_element_base() {}

list_element_base::~list_element_base() = default;
} // namespace intrusive::details
