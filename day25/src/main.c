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

const char test_input[] = "test_input/day25.txt";
const char real_input[] = "real_input/day25.txt";

void part_1(const char* path, const uint8_t* data);

int main(int argc, const char** argv) {
	uint8_t* test_data = read_data(test_input);

	part_1(test_input, test_data);

	if (TRY_REAL_INPUT) {
		printf("---\n");
		uint8_t* real_data = read_data(real_input);
		part_1(real_input, real_data);
	}
}

typedef struct {
	int n1;
	int n2;
} edge;

typedef struct {
	edge* data;
	size_t len;
	size_t cap;
} edge_vec;

void append(edge_vec* v, edge e) {
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(edge));
	}
	v->data[v->len++] = e;
}

int idx(const uint8_t* name) {
	return name[0] - 'a' + (name[1] - 'a') * 26 + (name[2] - 'a') * 26 * 26;
}

void name(const int idx, uint8_t (*dst)[4]) {
	(*dst)[0] = idx % 26 + 'a';
	(*dst)[1] = (idx / 26) % 26 + 'a';
	(*dst)[2] = (idx / 26 / 26) % 26 + 'a';
	(*dst)[3] = 0;
}

edge extract(edge_vec* vec, int index) {
	edge e = vec->data[index];
	memmove(vec->data + index, vec->data + index + 1, sizeof(edge) * (vec->len - index - 1));
	vec->len--;
	return e;
}

size_t karger(edge_vec v_in, int target) {
	edge_vec edges = {0};
	for (;;) {
		edges.len = 0;
		// copy edges
		for (int i = 0; i < v_in.len; i++) {
			append(&edges, v_in.data[i]);
		}
		// nodes
		int node_cnt = 0;
		int nodesize[26*26*26] = {0};
		// set up all the nodes
		for (int i = 0; i < edges.len; i++) {
			int a = edges.data[i].n1;
			int b = edges.data[i].n2;
			nodesize[a] = 1;
			nodesize[b] = 1;
		}
		// count
		for (int i = 0; i < 26 * 26 * 26; i++) {
			if (nodesize[i]) node_cnt++;
		}

		while (node_cnt > 2) {
			// grab a random edge
			int idx = random() % edges.len;
			edge e = extract(&edges, idx);

			int a = e.n1;
			int b = e.n2;

			// skip self-edges
			if (a == b) continue;

			// move all edges from b to a
			// b is n2 but for each edge it can be either n1 or n2
			for (int i = 0; i < edges.len; i++) {
				if (edges.data[i].n1 == b) {
					edges.data[i].n1 = a;
				}
				if (edges.data[i].n2 == b) {
					edges.data[i].n2 = a;
				}
			}

			node_cnt--;
			nodesize[a] += nodesize[b];

			// remove self-edges from the graph
			// gotta go backwards
			for (int i = edges.len - 1; i >= 0; i--) {
				if (edges.data[i].n1 == edges.data[i].n2) {
					extract(&edges, i);
				}
			}
		}

		if (edges.len <= target) {
			return nodesize[edges.data[0].n1] * nodesize[edges.data[0].n2];
		}
	}
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	// 3 letter names
	edge_vec edges = {0};
	while (*data) {
		// read in component name
		int src = idx(data);
		// '???:'
		data += 4;
		while (*data && *data != '\n') {
			data++; // skip the space
			int dst = idx(data);
			// '???'
			data += 3;
			int n1 = src;
			int n2 = dst;
			if (n1 > n2) {
				// n1 is the lower one
				n1 = dst;
				n2 = src;
			}
			append(&edges, (edge){.n1 = n1, .n2 = n2});
		}
		if (*data) data++;
	}

	printf("%zu\n", karger(edges, 3));

	free(edges.data);
}
