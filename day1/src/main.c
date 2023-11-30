#include <stdio.h>
#include <stdbool.h>

const bool TRY_REAL_INPUT = false;

const char test_input[] = "day1/test.txt";
const char real_input[] = "day1/real.txt";

void part_1(const char* path);
void part_2(const char* path);

int main(int argc, const char** argv) {
	part_1(test_input);
	if (TRY_REAL_INPUT)
		part_1(real_input);

	part_2(test_input);
	if (TRY_REAL_INPUT)
		part_2(real_input);
}

void part_1(const char* path) {
	printf("part 1: %s\n", path);
}
void part_2(const char* path) {
	printf("part 2: %s\n", path);
}
