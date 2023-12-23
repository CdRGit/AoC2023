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

const char test_input[] = "test_input/day23.txt";
const char real_input[] = "real_input/day23.txt";

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
	bool visited;
	int node_idx;
} cell;

typedef struct {
	ssize_t x, y;
	int node_idx;
	enum {
		NORTH,
		EAST,
		SOUTH,
		WEST,
	} direction;
	size_t dist;
} vec2;

typedef struct {
	vec2* data;
	size_t len;
	size_t cap;
} vec2_stack;

void push(vec2_stack* s, vec2 e) {
	if (s->len + 1 > s->cap) {
		s->cap = s->cap ? s->cap * 2 : 1;
		s->data = realloc(s->data, s->cap * sizeof(vec2));
	}
	s->data[s->len++] = e;
}

vec2 pop(vec2_stack* s) {
	return s->data[--s->len];
}

bool has_data(vec2_stack s) {
	return s.len > 0;
}

typedef struct {
	int target;
	size_t dist;
} connection;

typedef struct {
	connection* incoming;
	size_t in_len;
	size_t in_cap;

	connection* outgoing;
	size_t out_len;
	size_t out_cap;

	bool visited;
} node;

typedef struct {
	node* data;
	size_t len;
	size_t cap;
} node_vec;

void append(node_vec* n, node e) {
	if (n->len + 1 > n->cap) {
		n->cap = n->cap ? n->cap * 2 : 1;
		n->data = realloc(n->data, n->cap * sizeof(node));
	}
	n->data[n->len++] = e;
}

void connect(node* vec, int src, int dst, size_t distance) {
	if (vec[src].out_len + 1 > vec[src].out_cap) {
		vec[src].out_cap = vec[src].out_cap ? vec[src].out_cap * 2 : 1;
		vec[src].outgoing = realloc(vec[src].outgoing, vec[src].out_cap * sizeof(connection));
	}
	vec[src].outgoing[vec[src].out_len++] = (connection){.target = dst, .dist = distance};

	if (vec[dst].in_len + 1 > vec[dst].in_cap) {
		vec[dst].in_cap = vec[dst].in_cap ? vec[dst].in_cap * 2 : 1;
		vec[dst].incoming = realloc(vec[dst].incoming, vec[dst].in_cap * sizeof(connection));
	}
	vec[dst].incoming[vec[dst].in_len++] = (connection){.target = src, .dist = distance};
}

void part_1(const char* path, const uint8_t* data) {
	printf("part 1: %s\n", path);
	size_t width = 0, height = 0;
	const uint8_t* scanner = data;
	for (; *scanner != '\n'; scanner++, width++);
	for (; *scanner; scanner++, height++)
		for (; *scanner && *scanner != '\n'; scanner++);

	cell* grid = malloc(width * height * sizeof(cell));
	memset(grid, 0, width * height * sizeof(cell));

	node_vec nodes = {0};
	append(&nodes, (node){0});

	vec2 start = {.x = 1, .y = 0, .node_idx = 0, .direction = SOUTH, .dist = 0};
	vec2_stack s = {0};
	int target_idx = -1;
	push(&s, start);
	while (has_data(s)) {
		vec2 curr = pop(&s);
		if (curr.x < 0 || curr.x >= width || curr.y < 0 || curr.y >= height)
			continue;
		size_t vi = curr.x + curr.y * width;
		size_t di = curr.x + curr.y * (width + 1);
		if (grid[vi].visited) {
			if (grid[vi].node_idx != -1) {
				connect(nodes.data, curr.node_idx, grid[vi].node_idx, curr.dist);
			}
			continue;
		}
		grid[vi].visited = true;
		grid[vi].node_idx = -1;
		if (curr.x == width - 2 && curr.y == height - 1) {
			target_idx = nodes.len;
			append(&nodes, (node){0});
			connect(nodes.data, curr.node_idx, target_idx, curr.dist);
			grid[vi].node_idx = target_idx;
			continue;
		}
		switch (data[di]) {
			case '.': {
					size_t dirs = 0;
					size_t paths = 0;
					size_t in_paths = 1;
					size_t ni = di - width - 1;
					size_t ei = di + 1;
					size_t si = di + width + 1;
					size_t wi = di - 1;
					// north
					if (curr.y > 0 && curr.direction != SOUTH) {
						if (data[ni] == '.' || data[ni] == '^') {
							paths++;
							dirs |= 1 << NORTH;
						} else if (data[ni] == 'v') {
							in_paths++;
						}
					}
					// east
					if (curr.x < width - 1 && curr.direction != WEST) {
						if (data[ei] == '.' || data[ei] == '>') {
							paths++;
							dirs |= 1 << EAST;
						} else if (data[ei] == '<') {
							in_paths++;
						}
					}
					// south
					if (curr.y < height - 1 && curr.direction != NORTH) {
						if (data[si] == '.' || data[si] == 'v') {
							paths++;
							dirs |= 1 << SOUTH;
						} else if (data[si] == '^') {
							in_paths++;
						}
					}
					// west
					if (curr.x > 0 && curr.direction != EAST) {
						if (data[wi] == '.' || data[wi] == '<') {
							paths++;
							dirs |= 1 << WEST;
						} else if (data[wi] == '>') {
							in_paths++;
						}
					}
					assert(in_paths <= 2);
					assert(paths <= 2);
					int idx = curr.node_idx;
					size_t dist = curr.dist + 1;
					if (paths != 1 || in_paths != 1) {
						idx = nodes.len;
						dist = 1;
						append(&nodes, (node){0});
						connect(nodes.data, curr.node_idx, idx, curr.dist);
						grid[vi].node_idx = idx;
					}
					if (dirs & (1 << NORTH)) {
						push(&s, (vec2){.x = curr.x, .y = curr.y - 1, .direction = NORTH, .node_idx = idx, .dist = dist});
					}
					if (dirs & (1 << EAST)) {
						push(&s, (vec2){.x = curr.x + 1, .y = curr.y, .direction = EAST, .node_idx = idx, .dist = dist});
					}
					if (dirs & (1 << SOUTH)) {
						push(&s, (vec2){.x = curr.x, .y = curr.y + 1, .direction = SOUTH, .node_idx = idx, .dist = dist});
					}
					if (dirs & (1 << WEST)) {
						push(&s, (vec2){.x = curr.x - 1, .y = curr.y, .direction = WEST, .node_idx = idx, .dist = dist});
					}
				}
				break;
			case '^':
				push(&s, (vec2){.x = curr.x, .y = curr.y - 1, .direction = NORTH, .node_idx = curr.node_idx, .dist = curr.dist + 1});
				break;
			case '>':
				push(&s, (vec2){.x = curr.x + 1, .y = curr.y, .direction = EAST, .node_idx = curr.node_idx, .dist = curr.dist + 1});
				break;
			case 'v':
				push(&s, (vec2){.x = curr.x, .y = curr.y + 1, .direction = SOUTH, .node_idx = curr.node_idx, .dist = curr.dist + 1});
				break;
			case '<':
				push(&s, (vec2){.x = curr.x - 1, .y = curr.y, .direction = WEST, .node_idx = curr.node_idx, .dist = curr.dist + 1});
				break;
			default:
				printf("%c\n", data[di]);
				assert(false && "TODO");
				break;
		}
	}

	// graphs are directed, acyclic
	// let's set up a large list of values
	ssize_t* dist_from_end = malloc(nodes.len * sizeof(ssize_t));
	for (size_t i = 0; i < nodes.len; i++) {
		dist_from_end[i] = -1;
	}
	// so we can work backwards from the end
	// we want to repeatedly make each node the maximum of its output nodes + their distance
	// (assuming none of its outputs are -1)
	// until there is no more nodes with a distance of -1
	// then the distance of the initial node should be the final distance
	for (;;) {
		for (size_t i = 0; i < nodes.len; i++) {
			ssize_t max = 0;
			size_t in_non_negatives = 0;
			for (size_t j = 0; j < nodes.data[i].out_len; j++, in_non_negatives++) {
				connection conn = nodes.data[i].outgoing[j];
				if (dist_from_end[conn.target] == -1) break;
				ssize_t dist = dist_from_end[conn.target] + conn.dist;
				if (dist > max) {
					max = dist;
				}
			}
			if (in_non_negatives == nodes.data[i].out_len) {
				dist_from_end[i] = max;
			}
		}

		size_t non_negatives = 0;
		for (size_t i = 0; i < nodes.len; i++, non_negatives++) {
			if (dist_from_end[i] == -1) break;
		}
		if (non_negatives == nodes.len) break;
	}

	printf("%zd\n", dist_from_end[0]);

	// free all the nodes
	for (size_t i = 0; i < nodes.len; i++) {
		free(nodes.data[i].incoming);
		free(nodes.data[i].outgoing);
	}

	free(dist_from_end);
	free(grid);
	free(nodes.data);
	free(s.data);
}

void connect_bidirectional(node* vec, int src, int dst, size_t distance) {
	connect(vec, src, dst, distance);
	connect(vec, dst, src, distance);
}

ssize_t find_longest_path(node_vec nodes, int src, int dst, int call_depth) {
	// 0 means found the end
	if (src == dst) return 0;
	// mark as visited
	nodes.data[src].visited = true;
	// -1 means no viable path
	ssize_t max = -1;
	size_t furthest = 0;
	for (size_t i = 0; i < nodes.data[src].out_len; i++) {
		connection next = nodes.data[src].outgoing[i];

		// skip the visited ones
		if (nodes.data[next.target].visited) continue;

		// we gotta go deeper
		ssize_t dist = find_longest_path(nodes, next.target, dst, call_depth+1);
		if (dist != -1) {
			// I HATE DATATYPES I HATE DATATYPES I HATE DATATYPES
			if (dist + (ssize_t)next.dist > max) {
				max = dist + next.dist;
				furthest = next.target;
			}
		}
	}
	// un-mark
	nodes.data[src].visited = false;
	return max;
}

void part_2(const char* path, const uint8_t* data) {
	printf("part 2: %s\n", path);
	size_t width = 0, height = 0;
	const uint8_t* scanner = data;
	for (; *scanner != '\n'; scanner++, width++);
	for (; *scanner; scanner++, height++)
		for (; *scanner && *scanner != '\n'; scanner++);

	cell* grid = malloc(width * height * sizeof(cell));
	memset(grid, 0, width * height * sizeof(cell));

	node_vec nodes = {0};
	append(&nodes, (node){0});

	vec2 start = {.x = 1, .y = 0, .node_idx = 0, .direction = SOUTH, .dist = 0};
	vec2_stack s = {0};
	int target_idx = -1;
	push(&s, start);
	while (has_data(s)) {
		vec2 curr = pop(&s);
		if (curr.x < 0 || curr.x >= width || curr.y < 0 || curr.y >= height)
			continue;
		size_t vi = curr.x + curr.y * width;
		size_t di = curr.x + curr.y * (width + 1);
		if (grid[vi].visited) {
			if (grid[vi].node_idx != -1) {
				connect_bidirectional(nodes.data, curr.node_idx, grid[vi].node_idx, curr.dist);
			}
			continue;
		}
		grid[vi].visited = true;
		grid[vi].node_idx = -1;
		if (curr.x == width - 2 && curr.y == height - 1) {
			target_idx = nodes.len;
			append(&nodes, (node){0});
			connect_bidirectional(nodes.data, curr.node_idx, target_idx, curr.dist);
			grid[vi].node_idx = target_idx;
			continue;
		}
		switch (data[di]) {
			case '>':
			case '<':
			case '^':
			case 'v':
			case '.': {
					size_t dirs = 0;
					size_t paths = 1;
					size_t ni = di - width - 1;
					size_t ei = di + 1;
					size_t si = di + width + 1;
					size_t wi = di - 1;
					// north
					if (curr.y > 0 && curr.direction != SOUTH) {
						if (data[ni] != '#') {
							paths++;
							dirs |= 1 << NORTH;
						}
					}
					// east
					if (curr.x < width - 1 && curr.direction != WEST) {
						if (data[ei] != '#') {
							paths++;
							dirs |= 1 << EAST;
						}
					}
					// south
					if (curr.y < height - 1 && curr.direction != NORTH) {
						if (data[si] != '#') {
							paths++;
							dirs |= 1 << SOUTH;
						}
					}
					// west
					if (curr.x > 0 && curr.direction != EAST) {
						if (data[wi] != '#') {
							paths++;
							dirs |= 1 << WEST;
						}
					}
					int idx = curr.node_idx;
					size_t dist = curr.dist + 1;
					if (paths > 2) {
						idx = nodes.len;
						dist = 1;
						append(&nodes, (node){0});
						connect_bidirectional(nodes.data, curr.node_idx, idx, curr.dist);
						grid[vi].node_idx = idx;
					}
					if (dirs & (1 << NORTH)) {
						push(&s, (vec2){.x = curr.x, .y = curr.y - 1, .direction = NORTH, .node_idx = idx, .dist = dist});
					}
					if (dirs & (1 << EAST)) {
						push(&s, (vec2){.x = curr.x + 1, .y = curr.y, .direction = EAST, .node_idx = idx, .dist = dist});
					}
					if (dirs & (1 << SOUTH)) {
						push(&s, (vec2){.x = curr.x, .y = curr.y + 1, .direction = SOUTH, .node_idx = idx, .dist = dist});
					}
					if (dirs & (1 << WEST)) {
						push(&s, (vec2){.x = curr.x - 1, .y = curr.y, .direction = WEST, .node_idx = idx, .dist = dist});
					}
				}
				break;
			default:
				printf("%c\n", data[di]);
				assert(false && "TODO");
				break;
		}
	}

	// bad news, we are no longer acyclic or directed
	// :3
	// this problem is now NP-hard
	// luckily the number of nodes and edges is still limited tho
	
	// she depth on my first until I search her
	ssize_t longest = find_longest_path(nodes, 0, target_idx, 0);
	printf("%zd\n", longest);

	// free all the nodes
	for (size_t i = 0; i < nodes.len; i++) {
		free(nodes.data[i].incoming);
		free(nodes.data[i].outgoing);
	}

	free(grid);
	free(nodes.data);
	free(s.data);
}
