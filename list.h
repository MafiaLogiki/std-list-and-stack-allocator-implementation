#include <initializer_list>
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
    using difference_type = std::ptrdiff_t;
  
  private:
    Node* ptr;
    base_iterator(BaseNode* ptr): ptr(ptr) {}

    friend class list<T>;
  public:
    base_iterator() = default;
    base_iterator(const base_iterator&) = default;
    base_iterator operator=(const base_iterator&) = default;

    reference_type operator*() { return ptr->data; }
    pointer_type operator->() { return &(ptr->data); }

    base_iterator& operator++() {
      ptr = ptr->next;
      return ptr;
    }

    base_iterator operator++(int) {
      base_iterator copy = *this;
      ptr = ptr->next;
      return copy;
    }

    base_iterator& operator--() {
      ptr = ptr->prev;
      return ptr;
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


  BaseNode fakeNode_; // fakeNode_.next -> start of the list, fakeNode_.prev -> end of the list
  typename std::allocator_traits<Alloc>::template rebind_alloc<Node> alloc_;
  size_t sz_;
public:
  
  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  list(): fakeNode_{ &fakeNode_, &fakeNode_ }, sz_(0) {}
  list(std::initializer_list<T> init_list);

  void push_back(const T& elem);
  T pop_back();
  
  void push_front(const T& elem);
  T pop_front(); 

  void reverse();

  size_t size() const { return sz_; }
};




template<typename T, typename Alloc>
list<T, Alloc>::list(std::initializer_list<T> init_list) {
  for(auto& elem : init_list) {
    this->push_back(elem);
  }
}

template <typename T, typename Alloc>
void list<T, Alloc>::push_back(const T& elem) {
  Node* new_node = std::allocator_traits<decltype(alloc_)>::allocate(alloc_, 1);
  new_node->data = elem;

  if (sz_ == 0) {
    fakeNode_.next = new_node;
    fakeNode_.prev = new_node;
    new_node->next = &fakeNode_;
  } else {
    fakeNode_.prev->next = new_node;
    new_node->prev = fakeNode_.prev;
    fakeNode_.prev = new_node;
  }
  ++sz_;
}

template <typename T, typename Alloc>
void list<T, Alloc>::push_front(const T& elem) {
  Node* new_node = std::allocator_traits<Alloc>::allocate(alloc_, 1);
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
  delete fakeNode_.prev;

  fakeNode_.prev = prev_before_last_elem;
  prev_before_last_elem->next = &fakeNode_;
  fakeNode_.prev = prev_before_last_elem;

  sz_--;
  return result;
}


// TODO
/*
template <typename T, typename Alloc>
void list<T, Alloc>::reverse() {
  Node* ex_first_element = fakeNode_.next;

  fakeNode_.next->prev = &fakeNode_;
  fakeNode_.prev->next

  fakeNode_.next = fakeNode_.prev;
  fakeNode_.prev = ex_first_element;
}
*/
template <typename T, typename Alloc>
typename list<T, Alloc>::iterator list<T, Alloc>::begin() {
  return {fakeNode_.next};
}

template <typename T, typename Alloc>
typename list<T, Alloc>::const_iterator list<T, Alloc>::begin() const {
  return {fakeNode_.next};
}

template <typename T, typename Alloc>
typename list<T, Alloc>::iterator list<T, Alloc>::end() {
  return {&fakeNode_};
}

template <typename T, typename Alloc>
typename list<T, Alloc>::const_iterator list<T, Alloc>::end() const {
  return {&fakeNode_};
}
