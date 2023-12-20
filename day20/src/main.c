#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

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

const char test_input[] = "test_input/day20.txt";
const char real_input[] = "real_input/day20.txt";

void part_1(const char* path, const uint8_t* data);
void part_2(const char* path, const uint8_t* data);

int main(int argc, const char** argv) {
	uint8_t* test_data = read_data(test_input);

	part_1(test_input, test_data);
	// not useful at all
	//part_2(test_input, test_data);
	printf("part 2 does not work on test data today\nN/A\n");

	if (TRY_REAL_INPUT) {
		printf("---\n");
		uint8_t* real_data = read_data(real_input);
		part_1(real_input, real_data);
		part_2(real_input, real_data);
	}
}

typedef struct {
	int* outgoing;
	size_t len;
	size_t cap;

	enum {
		UNTYPED,
		FLIP_FLOP,
		CONJUNCTION,
	} type;

	// for flip-flop
	bool internal_state;
	// for conjunction
	bool* input_states;
	int* inputs;
	size_t input_len;
	size_t input_cap;
} module;

void attach(module* m_src, module* m_dst, int src_idx, int dst_idx) {
	if (m_src->len + 1 > m_src->cap) {
		m_src->cap = m_src->cap ? m_src->cap * 2 : 1;
		m_src->outgoing = realloc(m_src->outgoing, m_src->cap * sizeof(int));
	}
	m_src->outgoing[m_src->len++] = dst_idx;

	if (m_dst->input_len + 1 > m_dst->input_cap) {
		m_dst->input_cap = m_dst->input_cap ? m_dst->input_cap * 2 : 1;
		m_dst->input_states = realloc(m_dst->input_states, m_dst->input_cap * sizeof(bool));
		m_dst->inputs = realloc(m_dst->inputs, m_dst->input_cap * sizeof(int));
	}
	m_dst->input_states[m_dst->input_len] = 0;
	m_dst->inputs[m_dst->input_len++] = src_idx;
}

int parse_index(const uint8_t* name) {
	return (name[0] - 'a') * 26 + (name[1] - 'a');
}

typedef struct {
	int target;
	int source;
	bool high;
} pulse;

typedef struct {
	pulse* data;
	size_t len;
	size_t cap;
	size_t start;
} queue;

void enqueue(queue* q, pulse p) {
	if (q->len + 1 > q->cap) {
		q->cap = q->cap ? q->cap * 2 : 1;
		q->data = realloc(q->data, q->cap * sizeof(pulse));
	}
	q->data[q->len++] = p;
}

bool has_data(queue q) {
	return q.start != q.len;
}

pulse dequeue(queue* q) {
	return q->data[q->start++];
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	module modules[26 * 26] = {0};
	int broadcast_out[26 * 26] = {0};
	size_t broadcast_len = 0;
	while (*data) {
		enum {
			P_BROADCAST,
			P_FLIP_FLOP,
			P_CONJUNCT,
		} current_module = 0;
		int current_index = 0;
		switch (*data++) {
			case 'b':
				// 'roadcaster -> '
				data += 14;
				current_module = P_BROADCAST;
				break;
			case '%':
				current_index = parse_index(data);
				// '?? -> '
				data += 6;
				current_module = P_FLIP_FLOP;
				break;
			case '&':
				current_index = parse_index(data);
				// '?? -> '
				data += 6;
				current_module = P_CONJUNCT;
				break;
			default:
				printf("'%c'?\n", data[-1]);
				assert(false && "TODO");
		}
		while (true) {
			int idx = parse_index(data);
			data += 2;
			if (current_module == P_BROADCAST) {
				broadcast_out[broadcast_len++] = idx;
				if (modules[idx].input_len + 1 > modules[idx].input_cap) {
					modules[idx].input_cap = modules[idx].input_cap ? modules[idx].input_cap * 2 : 1;
					modules[idx].input_states = realloc(modules[idx].input_states, modules[idx].input_cap * sizeof(bool));
					modules[idx].inputs = realloc(modules[idx].inputs, modules[idx].input_cap * sizeof(int));
				}
				modules[idx].input_states[modules[idx].input_len] = 0;
				modules[idx].inputs[modules[idx].input_len++] = -1;
			} else {
				attach(&modules[current_index], &modules[idx], current_index, idx);
			}
			if (*data == ',') data += 2;
			else break;
		}
		if (current_module != P_BROADCAST) {
			modules[current_index].type = current_module == P_FLIP_FLOP ? FLIP_FLOP : CONJUNCTION;
		}
		if (*data) data++;
	}

	size_t low_pulses = 0;
	size_t high_pulses = 0;
	queue pulse_queue = {0};
	for (size_t i = 0; i < 1000; i++) {
		pulse_queue.len = 0;
		pulse_queue.start = 0;
		// push button
		low_pulses++;
		// broadcaster
		for (int j = 0; j < broadcast_len; j++) {
			pulse p = {0};
			p.source = -1;
			p.target = broadcast_out[j];
			p.high = false;
			enqueue(&pulse_queue, p);
		}
		// propagate
		while (has_data(pulse_queue)) {
			pulse curr = dequeue(&pulse_queue);
			if (curr.high) high_pulses++;
			else            low_pulses++;

			module* m = &modules[curr.target];
			switch (m->type) {
				case UNTYPED:
					// NOP
					break;
				case FLIP_FLOP:
					if (!curr.high) {
						// toggle
						m->internal_state ^= true;
						for (int i = 0; i < m->len; i++) {
							pulse p = {0};
							p.source = curr.target;
							p.target = m->outgoing[i];
							p.high = m->internal_state;
							enqueue(&pulse_queue, p);
						}
					}
					break;
				case CONJUNCTION: {
						bool val = true;
						for (int i = 0; i < m->input_len; i++) {
							if (m->inputs[i] == curr.source) {
								m->input_states[i] = curr.high;
							}
							val &= m->input_states[i];
						}
						for (int i = 0; i < m->len; i++) {
							pulse p = {0};
							p.source = curr.target;
							p.target = m->outgoing[i];
							p.high = !val;
							enqueue(&pulse_queue, p);
						}
					}
					break;
			}
		}
	}
	free(pulse_queue.data);

	printf("%zu\n", low_pulses * high_pulses);

	for (int i = 0; i < 26 * 26; i++) {
		if (modules[i].cap > 0) {
			free(modules[i].outgoing);
		}
		if (modules[i].input_cap > 0) {
			free(modules[i].inputs);
			free(modules[i].input_states);
		}
	}
}

size_t gcd(size_t a, size_t b) {
	if (b == 0)
		return a;
	return gcd(b, a % b);
}

size_t lcm(size_t a, size_t b) {
	return (a / gcd(a, b)) * b;
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	module modules[26 * 26] = {0};
	int broadcast_out[26 * 26] = {0};
	size_t broadcast_len = 0;
	while (*data) {
		enum {
			P_BROADCAST,
			P_FLIP_FLOP,
			P_CONJUNCT,
		} current_module = 0;
		int current_index = 0;
		switch (*data++) {
			case 'b':
				// 'roadcaster -> '
				data += 14;
				current_module = P_BROADCAST;
				break;
			case '%':
				current_index = parse_index(data);
				// '?? -> '
				data += 6;
				current_module = P_FLIP_FLOP;
				break;
			case '&':
				current_index = parse_index(data);
				// '?? -> '
				data += 6;
				current_module = P_CONJUNCT;
				break;
			default:
				printf("'%c'?\n", data[-1]);
				assert(false && "TODO");
		}
		while (true) {
			int idx = parse_index(data);
			data += 2;
			if (current_module == P_BROADCAST) {
				broadcast_out[broadcast_len++] = idx;
				if (modules[idx].input_len + 1 > modules[idx].input_cap) {
					modules[idx].input_cap = modules[idx].input_cap ? modules[idx].input_cap * 2 : 1;
					modules[idx].input_states = realloc(modules[idx].input_states, modules[idx].input_cap * sizeof(bool));
					modules[idx].inputs = realloc(modules[idx].inputs, modules[idx].input_cap * sizeof(int));
				}
				modules[idx].input_states[modules[idx].input_len] = 0;
				modules[idx].inputs[modules[idx].input_len++] = -1;
			} else {
				attach(&modules[current_index], &modules[idx], current_index, idx);
			}
			if (*data == ',') data += 2;
			else break;
		}
		if (current_module != P_BROADCAST) {
			modules[current_index].type = current_module == P_FLIP_FLOP ? FLIP_FLOP : CONJUNCTION;
		}
		if (*data) data++;
	}
	// this assumes there are 4 counters, because if not I will cry
	assert(broadcast_len == 4);
	// it also assumes each counter is 12 bits long, again, see previous comment
	// this is what my graphviz showed
	int counters[12 * 4];
	// counters to conjunction
	uint16_t counter_to_conjunction[4] = {0};
	// when a counter-conjunction triggers it resets the counter to 0
	// meaning each counter is the length of this value
	// I can then LCM them all at the end
	for (int ci = 0; ci < 4; ci++) {
		int idx = broadcast_out[ci];
		for (int cd = 0; cd < 12; cd++) {
			int i = ci * 12 + cd;
			counters[i] = idx;
			int n_idx = idx;
			for (int j = 0; j < modules[idx].len; j++) {
				if (modules[modules[idx].outgoing[j]].type == FLIP_FLOP) {
					n_idx = modules[idx].outgoing[j];
				} else {
					counter_to_conjunction[ci] |= 1 << cd;
				}
			}
			idx = n_idx;
		}
	}

	size_t multiple = 1;
	for (int i = 0; i < 4; i++) {
		multiple = lcm(multiple, counter_to_conjunction[i]);
	}

	printf("%zu\n", multiple);

	for (int i = 0; i < 26 * 26; i++) {
		if (modules[i].cap > 0) {
			free(modules[i].outgoing);
		}
		if (modules[i].input_cap > 0) {
			free(modules[i].inputs);
			free(modules[i].input_states);
		}
	}
}
