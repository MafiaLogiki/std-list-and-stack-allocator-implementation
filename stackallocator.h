#include <memory>

template <size_t size>
class StackStorage {
private:  
  char storage_[size];
  size_t reserved_ = 0;

public:
  StackStorage& operator=(const StackStorage&) = delete;
  StackStorage() = default;
  StackStorage(const StackStorage& other) = delete;
  char* get_array() { return storage_ + reserved_; }
  void reserve(size_t n) { reserved_ += n; }
};

template <typename T, size_t size>
class StackAllocator {

  void* current_storage_pos_;
  void** ptr_to_storage_;
  size_t sz_;

public:

  using value_type = T;
  using pointer_type = T*;
  using size_type = size_t;

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, size>;
  };

  StackAllocator() = delete;

  template <size_t N>
  StackAllocator(StackStorage<N>& storage);
  
  template <typename U>
  StackAllocator(const StackAllocator<U, size>&);

  T* allocate(size_t n);
  void deallocate(T* pointer, size_t n);
  bool operator==(const StackAllocator<T, size>& alloc) { return ptr_to_storage_ == alloc.get_ptr_to_storage(); }

  void* get_storage() const { return current_storage_pos_; }
  void** get_ptr_to_storage() const { return ptr_to_storage_; }
  size_t get_size() const { return sz_; }
};

template <typename T, size_t size>
template <size_t U>
StackAllocator<T, size>::StackAllocator(StackStorage<U>& storage)
                       : current_storage_pos_(storage.get_array())
                       , ptr_to_storage_(&current_storage_pos_)
                       , sz_(size) {
  storage.reserve(size);
}

template <typename T, size_t size>
template <typename U>
StackAllocator<T, size>::StackAllocator(const StackAllocator<U, size>& alloc)
                       : current_storage_pos_(alloc.get_storage())
                       , ptr_to_storage_(alloc.get_ptr_to_storage())
                       , sz_(alloc.get_size())
{}

template <typename T, size_t size>
T* StackAllocator<T, size>::allocate(size_t n) {
  if (std::align(alignof(T), n * sizeof(T), *ptr_to_storage_, sz_) != nullptr) {
    T* result = reinterpret_cast<T*>(*ptr_to_storage_);
    *ptr_to_storage_ = reinterpret_cast<char*>(*ptr_to_storage_) + sizeof(T) * n;
    sz_ -= sizeof(T) * n;
    return result;
  }

  throw std::bad_alloc();
}

template <typename T, size_t size>
void StackAllocator<T, size>::deallocate(T* pointer, size_t n) {
  std::ignore = pointer;
  std::ignore = n;
}
