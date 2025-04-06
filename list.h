#pragma once
#include <iterator>
#include <memory>
#include <type_traits>

template <typename T, typename Alloc = std::allocator<T>>
class list {
  struct BaseNode {
    BaseNode* next;
    BaseNode* prev;

    BaseNode() = default;
    BaseNode(BaseNode* next, BaseNode* prev): next(next), prev(prev) {}
  };

  struct Node : BaseNode {
    T data;

    template <typename... Args>
    Node(BaseNode* next, BaseNode* prev, Args&&... args): BaseNode(next, prev), data(std::forward<Args>(args)...) {}
  };
  

  // шаблонный параметр isConst говорит о том, является ли итератор константным
  template <bool isConst>
  class base_iterator {
  public:
    using reference_type = std::conditional_t<isConst, const T&, T&>;
    using pointer_type = std::conditional_t<isConst, const T*, T*>;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
  
  private:

    BaseNode* ptr;
    base_iterator(BaseNode* ptr): ptr(ptr) {}
    base_iterator(const BaseNode* ptr): ptr(const_cast<BaseNode*>(ptr)) {}

    friend class list<T, Alloc>;
  public:
    base_iterator() = default;
    base_iterator(const base_iterator&) = default;
    base_iterator& operator=(const base_iterator&) = default;
    bool operator==(const base_iterator&) const = default; 

    reference_type operator*() const { return static_cast<Node*>(ptr)->data; }
    pointer_type operator->() const { return &(static_cast<Node*>(ptr)->data); }

    base_iterator& operator++() {
      ptr = ptr->next;
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator copy = *this;
      ptr = ptr->next;
      return copy;
    }

    base_iterator& operator--() {
      ptr = ptr->prev;
      return *this;
    }

    base_iterator operator--(int) {
      base_iterator copy = *this;
      ptr = ptr->prev;
      return copy;
    }

    operator base_iterator<true>() const {
      return {ptr};
    }
  };

  using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>; 

  void swap(list& other);

  BaseNode fakeNode_; // fakeNode_.next -> start of the list, fakeNode_.prev -> end of the list
  node_allocator alloc_; // allocator for Node
  size_t sz_;

public:
 
  using value_type = T;
  using allocator_type = Alloc;

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end() const;

  const_iterator cbegin() const;
  const_iterator cend() const;

  reverse_iterator rbegin();
  reverse_iterator rend();

  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;

  list(): fakeNode_{ &fakeNode_, &fakeNode_ }, alloc_(node_allocator()), sz_(0) {}
  list(size_t count, const T& value, const Alloc& allocator = Alloc());
  list(const list& other);
  list(const list& other, const Alloc& allocator);
  explicit list(const Alloc& other_alloc);
  explicit list(size_t count, const Alloc& allocator = Alloc());

  ~list();

  list& operator=(const list& other);

  void push_back(const T& elem);
  void pop_back();
  
  void push_front(const T& elem);
  void pop_front(); 
  iterator erase(const_iterator iter);

  void reverse();

  const_iterator insert(const_iterator pos, const T& value = T());
  template <typename... Args>
  iterator emplace(const_iterator iter, Args&&... args);

  size_t size() const { return sz_; }
  allocator_type get_allocator() const { return alloc_; }
};

template <typename T, typename Alloc>
list<T, Alloc>::list(const Alloc& allocator) 
              : fakeNode_{ &fakeNode_, &fakeNode_ },
                alloc_(allocator),
                sz_(0)
{}

template <typename T, typename Alloc>
list<T, Alloc>::list(size_t count, const T& value, const Alloc& allocator)
    : fakeNode_{ &fakeNode_, &fakeNode_ },
      alloc_(allocator),
      sz_(0) {
  
  size_t count_of_nodes_;
  try {
    for (count_of_nodes_ = 0; count_of_nodes_ < count; ++count_of_nodes_) {
      emplace(begin(), value);
    }
  } catch(...) {
    for (size_t i = 0; i < count_of_nodes_; ++i) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::list(const list& other, const Alloc& allocator)
              : fakeNode_{ &fakeNode_, &fakeNode_ },
                alloc_(allocator),
                sz_(0) {
  size_t count_of_nodes_ = 0;
  try {
    for(const auto& elem : other) {
      emplace(begin(), elem);
      count_of_nodes_++;
    }
  } catch(...) {
    for (size_t i = 0; i < count_of_nodes_; ++i) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::list(const list& other)
              : fakeNode_{ &fakeNode_, &fakeNode_ },
                alloc_(std::allocator_traits<node_allocator>
                          ::select_on_container_copy_construction(other.get_allocator())),
                sz_(0) {
  size_t count_of_nodes_ = 0;
  try {
    for (const auto& elem : other) {
      emplace(end(), elem);
      count_of_nodes_++;
    }
  } catch(...) {
    for (size_t i = 0; i < count_of_nodes_; ++i) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::list(size_t count, const Alloc& allocator)
              : fakeNode_{ &fakeNode_, &fakeNode_ },
                alloc_(allocator), 
                sz_(0) {
  size_t count_of_nodes_;
  try {
    for (count_of_nodes_ = 0; count_of_nodes_ < count; count_of_nodes_++) {
      emplace(begin());
    }
  } catch(...) {
    for (size_t i = 0; i < count_of_nodes_; ++i) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::~list() {
  size_t old_sz_ = sz_;
  for (size_t i = 0; i < old_sz_; ++i) {
    pop_back();
  }
}

template<typename T, typename Alloc>
list<T, Alloc>& list<T, Alloc>::operator=(const list& other) {
  list<T, Alloc> temp_list(other, other.alloc_);
  swap(temp_list);
  return *this;
}

template <typename T, typename Alloc>
void list<T, Alloc>::push_back(const T& elem) {
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  try {
    std::allocator_traits<node_allocator>::construct(alloc_, new_node, &fakeNode_, nullptr, elem);
  } catch(...) {
    std::allocator_traits<node_allocator>::deallocate(alloc_, new_node, 1);
    throw;
  }

  if (sz_ == 0) {
    fakeNode_.next = new_node;
    fakeNode_.prev = new_node;
  } else {
    fakeNode_.prev->next = new_node;
    new_node->prev = fakeNode_.prev;
    fakeNode_.prev = new_node;
  }
  ++sz_;
}

template <typename T, typename Alloc>
void list<T, Alloc>::push_front(const T& elem) {
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  try {
    std::allocator_traits<node_allocator>::construct(alloc_, new_node, nullptr, nullptr, elem);
  } catch(...) {
    std::allocator_traits<node_allocator>::deallocate(alloc_, new_node, 1);
    throw;
  }

  if (sz_ == 0) {
    fakeNode_.next = new_node;
    fakeNode_.prev = new_node;
    new_node->next = &fakeNode_;
  } else {
    fakeNode_.next->prev = new_node;
    new_node->next = fakeNode_.next;
    fakeNode_.next = new_node;
  }
  ++sz_;
}

template <typename T, typename Alloc>
void list<T, Alloc>::pop_back() {

  Node* prev_before_last_elem = static_cast<Node*>(fakeNode_.prev->prev);
  std::allocator_traits<node_allocator>::destroy(alloc_, static_cast<Node*>(fakeNode_.prev));
  std::allocator_traits<node_allocator>::deallocate(alloc_, static_cast<Node*>(fakeNode_.prev), 1);

  if (sz_ == 1) {
    fakeNode_.next = &fakeNode_;
    fakeNode_.prev = &fakeNode_;
  } else {
    fakeNode_.prev = prev_before_last_elem;
    prev_before_last_elem->next = &fakeNode_;
  }

  sz_--;
}

template <typename T, typename Alloc>
void list<T, Alloc>::pop_front() {

  Node* second_element = static_cast<Node*>(fakeNode_.next->next);
  std::allocator_traits<node_allocator>::destroy(alloc_, static_cast<Node*>(fakeNode_.next));
  std::allocator_traits<node_allocator>::deallocate(alloc_, static_cast<Node*>(fakeNode_.next), 1);
  
  if (sz_ == 1) {
    fakeNode_.next = &fakeNode_;
    fakeNode_.prev = &fakeNode_;
  } else {
    fakeNode_.next = second_element;
    second_element->prev = nullptr;
  }

  sz_--;
}

template <typename T, typename Alloc>
auto list<T, Alloc>::erase(typename list<T, Alloc>::const_iterator iter) 
          -> typename list<T, Alloc>::iterator {
  if (iter == this->cend()) {
    return this->end();
  }

  if (iter.ptr->prev != nullptr) {
    iter.ptr->prev->next = iter.ptr->next;
    iter.ptr->next->prev = iter.ptr->prev;
  } else {
   fakeNode_.next = iter.ptr->next;
   iter.ptr->next->prev = nullptr;
  }

  iterator result = iter.ptr->next;
  std::allocator_traits<node_allocator>::destroy(alloc_, static_cast<Node*>(iter.ptr));
  std::allocator_traits<node_allocator>::deallocate(alloc_, static_cast<Node*>(iter.ptr), 1);

  sz_--;
  return result;
}

template <typename T, typename Alloc>
template <typename... Args>
auto list<T, Alloc>::emplace(list<T, Alloc>::const_iterator iter, Args&&... args) 
                   -> list<T, Alloc>::iterator{
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  try {
    std::allocator_traits<node_allocator>::construct(alloc_, new_node, iter.ptr, nullptr, std::forward<Args>(args)...);
  } catch(...) {
    std::allocator_traits<node_allocator>::deallocate(alloc_, new_node, 1);
    throw;
  }

  if (sz_ == 0) {
    fakeNode_.next = new_node;
    fakeNode_.prev = new_node;
  } else if (iter.ptr->prev == nullptr) {
    iter.ptr->prev = new_node;
    fakeNode_.next = new_node;
  } else {
    iter.ptr->prev->next = new_node;
    new_node->prev = iter.ptr->prev;
    iter.ptr->prev = new_node;
  }

  sz_++;

  return { new_node };
}

template <typename T, typename Alloc>
auto list<T, Alloc>::insert(const_iterator pos, const T& value)
                            -> list<T, Alloc>::const_iterator {
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  try {
    std::allocator_traits<node_allocator>::construct(alloc_, new_node, pos.ptr, pos.ptr->prev, value);
  } catch(...) {
    std::allocator_traits<node_allocator>::deallocate(alloc_, new_node, 1);
    throw;
  }

  if (new_node->prev != nullptr) {
    pos.ptr->prev->next = new_node;
  }

  pos.ptr->prev = new_node;

  sz_++;

  return { new_node };
}

template <typename T, typename Alloc>
void list<T, Alloc>::swap(list<T, Alloc>& other) {
  using std::swap;
  swap(fakeNode_, other.fakeNode_);
  swap(sz_, other.sz_);
  if (std::allocator_traits<node_allocator>::propagate_on_container_copy_assignment::value) {
    swap(alloc_, other.alloc_);
  }
}

// BEGIN
template <typename T, typename Alloc>
auto list<T, Alloc>::begin()
          -> typename list<T, Alloc>::iterator {
  return { static_cast<Node*>(fakeNode_.next) };
}

template <typename T, typename Alloc>
auto list<T, Alloc>::begin() const 
          -> typename list<T, Alloc>::const_iterator {
  return { fakeNode_.next };
}

template <typename T, typename Alloc>
auto list<T, Alloc>::cbegin() const
          -> typename list<T, Alloc>::const_iterator {
  return { fakeNode_.next };
}


// END
template <typename T, typename Alloc>
auto list<T, Alloc>::end()
          -> typename list<T, Alloc>::iterator {
  return { &fakeNode_ };
}

template <typename T, typename Alloc>
auto list<T, Alloc>::end() const 
          -> typename list<T, Alloc>::const_iterator {
  return { &fakeNode_ };
}

template <typename T, typename Alloc>
auto list<T, Alloc>::cend() const
          -> typename list<T, Alloc>::const_iterator {
  return { &fakeNode_ };
}

// RBEGIN
template <typename T, typename Alloc>
auto list<T, Alloc>::rbegin() 
          -> typename list<T, Alloc>::reverse_iterator {
  return std::reverse_iterator(this->end());
}

template <typename T, typename Alloc>
auto list<T, Alloc>::rbegin() const
          -> typename list<T, Alloc>::const_reverse_iterator {
  return std::reverse_iterator(this->cend());
}

//REND
template <typename T, typename Alloc>
auto list<T, Alloc>::rend() -> 
          typename list<T, Alloc>::reverse_iterator {
  return std::reverse_iterator(this->begin());
}

template <typename T, typename Alloc>
auto list<T, Alloc>::rend() const -> 
          typename list<T, Alloc>::const_reverse_iterator {
  return std::reverse_iterator(this->cbegin());
}
