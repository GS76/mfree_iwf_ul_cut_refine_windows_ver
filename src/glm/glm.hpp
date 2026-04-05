#ifndef MFREE_MINI_GLM_HPP_
#define MFREE_MINI_GLM_HPP_

#include <cmath>
#include <cstddef>

namespace glm {

struct dvec2 {
	double x = 0.;
	double y = 0.;
	dvec2() = default;
	explicit dvec2(double v) : x(v), y(v) {}
	dvec2(double x, double y) : x(x), y(y) {}
};

struct dvec3 {
	double x = 0.;
	double y = 0.;
	double z = 0.;
	dvec3() = default;
	explicit dvec3(double v) : x(v), y(v), z(v) {}
	dvec3(double x, double y, double z) : x(x), y(y), z(z) {}
};

inline dvec2 operator+(const dvec2 &a, const dvec2 &b) { return dvec2(a.x + b.x, a.y + b.y); }
inline dvec2 operator-(const dvec2 &a, const dvec2 &b) { return dvec2(a.x - b.x, a.y - b.y); }
inline dvec2 operator-(const dvec2 &v) { return dvec2(-v.x, -v.y); }
inline dvec2 operator*(const dvec2 &a, const dvec2 &b) { return dvec2(a.x * b.x, a.y * b.y); }
inline dvec2 operator/(const dvec2 &a, const dvec2 &b) { return dvec2(a.x / b.x, a.y / b.y); }
inline dvec2 operator*(double s, const dvec2 &v) { return dvec2(s * v.x, s * v.y); }
inline dvec2 operator*(const dvec2 &v, double s) { return dvec2(s * v.x, s * v.y); }
inline dvec2 operator/(const dvec2 &v, double s) { return dvec2(v.x / s, v.y / s); }
inline dvec2 &operator+=(dvec2 &a, const dvec2 &b) { a.x += b.x; a.y += b.y; return a; }
inline dvec2 &operator-=(dvec2 &a, const dvec2 &b) { a.x -= b.x; a.y -= b.y; return a; }
inline dvec2 &operator*=(dvec2 &a, double s) { a.x *= s; a.y *= s; return a; }
inline dvec2 &operator/=(dvec2 &a, double s) { a.x /= s; a.y /= s; return a; }

inline dvec3 operator+(const dvec3 &a, const dvec3 &b) { return dvec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline dvec3 operator-(const dvec3 &a, const dvec3 &b) { return dvec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline dvec3 operator-(const dvec3 &v) { return dvec3(-v.x, -v.y, -v.z); }
inline dvec3 operator*(const dvec3 &a, const dvec3 &b) { return dvec3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline dvec3 operator/(const dvec3 &a, const dvec3 &b) { return dvec3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline dvec3 operator*(double s, const dvec3 &v) { return dvec3(s * v.x, s * v.y, s * v.z); }
inline dvec3 operator*(const dvec3 &v, double s) { return dvec3(s * v.x, s * v.y, s * v.z); }
inline dvec3 operator/(const dvec3 &v, double s) { return dvec3(v.x / s, v.y / s, v.z / s); }
inline dvec3 &operator+=(dvec3 &a, const dvec3 &b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
inline dvec3 &operator-=(dvec3 &a, const dvec3 &b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
inline dvec3 &operator*=(dvec3 &a, double s) { a.x *= s; a.y *= s; a.z *= s; return a; }
inline dvec3 &operator/=(dvec3 &a, double s) { a.x /= s; a.y /= s; a.z /= s; return a; }

inline double dot(const dvec2 &a, const dvec2 &b) { return a.x * b.x + a.y * b.y; }
inline double dot(const dvec3 &a, const dvec3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline double length(const dvec2 &v) { return std::sqrt(dot(v, v)); }
inline double length(const dvec3 &v) { return std::sqrt(dot(v, v)); }

inline dvec2 normalize(const dvec2 &v) {
	double l = length(v);
	if (l <= 0.) return dvec2(0.);
	return v / l;
}

inline dvec3 normalize(const dvec3 &v) {
	double l = length(v);
	if (l <= 0.) return dvec3(0.);
	return v / l;
}

struct dmat2x2 {
	double v[2][2];
	dmat2x2() { v[0][0] = v[0][1] = v[1][0] = v[1][1] = 0.; }
	explicit dmat2x2(double s) { v[0][0] = v[0][1] = v[1][0] = v[1][1] = s; }
	dmat2x2(double c0r0, double c0r1, double c1r0, double c1r1) {
		v[0][0] = c0r0; v[0][1] = c0r1;
		v[1][0] = c1r0; v[1][1] = c1r1;
	}
	double *operator[](std::size_t i) { return v[i]; }
	const double *operator[](std::size_t i) const { return v[i]; }
};

inline double determinant(const dmat2x2 &m) {
	double m00 = m.v[0][0];
	double m10 = m.v[0][1];
	double m01 = m.v[1][0];
	double m11 = m.v[1][1];
	return m00 * m11 - m01 * m10;
}

inline dmat2x2 inverse(const dmat2x2 &m) {
	double det = determinant(m);
	dmat2x2 inv(0.);
	double m00 = m.v[0][0];
	double m10 = m.v[0][1];
	double m01 = m.v[1][0];
	double m11 = m.v[1][1];
	inv.v[0][0] = m11 / det;
	inv.v[0][1] = -m10 / det;
	inv.v[1][0] = -m01 / det;
	inv.v[1][1] = m00 / det;
	return inv;
}

inline dvec2 operator*(const dvec2 &v, const dmat2x2 &m) {
	double m00 = m.v[0][0];
	double m10 = m.v[0][1];
	double m01 = m.v[1][0];
	double m11 = m.v[1][1];
	return dvec2(v.x * m00 + v.y * m10, v.x * m01 + v.y * m11);
}

inline dvec2 operator*(const dmat2x2 &m, const dvec2 &v) {
	double m00 = m.v[0][0];
	double m10 = m.v[0][1];
	double m01 = m.v[1][0];
	double m11 = m.v[1][1];
	return dvec2(m00 * v.x + m01 * v.y, m10 * v.x + m11 * v.y);
}

inline dmat2x2 operator+(const dmat2x2 &a, const dmat2x2 &b) {
	dmat2x2 r(0.);
	for (int c = 0; c < 2; c++) for (int r0 = 0; r0 < 2; r0++) r.v[c][r0] = a.v[c][r0] + b.v[c][r0];
	return r;
}

inline dmat2x2 operator-(const dmat2x2 &a, const dmat2x2 &b) {
	dmat2x2 r(0.);
	for (int c = 0; c < 2; c++) for (int r0 = 0; r0 < 2; r0++) r.v[c][r0] = a.v[c][r0] - b.v[c][r0];
	return r;
}

inline dmat2x2 operator*(double s, const dmat2x2 &m) {
	dmat2x2 r(0.);
	for (int c = 0; c < 2; c++) for (int r0 = 0; r0 < 2; r0++) r.v[c][r0] = s * m.v[c][r0];
	return r;
}

inline dmat2x2 operator*(const dmat2x2 &m, double s) { return s * m; }

inline dmat2x2 operator*(const dmat2x2 &a, const dmat2x2 &b) {
	dmat2x2 r(0.);
	for (int c = 0; c < 2; c++) {
		for (int r0 = 0; r0 < 2; r0++) {
			r.v[c][r0] = a.v[0][r0] * b.v[c][0] + a.v[1][r0] * b.v[c][1];
		}
	}
	return r;
}

struct dmat3x3 {
	double v[3][3];
	dmat3x3() {
		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) v[i][j] = 0.;
	}
	explicit dmat3x3(double s) {
		for (int c = 0; c < 3; c++) for (int r0 = 0; r0 < 3; r0++) v[c][r0] = 0.;
		v[0][0] = s; v[1][1] = s; v[2][2] = s;
	}
	dmat3x3(double a00, double a01, double a02,
	        double a10, double a11, double a12,
	        double a20, double a21, double a22) {
		v[0][0] = a00; v[0][1] = a10; v[0][2] = a20;
		v[1][0] = a01; v[1][1] = a11; v[1][2] = a21;
		v[2][0] = a02; v[2][1] = a12; v[2][2] = a22;
	}
	double *operator[](std::size_t i) { return v[i]; }
	const double *operator[](std::size_t i) const { return v[i]; }
};

inline double determinant(const dmat3x3 &m) {
	double a = m.v[0][0], b = m.v[1][0], c = m.v[2][0];
	double d = m.v[0][1], e = m.v[1][1], f = m.v[2][1];
	double g = m.v[0][2], h = m.v[1][2], i = m.v[2][2];
	return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

inline dmat3x3 transpose(const dmat3x3 &m) {
	dmat3x3 t;
	for (int c = 0; c < 3; c++) for (int r0 = 0; r0 < 3; r0++) t.v[c][r0] = m.v[r0][c];
	return t;
}

inline dmat3x3 operator+(const dmat3x3 &a, const dmat3x3 &b) {
	dmat3x3 r;
	for (int c = 0; c < 3; c++) for (int r0 = 0; r0 < 3; r0++) r.v[c][r0] = a.v[c][r0] + b.v[c][r0];
	return r;
}

inline dmat3x3 operator-(const dmat3x3 &a, const dmat3x3 &b) {
	dmat3x3 r;
	for (int c = 0; c < 3; c++) for (int r0 = 0; r0 < 3; r0++) r.v[c][r0] = a.v[c][r0] - b.v[c][r0];
	return r;
}

inline dmat3x3 operator*(double s, const dmat3x3 &m) {
	dmat3x3 r;
	for (int c = 0; c < 3; c++) for (int r0 = 0; r0 < 3; r0++) r.v[c][r0] = s * m.v[c][r0];
	return r;
}

inline dmat3x3 operator*(const dmat3x3 &m, double s) { return s * m; }

inline dmat3x3 operator*(const dmat3x3 &a, const dmat3x3 &b) {
	dmat3x3 r;
	for (int c = 0; c < 3; c++) {
		for (int r0 = 0; r0 < 3; r0++) {
			r.v[c][r0] =
				a.v[0][r0] * b.v[c][0] +
				a.v[1][r0] * b.v[c][1] +
				a.v[2][r0] * b.v[c][2];
		}
	}
	return r;
}

}  // namespace glm

#endif
