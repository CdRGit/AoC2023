#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <math.h>

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

const char test_input[] = "test_input/day24.txt";
const char real_input[] = "real_input/day24.txt";

void part_1(const char* path, const uint8_t* data, bool test_input);
void part_2(const char* path, const uint8_t* data, bool test_input);

int main(int argc, const char** argv) {
	uint8_t* test_data = read_data(test_input);

	part_1(test_input, test_data, true);
	part_2(test_input, test_data, true);

	if (TRY_REAL_INPUT) {
		printf("---\n");
		uint8_t* real_data = read_data(real_input);
		part_1(real_input, real_data, false);
		part_2(real_input, real_data, false);
	}
}

typedef struct {
	double px, py, pz;
	double vx, vy, vz;
} hailstone;

typedef struct {
	hailstone* data;
	size_t len;
	size_t cap;
} vec;

void append(vec* v, hailstone e) {
	if (v->len + 1 > v->cap) {
		v->cap = v->cap ? v->cap * 2 : 1;
		v->data = realloc(v->data, v->cap * sizeof(hailstone));
	}
	v->data[v->len++] = e;
}

void part_1(const char* path, const uint8_t* data, bool test_input) {
	const double box_min = test_input ? 7  : 200000000000000;
	const double box_max = test_input ? 27 : 400000000000000;
	printf("part 1: %s\n", path);
	vec v = {0};
	while (*data) {
		hailstone h = {0};
		while (isdigit(*data)) {
			h.px *= 10;
			h.px += *data++ - '0';
		}
		data += 2; // ', '
		while (isdigit(*data)) {
			h.py *= 10;
			h.py += *data++ - '0';
		}
		data += 2; // ', '
		while (isdigit(*data)) {
			h.pz *= 10;
			h.pz += *data++ - '0';
		}
		data += 2;
		while (*data == ' ') data++;
		bool xneg = false, yneg = false, zneg = false;
		if (*data == '-') {
			data++;
			xneg = true;
		}
		while (isdigit(*data)) {
			h.vx *= 10;
			h.vx += *data++ - '0';
		}
		data++;
		while (*data == ' ') data++;
		if (*data == '-') {
			data++;
			yneg = true;
		}
		while (isdigit(*data)) {
			h.vy *= 10;
			h.vy += *data++ - '0';
		}
		data++;
		while (*data == ' ') data++;
		if (*data == '-') {
			data++;
			zneg = true;
		}
		while (isdigit(*data)) {
			h.vz *= 10;
			h.vz += *data++ - '0';
		}
		if (*data) data++;
		h.vx *= xneg ? -1 : 1;
		h.vy *= yneg ? -1 : 1;
		h.vz *= zneg ? -1 : 1;
		assert(h.vx != 0.0);
		append(&v, h);
	}

	// ax + c = bx + d
	size_t count = 0;
	for (int i = 0; i < v.len; i++) {
		hailstone first = v.data[i];
		double a = first.vy / first.vx;
		// y = ax + c
		// y - y0 = a(x - x0)
		// y - y0 = ax - ax0
		// y = ax - ax0 + y0
		// c = - ax0 + y0
		double c = -a * first.px + first.py;
		for (int j = i + 1; j < v.len; j++) {
			hailstone second = v.data[j];
			double b = second.vy / second.vx;
			double d = -b * second.px + second.py;

			if (a == b) {
				continue;
			}
			// ax - bx = d - c
			// x = (d - c) / (a - b)
			double x = (d - c) / (a - b);
			double y = a * x + c;
			// are we inside the test area
			if (x < box_min || x > box_max || y < box_min || y > box_max) {
				continue;
			}
			// is it in the future for `first`
			double dx1 = x - first.px;
			double dy1 = y - first.py;
			if (signbit(dx1) != signbit(first.vx) || signbit(dy1) != signbit(first.vy)) {
				// in the past
				continue;
			}
			// is it in the future for `second`
			double dx2 = x - second.px;
			double dy2 = y - second.py;
			if (signbit(dx2) != signbit(second.vx) || signbit(dy2) != signbit(second.vy)) {
				// in the past
				continue;
			}
			count++;
		}
	}
	printf("%zu\n", count);

	free(v.data);
}

// linear algebra implementation time
double determinant2x2(double a, double b, double c, double d) {
	return a * d - b * c;
}

double determinant3x3(double mat[3][3]) {
	double det = 0;
	for (int i = 0; i < 3; i++) {
		det += mat[0][i] * determinant2x2(mat[1][(i + 1) % 3], mat[1][(i + 2) % 3], mat[2][(i + 1) % 3], mat[2][(i + 2) % 3]);
	}
	return det;
}

double determinant4x4(double mat[4][4]) {
	double det = 0;
	for (int i = 0; i < 4; i++) {
		double minorMatrix[3][3];
		for (int j = 1; j < 4; j++) {
			int col = 0;
			for (int k = 0; k < 4; k++) {
				if (k != i) {
					minorMatrix[j - 1][col] = mat[j][k];
					col++;
				}
			}
		}
		double minorDet = determinant3x3(minorMatrix);
		det += (i % 2 == 0 ? 1 : -1) * mat[0][i] * minorDet;
	}
	return det;
}

void getCofactor(double mat[4][4], double (*temp)[4][4], int p, int q) {
	int i = 0, j = 0;

	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			if (row != p && col != q) {
				(*temp)[i][j++] = mat[row][col];
				if (j == 3) {
					j = 0;
					i++;
				}
			}
		}
	}
}

// blatantly stole the algorithm from https://github.com/fdlk/advent-2023/blob/master/day24.sc
void part_2(const char* path, const uint8_t* data, bool test_input) {
	printf("part 2: %s\n", path);
	vec v = {0};
	while (*data) {
		hailstone h = {0};
		while (isdigit(*data)) {
			h.px *= 10;
			h.px += *data++ - '0';
		}
		data += 2; // ', '
		while (isdigit(*data)) {
			h.py *= 10;
			h.py += *data++ - '0';
		}
		data += 2; // ', '
		while (isdigit(*data)) {
			h.pz *= 10;
			h.pz += *data++ - '0';
		}
		data += 2;
		while (*data == ' ') data++;
		bool xneg = false, yneg = false, zneg = false;
		if (*data == '-') {
			data++;
			xneg = true;
		}
		while (isdigit(*data)) {
			h.vx *= 10;
			h.vx += *data++ - '0';
		}
		data++;
		while (*data == ' ') data++;
		if (*data == '-') {
			data++;
			yneg = true;
		}
		while (isdigit(*data)) {
			h.vy *= 10;
			h.vy += *data++ - '0';
		}
		data++;
		while (*data == ' ') data++;
		if (*data == '-') {
			data++;
			zneg = true;
		}
		while (isdigit(*data)) {
			h.vz *= 10;
			h.vz += *data++ - '0';
		}
		if (*data) data++;
		h.vx *= xneg ? -1 : 1;
		h.vy *= yneg ? -1 : 1;
		h.vz *= zneg ? -1 : 1;
		assert(h.vx != 0.0);
		append(&v, h);
	}

	double A[4][4] = {0};
	double x[4] = {0};
	hailstone pairs[4][2] = {0};
	size_t pair_cnt = 0;
	for (int i = 0; i < v.len; i++) {
		hailstone first = v.data[i];
		for (int j = i + 1; j < v.len; j++) {
			hailstone second = v.data[j];
			pairs[pair_cnt][0] = first;
			pairs[pair_cnt++][1] = second;
			if (pair_cnt == 4) goto pairs_got;
		}
	}
pairs_got:
	// time to calculate the coefficients?
	// https://github.com/fdlk/advent-2023/blob/e0cdc66197550daf77500707fe2c705088292f6c/day24.sc#L63
	for (int i = 0; i < 4; i++) {
		hailstone h1 = pairs[i][0];
		hailstone h2 = pairs[i][1];
		A[i][0] = h2.vy - h1.vy;
		A[i][1] = h1.vx - h2.vx;
		A[i][2] = h1.py - h2.py;
		A[i][3] = h2.px - h1.px;
		x[i] = h1.vx * h1.py - h2.vx * h2.py + h2.px * h2.vy - h1.px * h1.vy;
	}
	// matrix inverting time
	double adj[4][4] = {0};
	double det = determinant4x4(A);

	int sign = 1;
	// cofactors stored here
	double temp[4][4] = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			getCofactor(A, &temp, i, j);

			sign = ((i + j) % 2 == 0) ? 1 : -1;
			double ntmp[3][3] = {0};
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					ntmp[k][l] = temp[k][l];
				}
			}
			adj[j][i] = sign * determinant3x3(ntmp);
		}
	}


	// store inverse in A
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			A[i][j] = adj[i][j] / det;
		}
	}

	// matrix vector multiplication
	// but I only care about the `[i][0] value`
	double result[4] = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i] += A[i][j] * x[j];
		}
		result[i] = round(result[i]);
	}

	double a = result[0],
	       b = result[1],
	       d = result[2],
	       e = result[3];

	hailstone h1 = v.data[0];
	double t1 = (a - h1.px) / (h1.vx - d);

	hailstone h2 = v.data[1];
	double t2 = (a - h2.px) / (h2.vx - d);

	double f = ((h1.pz - h2.pz) + t1 * h1.vz - t2 * h2.vz) / (t1 - t2);
	double c = h1.pz + t1 * (h1.vz - f);

	size_t val = (size_t)(round(a) + round(b) + round(c));
	printf("%zu\n", val);

	free(v.data);
}
