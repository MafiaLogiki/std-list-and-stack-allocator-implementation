#include "list.h"
#include <cassert>
#include <iostream>


void test1() {
  list<int> l1;
  for (int i = 0; i < 1'000; ++i) {
    l1.push_back(i);
  }

  assert(l1.size() == 1000);

  for (int i = 999; i > 0; --i) {
    int k = l1.pop_back();
    assert(k == i);
  }

  l1.push_back(12);
  assert(l1.pop_back() == 12);


}

int main() {
  test1();
  std::cout << "test1 passed";
}
