#include <stdio.h>
#include "another_file.h"

int main(int argc, char** argv) {
	Something s;
	s.a = 5;
	s.b = 6;
	print_something(&s);
	printf("hello, world!\n");
}
