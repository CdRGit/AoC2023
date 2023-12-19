#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <ctype.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdint.h>
uint8_t* read_data(const char* const fileName) {
	uint8_t* data;

	printf("reading: %s\n", fileName);

	int fd = open(fileName, O_RDONLY);

	struct stat sb;

	fstat(fd, &sb);
	printf("Size: %lu\n", (uint64_t)sb.st_size);

	data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "Could not memory map file: %s\n", fileName);
		exit(1);
	}

	return data;
}

const bool TRY_REAL_INPUT = true;

const char test_input[] = "test_input/day19.txt";
const char real_input[] = "real_input/day19.txt";

void part_1(const char* path, const uint8_t* data);
void part_2(const char* path, const uint8_t* data);

int main(int argc, const char** argv) {
	uint8_t* test_data = read_data(test_input);

	part_1(test_input, test_data);
	part_2(test_input, test_data);

	if (TRY_REAL_INPUT) {
		printf("---\n");
		uint8_t* real_data = read_data(real_input);
		part_1(real_input, real_data);
		part_2(real_input, real_data);
	}
}

typedef struct {
	uint8_t category;
	enum {
		LESS_THAN,
		MORE_THAN,
		ALWAYS,
	} condition;
	uint64_t value;
	int next;
} rule;

typedef struct {
	rule* rules;
	size_t len;
	size_t cap;
} workflow;

void add_rule(workflow* flow, rule r) {
	if (flow->len + 1 > flow->cap) {
		flow->cap = flow->cap ? flow->cap * 2 : 1;
		flow->rules = realloc(flow->rules, flow->cap * sizeof(rule));
	}
	flow->rules[flow->len++] = r;
}

int get_flow_index(const uint8_t* name) {
	if (*name == 'A') return -1; // accept = -1
	if (*name == 'R') return -2; // reject = -2
	int idx = 0;
	for (int i = 0; i < 3; i++) {
		idx *= 27;
		if (isalpha(*name)) {
			idx += *name++ - 'a' + 1;
		}
	}
	return idx;
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	// 26 letters + 'no letter' option
	workflow flows[27 * 27 * 27] = {0};

	while (*data != '\n') {
		int idx = get_flow_index(data);
		while (*data++ != '{');
		while (true) {
			const uint8_t* name = data;
			size_t len = 0;
			while (*data != '<' && *data != '>' && *data != '}') {
				data++;
				len++;
			}
			uint8_t type = *data++;
			if (type == '}') {
				assert(len <= 3 && "name too long");
				add_rule(&flows[idx], (rule){.category = 0, .condition = ALWAYS, .value = 0, .next = get_flow_index(name)});
				data++;
				break;
			}
			// < or >
			assert(len == 1 && "category too long");
			uint8_t category = *name;
			uint64_t value = 0;
			while (isdigit(*data)) {
				value *= 10;
				value += *data++ - '0';
			}
			data++; // skip the ':'
			len = 0;
			name = data;
			while (*data++ != ',') len++;
			assert(len <= 3 && "name too long");
			switch (category) {
				case 'x': category = 0; break;
				case 'm': category = 1; break;
				case 'a': category = 2; break;
				case 's': category = 3; break;
			}
			add_rule(&flows[idx], (rule){.category = category, .condition = type == '>' ? MORE_THAN : LESS_THAN, .value = value, .next = get_flow_index(name)});
		}
	}

	data++;
	uint64_t sum = 0;
	while (*data) {
		int idx = get_flow_index((const uint8_t*)"in");
		uint64_t categories[4];
		uint64_t part_sum = 0;
		for (int i = 0; i < 4; i++) {
			categories[i] = 0;
			data += 3; // skip '{x=' or ',?='
			while (isdigit(*data)) {
				categories[i] *= 10;
				categories[i] += *data++ - '0';
			}
			part_sum += categories[i];
		}
		data+=2; // skip '}\n'
		// actually perform the workflows
		while (idx >= 0) {
			for (int i = 0; i < flows[idx].len; i++) {
				rule r = flows[idx].rules[i];
				uint64_t value = categories[r.category];
				if (r.condition == LESS_THAN) {
					if (value < r.value) {
						idx = r.next;
						break;
					}
				} else if (r.condition == MORE_THAN) {
					if (value > r.value) {
						idx = r.next;
						break;
					}
				} else {
					idx = r.next;
					break;
				}
			}
		}
		// accepted?
		if (idx == -1) {
			sum += part_sum;
		}
	}
	printf("%lu\n", sum);

	for (int i = 0; i < 27 * 27 * 27; i++) {
		if (flows[i].rules) free(flows[i].rules);
	}
}

typedef struct {
	int idx;
	uint64_t range_min[4];
	uint64_t range_max[4];
} range_set;

typedef struct {
	range_set* data;
	size_t len;
	size_t cap;
} stack;

void push(stack* s, range_set r) {
	if (s->len + 1 > s->cap) {
		s->cap = s->cap ? s->cap * 2 : 1;
		s->data = realloc(s->data, s->cap * sizeof(range_set));
	}
	s->data[s->len++] = r;
}

bool has_data(stack s) {
	return s.len != 0;
}

range_set pop(stack* s) {
	return s->data[--s->len];
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	// 26 letters + 'no letter' option
	workflow flows[27 * 27 * 27] = {0};

	while (*data != '\n') {
		int idx = get_flow_index(data);
		while (*data++ != '{');
		while (true) {
			const uint8_t* name = data;
			size_t len = 0;
			while (*data != '<' && *data != '>' && *data != '}') {
				data++;
				len++;
			}
			uint8_t type = *data++;
			if (type == '}') {
				assert(len <= 3 && "name too long");
				add_rule(&flows[idx], (rule){.category = 0, .condition = ALWAYS, .value = 0, .next = get_flow_index(name)});
				data++;
				break;
			}
			// < or >
			assert(len == 1 && "category too long");
			uint8_t category = *name;
			uint64_t value = 0;
			while (isdigit(*data)) {
				value *= 10;
				value += *data++ - '0';
			}
			data++; // skip the ':'
			len = 0;
			name = data;
			while (*data++ != ',') len++;
			assert(len <= 3 && "name too long");
			switch (category) {
				case 'x': category = 0; break;
				case 'm': category = 1; break;
				case 'a': category = 2; break;
				case 's': category = 3; break;
			}
			add_rule(&flows[idx], (rule){.category = category, .condition = type == '>' ? MORE_THAN : LESS_THAN, .value = value, .next = get_flow_index(name)});
		}
	}

	data++;
	uint64_t count = 0;
	stack s = {0};
	range_set start = {0};
	start.idx = get_flow_index((const uint8_t*)"in");
	for (int i = 0; i < 4; i++) {
		start.range_min[i] = 1;
		start.range_max[i] = 4000;
	}
	push(&s, start);
	while (has_data(s)) {
		range_set curr = pop(&s);
		if (curr.idx < 0) {
			uint64_t size = 1;
			for (int i = 0; i < 4; i++) {
				size *= (curr.range_max[i] - curr.range_min[i] + 1);
			}
			if (curr.idx == -1) {
				count += size;
			}
			continue;
		}
		for (int i = 0; i < flows[curr.idx].len; i++) {
			rule r = flows[curr.idx].rules[i];
			uint64_t min = curr.range_min[r.category];
			uint64_t max = curr.range_max[r.category];
			if (r.condition == LESS_THAN) {
				if (max < r.value) {
					// trivial case, we move the entire thing
					curr.idx = r.next;
					push(&s, curr);
					break;
				} else if (min >= r.value) {
					// another trivial case, we ignore the rule
					continue;
				}
				// make a copy
				range_set new = curr;
				// bisect the range
				new.range_max[r.category] = r.value - 1;
				curr.range_min[r.category] = r.value;
				// move the new one to the right rule
				new.idx = r.next;
				push(&s, new);
			} else if (r.condition == MORE_THAN) {
				if (min > r.value) {
					// trivial, move the entire thing
					curr.idx = r.next;
					push(&s, curr);
					break;
				} else if (max <= r.value) {
					// another trivial case, we ignore the rule
					continue;
				}
				// make a copy
				range_set new = curr;
				// bisect the range
				new.range_min[r.category] = r.value + 1;
				curr.range_max[r.category] = r.value;
				// move the new one to the right rule
				new.idx = r.next;
				push(&s, new);
			} else {
				curr.idx = r.next;
				push(&s, curr);
				break;
			}
		}
	}
	printf("%lu\n", count);

	for (int i = 0; i < 27 * 27 * 27; i++) {
		if (flows[i].rules) free(flows[i].rules);
	}
}
