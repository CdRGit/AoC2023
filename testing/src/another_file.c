#include <stdio.h>
#include "another_file.h"

void print_something(Something* smth) {
	printf("Something.a: %d, Something.b: %d\n", smth->a, smth->b);
}
