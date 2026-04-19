#ifndef GEOM_VALIDATION_MATH_H_
#define GEOM_VALIDATION_MATH_H_

#include "glm/glm.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace geom_validation_math {

inline double polygon_signed_area(const std::vector<glm::dvec2> &poly) {
	if (poly.size() < 3) return std::numeric_limits<double>::quiet_NaN();
	double a = 0.0;
	for (std::size_t i = 0; i < poly.size(); i++) {
		const glm::dvec2 p = poly[i];
		const glm::dvec2 q = poly[(i + 1) % poly.size()];
		if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(q.x) || !std::isfinite(q.y)) return std::numeric_limits<double>::quiet_NaN();
		a += p.x * q.y - q.x * p.y;
	}
	if (!std::isfinite(a)) return std::numeric_limits<double>::quiet_NaN();
	a *= 0.5;
	if (std::abs(a) <= 1e-30) return std::numeric_limits<double>::quiet_NaN();
	return a;
}

inline glm::dvec2 polygon_closest_point(glm::dvec2 p, const std::vector<glm::dvec2> &poly, std::size_t *edge_idx_out = nullptr, double *edge_t_out = nullptr) {
	glm::dvec2 best(0.);
	double best_d2 = std::numeric_limits<double>::infinity();
	std::size_t best_i = 0;
	double best_t = 0.0;
	if (poly.size() < 2) return best;
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 ab = b - a;
		double ab2 = ab.x * ab.x + ab.y * ab.y;
		double t = 0.0;
		if (!std::isfinite(ab2) || ab2 < 1e-12) {
			t = 0.0;
		} else {
			t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
			if (!std::isfinite(t)) t = 0.0;
			t = std::max(0.0, std::min(1.0, t));
		}
		glm::dvec2 cp = a + t * ab;
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best = cp;
			best_i = i;
			best_t = t;
		}
	}
	if (edge_idx_out) *edge_idx_out = best_i;
	if (edge_t_out) *edge_t_out = best_t;
	return best;
}

}

#endif

