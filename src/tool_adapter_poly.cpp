#include "tool_adapter_poly.h"

#include <algorithm>
#include <cmath>
#include <limits>

static glm::dvec2 closest_point_on_segment(glm::dvec2 p, glm::dvec2 a, glm::dvec2 b) {
	glm::dvec2 ab = b - a;
	double ab2 = ab.x * ab.x + ab.y * ab.y;
	if (!(ab2 > 0.0) || !std::isfinite(ab2))
		return a;
	double t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
	if (!std::isfinite(t))
		t = 0.0;
	t = std::max(0.0, std::min(1.0, t));
	return a + t * ab;
}

static bool point_in_polygon(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	bool inside = false;
	std::size_t n = poly.size();
	for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
		const glm::dvec2 pi = poly[i];
		const glm::dvec2 pj = poly[j];
		bool intersect = ((pi.y > p.y) != (pj.y > p.y)) && (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y + 0.0) + pi.x);
		if (intersect)
			inside = !inside;
	}
	return inside;
}

poly_tool_contact_adapter::poly_tool_contact_adapter(const std::vector<glm::dvec2> &poly, double mu, glm::dvec2 vel)
	: m_poly(&poly), m_mu(mu), m_vel(vel) {
	double xmin = std::numeric_limits<double>::infinity();
	double ymin = std::numeric_limits<double>::infinity();
	double xmax = -std::numeric_limits<double>::infinity();
	double ymax = -std::numeric_limits<double>::infinity();
	for (const auto &p : poly) {
		xmin = std::min(xmin, p.x);
		ymin = std::min(ymin, p.y);
		xmax = std::max(xmax, p.x);
		ymax = std::max(ymax, p.y);
	}
	if (!std::isfinite(xmin) || !std::isfinite(ymin) || !std::isfinite(xmax) || !std::isfinite(ymax)) {
		xmin = ymin = xmax = ymax = 0.0;
	}
	m_bbox_min = glm::dvec2(xmin, ymin);
	m_bbox_max = glm::dvec2(xmax, ymax);
}

bool poly_tool_contact_adapter::contact(glm::dvec2 x_slave, tool_contact_hit_2d &out) const {
	out.inside = false;
	out.x_contact = glm::dvec2(0.);
	out.normal = glm::dvec2(0.);

	if (!m_poly || m_poly->size() < 3)
		return false;
	const std::vector<glm::dvec2> &poly = *m_poly;

	if (x_slave.x < m_bbox_min.x || x_slave.x > m_bbox_max.x || x_slave.y < m_bbox_min.y || x_slave.y > m_bbox_max.y) {
		return false;
	}

	if (!point_in_polygon(x_slave, poly))
		return false;

	double best_d2 = std::numeric_limits<double>::infinity();
	glm::dvec2 best_cp(0.);
	std::size_t n = poly.size();
	for (std::size_t i = 0; i < n; i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % n];
		glm::dvec2 cp = closest_point_on_segment(x_slave, a, b);
		glm::dvec2 d = x_slave - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best_cp = cp;
		}
	}

	glm::dvec2 nvec = x_slave - best_cp;
	double n2 = nvec.x * nvec.x + nvec.y * nvec.y;
	if (n2 > 0.0 && std::isfinite(n2)) {
		double inv = 1.0 / std::sqrt(n2);
		nvec *= inv;
	} else {
		nvec = glm::dvec2(0., 1.);
	}

	out.inside = true;
	out.x_contact = best_cp;
	out.normal = nvec;
	return true;
}

glm::dvec2 poly_tool_contact_adapter::velocity_world() const { return m_vel; }
double poly_tool_contact_adapter::mu() const { return m_mu; }
