// #include "list.h"
#include <cassert>
#include <iostream>
#include <list>
#include <string>
#include <iterator>
#include <algorithm>
#include <memory>
#include "stackallocator.h"

template <typename T, typename Alloc>
using list = std::list<T, Alloc>;

const int STORAGE_SIZE = 200'000'000;
StackStorage<STORAGE_SIZE> STORAGE;

template <typename Alloc = std::allocator<int>>
void BasicListTest(Alloc alloc = Alloc()) {
    list<int, Alloc> lst(alloc);

    assert(lst.size() == 0);

    lst.push_back(3);
    lst.push_back(4);
    lst.push_front(2);
    lst.push_back(5);
    lst.push_front(1);

    std::reverse(lst.begin(), lst.end());
    // now lst is 5 4 3 2 1
    
    assert(lst.size() == 5);

    std::string s;
    for (int x: lst) {
        s += std::to_string(x);
    }
    assert(s == "54321");
    std::cerr << " check 1.1 ok, list contains 5 4 3 2 1" << std::endl;

    auto cit = lst.cbegin();
    std::advance(cit, 3);
    
    lst.insert(cit, 6);
    lst.insert(cit, 7);

    std::advance(cit, -3);
    lst.insert(cit, 8);
    lst.insert(cit, 9);
    // now lst is 5 4 8 9 3 6 7 2 1
    
    assert(lst.size() == 9);

    s.clear();
    for (int x: lst) {
        s += std::to_string(x);
    }
    assert(s == "548936721");
    std::cerr << " check 1.2 ok, list contains 5 4 8 9 3 6 7 2 1" << std::endl;

    lst.erase(lst.cbegin());
    lst.erase(cit);

    lst.pop_front();
    lst.pop_back();

    const auto copy = lst;
    assert(lst.size() == 5);
    assert(copy.size() == 5);
    // now both lists are 8 9 6 7 2

    s.clear();
    for (int x: lst) {
        s += std::to_string(x);
    }
    assert(s == "89672");
    std::cerr << " check 1.3 ok, list contains 8 9 6 7 2" << std::endl;
    
    auto rit = lst.rbegin();
    ++rit;
    lst.erase(rit.base());
    assert(lst.size() == 4);

    rit = lst.rbegin();
    *rit = 3;

    // now lst: 8 9 6 3, copy: 8 9 6 7 2
    s.clear();
    for (int x: lst) {
        s += std::to_string(x);
    }
    assert(s == "8963");
    
    assert(copy.size() == 5);

    s.clear();
    for (int x: copy) {
        s += std::to_string(x);
    }
    assert(s == "89672");

    std::cerr << " check 1.4 ok, list contains 8 9 6 3, another list is still 8 9 6 7 2" << std::endl;

    typename list<int, Alloc>::const_reverse_iterator crit = rit;
    crit = copy.rbegin();
    assert(*crit == 2);

    cit = crit.base();
    std::advance(cit, -2);
    assert(*cit == 7);

}

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
struct WhimsicalAllocator : public std::allocator<T> {
    std::shared_ptr<int> number;

    auto select_on_container_copy_construction() const {
        return PropagateOnConstruct 
            ? WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>() 
            : *this;
    }

    struct propagate_on_container_copy_assignment
        : std::conditional_t<PropagateOnAssign, std::true_type, std::false_type> 
    {};

    template <typename U>
    struct rebind {
        using other = WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>;
    };

    WhimsicalAllocator(): number(std::make_shared<int>(counter)) {
        ++counter;
    }

    template <typename U>
    WhimsicalAllocator(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another)
            : number(another.number)
    {}

    template <typename U>
    auto& operator=(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) {
        number = another.number;
        return *this;
    }
    
    template <typename U>
    bool operator==(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) const {
        return std::is_same_v<decltype(*this), decltype(another)> 
            && *number == *another.number;
    }

    template <typename U>
    bool operator!=(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) const {
        return !(*this == another);
    }

    static size_t counter;
};

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
size_t WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>::counter = 0;

void TestWhimsicalAllocator() {
    {
        list<int, WhimsicalAllocator<int, true, true>> lst;

        lst.push_back(1);
        lst.push_back(2);

        auto copy = lst;
        assert(copy.get_allocator() != lst.get_allocator());

        lst = copy;
        assert(copy.get_allocator() == lst.get_allocator());
    }
    {
        list<int, WhimsicalAllocator<int, false, false>> lst;

        lst.push_back(1);
        lst.push_back(2);

        auto copy = lst;
        assert(copy.get_allocator() == lst.get_allocator());

        lst = copy;
        assert(copy.get_allocator() == lst.get_allocator());
    }
    {
        list<int, WhimsicalAllocator<int, true, false>> lst;

        lst.push_back(1);
        lst.push_back(2);

        auto copy = lst;
        assert(copy.get_allocator() != lst.get_allocator());

        lst = copy;
        assert(copy.get_allocator() != lst.get_allocator());
    }
}


struct Accountant {
    // Some field of strange size
    char arr[40];

    static size_t ctor_calls;
    static size_t dtor_calls;

    static void reset() {
        ctor_calls = 0;
        dtor_calls = 0;
    }

    Accountant() {
        ++ctor_calls;
    }
    Accountant(const Accountant&) {
        ++ctor_calls;
    }

    Accountant& operator=(const Accountant&) {
        // Actually, when it comes to assign one list to another,
        // list can use element-wise assignment instead of destroying nodes and creating new ones
        ++ctor_calls;
        ++dtor_calls;
        return *this;
    }

    Accountant(Accountant&&) = delete;
    Accountant& operator=(Accountant&&) = delete;

    ~Accountant() {
        ++dtor_calls;
    }
};

size_t Accountant::ctor_calls = 0;
size_t Accountant::dtor_calls = 0;

template<typename Alloc = std::allocator<Accountant>>
void TestAccountant(Alloc alloc = Alloc()) {
    Accountant::reset();
    
    {
        list<Accountant, Alloc> lst(5, alloc);
        assert(lst.size() == 5);
        assert(Accountant::ctor_calls == 5);

        list<Accountant, Alloc> another = lst;
        assert(another.size() == 5);
        assert(Accountant::ctor_calls == 10);
        assert(Accountant::dtor_calls == 0);

        another.pop_back();
        another.pop_front();
        assert(Accountant::dtor_calls == 2);

        lst = another; // dtor_calls += 5, ctor_calls += 3
        assert(another.size() == 3);
        assert(lst.size() == 3);

        assert(Accountant::ctor_calls == 13);
        assert(Accountant::dtor_calls == 7);
    
    } // dtor_calls += 6
    
    assert(Accountant::ctor_calls == 13);
    assert(Accountant::dtor_calls == 13);
}

struct VerySpecialType {
    int x = 0;
    explicit VerySpecialType(int x): x(x) {}
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;
    NotDefaultConstructible(VerySpecialType x): x(x) {}
    VerySpecialType x;
};

template <typename Alloc = std::allocator<NotDefaultConstructible>>
void TestNotDefaultConstructible(Alloc alloc = Alloc()) {
    list<NotDefaultConstructible, Alloc> lst(alloc);
    assert(lst.size() == 0);
    lst.push_back(VerySpecialType(0));
    assert(lst.size() == 1);
    lst.pop_front();
    assert(lst.size() == 0);
}


void TestAlignment() {

    StackStorage<200'000> storage;

    StackAllocator<char, 200'000> charalloc(storage);

    StackAllocator<int, 200'000> intalloc(charalloc);

    auto* pchar = charalloc.allocate(3);
    
    auto* pint = intalloc.allocate(1);

    assert((void*)pchar != (void*)pint);
    
    assert(reinterpret_cast<uintptr_t>(pint) % sizeof(int) == 0);

    charalloc.deallocate(pchar, 3);

    pchar = charalloc.allocate(555);

    intalloc.deallocate(pint, 1);

    StackAllocator<long double, 200'000> ldalloc(charalloc);

    auto* pld = ldalloc.allocate(25);

    assert(reinterpret_cast<uintptr_t>(pld) % sizeof(long double) == 0);

    charalloc.deallocate(pchar, 555);
    ldalloc.deallocate(pld, 25);
}

int main() {

  BasicListTest();
  std::cout << "Test 1 (basic list test) with std::allocator passed" << std::endl;

  {
    StackStorage<200'000> storage;
    StackAllocator<int, 200'000> alloc(storage);
    BasicListTest<StackAllocator<int, 200'000>>(alloc);
  }

  TestWhimsicalAllocator();
  std::cout << "Allocator Aware container test passed" << std::endl;

  TestAccountant();
  std::cout << "Accountant test passed" << std::endl;

  TestNotDefaultConstructible();
  std::cout << "NotDefaultConstructible test passed" << std::endl;

  TestAlignment();
  std::cout << "Alingment test with stack_allocator passed" << std::endl;
}
