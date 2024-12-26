#include <iterator>
#include <memory>
#include <type_traits>

template <typename T, typename Alloc = std::allocator<T>>
class list {
  struct BaseNode {
    BaseNode* next;
    BaseNode* prev;
  };

  struct Node : BaseNode {
    T data;
  };
  

  // шаблонный параметр isConst говорит о том, является ли итератор константным
  template <bool isConst>
  class base_iterator {
  public:
    using reference_type = std::conditional_t<isConst, const T&, T&>;
    using pointer_type = std::conditional_t<isConst, const T*, T*>;
    using value_type = T;
    using diffirence_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
  
  private:
    BaseNode* ptr;
    base_iterator(Node* ptr): ptr(ptr) {}

    friend class list<T>;
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

  void reverse_node(BaseNode* node);

  using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>; 
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

  list(): fakeNode_{ &fakeNode_, &fakeNode_ }, sz_(0) {}
  list(size_t count, const T& value, const Alloc& allocator = Alloc());
  list(const list& other);
  list(const list& other, const Alloc& allocator);
  explicit list(const Alloc& other_alloc);

  ~list();

  list& operator=(const list& other);

  void push_back(const T& elem);
  T pop_back();
  
  void push_front(const T& elem);
  T pop_front(); 
  void erase(iterator iter); // TODO

  void reverse(); // ?

  const_iterator insert(const_iterator pos, const T& value);

  size_t size() const { return sz_; }
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
  for(int i = 0; i < count; ++i) {
    this->push_back(value);
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::list(const list& other, const Alloc& allocator)
              : fakeNode_{ &fakeNode_, &fakeNode_ },
                alloc_(allocator),
                sz_(0) {
  for(const auto& elem : other) {
    this->push_back(elem);
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::list(const list<T, Alloc>& other)
              : fakeNode_{ &fakeNode_, &fakeNode_ },
                alloc_(std::allocator_traits<node_allocator>
                          ::select_on_container_copy_construction(other.alloc_)),
                sz_(0)
{
  for (const auto& elem : other) {
    this->push_back(elem);
  }
}

template <typename T, typename Alloc>
list<T, Alloc>::~list() {
  Node* current_node = static_cast<Node*>(fakeNode_.next);
  Node* next_node = current_node;
  while (current_node->next != &fakeNode_) {
    next_node = static_cast<Node*>(current_node->next);
    std::allocator_traits<node_allocator>::destroy(alloc_, current_node);
    std::allocator_traits<node_allocator>::deallocate(alloc_, current_node, 1);
    current_node = next_node;
  }
  std::allocator_traits<node_allocator>::destroy(alloc_, current_node);
  std::allocator_traits<node_allocator>::deallocate(alloc_, current_node, 1);
}

template <typename T, typename Alloc>
void list<T, Alloc>::push_back(const T& elem) {
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  new_node->data = elem;

  if (sz_ == 0) {
    fakeNode_.next = new_node;
    fakeNode_.prev = new_node;
    new_node->next = &fakeNode_;
  } else {
    fakeNode_.prev->next = new_node;
    new_node->prev = fakeNode_.prev;
    fakeNode_.prev = new_node;
    new_node->next = &fakeNode_;
  }
  ++sz_;
}

template <typename T, typename Alloc>
void list<T, Alloc>::push_front(const T& elem) {
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  new_node->data = elem;

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
T list<T, Alloc>::pop_back() {
  T result = static_cast<Node*>(fakeNode_.prev)->data;

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
  return result;
}

template <typename T, typename Alloc>
T list<T, Alloc>::pop_front() {
  T result = static_cast<Node*>(fakeNode_.next)->data;

  Node* second_element = static_cast<Node*>(fakeNode_.next->next);
  std::allocator_traits<node_allocator>::destroy(alloc_, static_cast<Node*>(fakeNode_.next));
  std::allocator_traits<node_allocator>::deallocate(alloc_, static_cast<Node*>(fakeNode_.next));
  
  if (sz_ == 1) {
    fakeNode_.next = &fakeNode_;
    fakeNode_.prev = &fakeNode_;
  } else {
    fakeNode_.next = second_element;
    second_element->prev = nullptr;
  }

  sz_--;
  return result;
}

// TODO
template <typename T, typename Alloc>
void list<T, Alloc>::erase(typename list<T, Alloc>::iterator iter) {
  
  if (iter.ptr->prev != nullptr) {
    iter.ptr->prev->next = iter.ptr->next;
    iter.ptr->next->prev = iter.ptr->prev;
  } else {
   this->fakeNode_.next = iter.ptr->next;
   iter.ptr->next->prev = nullptr;
  }
  std::allocator_traits<node_allocator>::destroy(alloc_, static_cast<Node*>(iter.ptr));
  std::allocator_traits<node_allocator>::deallocate(alloc_, static_cast<Node*>(iter.ptr), 1);

  sz_--;
}

template <typename T, typename Alloc>
void list<T, Alloc>::reverse() {
  Node* current_node = fakeNode_.next;
  while (current_node != &fakeNode_) {
    reverse_node(current_node);
    current_node = current_node->prev; // prev because node already reversed
  }

  reverse_node(fakeNode_);
}

template <typename T, typename Alloc>
void list<T, Alloc>::reverse_node(BaseNode* node) {
  BaseNode* prev_node = node->prev;
  node->prev = node->next;
  node->next = prev_node;
}

template <typename T, typename Alloc>
auto list<T, Alloc>::insert(const_iterator pos, const T& value)
                            -> list<T, Alloc>::const_iterator {
  Node* new_node = std::allocator_traits<node_allocator>::allocate(alloc_, 1);
  new_node->data = value;

  new_node->next = pos;
  new_node->prev = pos->prev;

  if (new_node->prev != nullptr) {
    pos->prev->next = new_node;
  }
  pos->prev = new_node;
  return { new_node };
}

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
auto list<T, Alloc>::end()
          -> typename list<T, Alloc>::iterator {
  return { static_cast<Node*>(&fakeNode_) };
}

template <typename T, typename Alloc>
auto list<T, Alloc>::end() const
          -> typename list<T, Alloc>::const_iterator {
  return { &fakeNode_ };
}
