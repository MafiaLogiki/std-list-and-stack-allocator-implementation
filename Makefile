main:
	clang++ -std=c++20 -Wall -Wextra -Wpedantic -Weffc++ -Werror -fsanitize=address,undefined,leak -g stack_allocator_test.cpp

