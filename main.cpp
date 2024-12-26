#include "list.h"
#include <cassert>
#include <iostream>
#include <string>
#include <iterator>

void test1() {

  // static_assert(std::bidirectional_iterator<list<int>::iterator>);


  list<int> l1;
  for (int i = 0; i < 1'000; ++i) {
    l1.push_back(i);
  }

  assert(l1.size() == 1000);
  std::cerr << "push_back and size test passed" << std::endl;


  for (int i = 999; i >= 0; --i) {
    int k = l1.pop_back();
    assert(k == i);
  }
  std::cerr << "pop+back test passed" << std::endl;


  l1.push_back(12);
  assert(l1.pop_back() == 12);
  assert(l1.size() == 0);

  l1.push_back(10);
  l1.push_front(11);
  l1.push_back(12);
  l1.push_front(13);
  
  std::string s = "";
  for (const auto& elem : l1) {
    s += std::to_string(elem);
  }
  assert(s == "13111012");

  l1.erase(l1.begin());
}


int main() {
  test1();
  std::cout << "test1 passed";
}
