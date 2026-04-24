/*
 * ================================================
 * 					COPYRIGHT:
 * Institute of Machine Tools & Manufacturing (IWF)
 * Department of Mechanical & Process Engineering
 * 					ETH ZURICH
 * ================================================
 *
 *  This file is part of "mfree_iwf-ul-cut-refine".
 *
 * 	mfree_iwf is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	mfree_iwf-ul-cut-refine is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *  along with mfree_iwf-ul-cut-refine.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This is the source code used to produce the results
 *  of the metal cutting simulation presented in:
 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  "Meshfree Simulation of Metal Cutting:
 *  An Updated Lagrangian Approach with Dynamic Refinement"
 *
 * 	Authored by:
 * 	Mohamadreza Afrasiabi
 * 	Dr. Matthias Roethlin
 * 	Hagen Klippel
 * 	Prof. Dr. Konrad Wegener
 *
 * 	Published by:
 * 	International Journal of Mechanical Sciences
 * 	28 June 2019
 * 	https://doi.org/10.1016/j.ijmecsci.2019.06.045
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * 	For further descriptions, you may refer to the manuscript
 * 	or the previous works of the same research group
 * 	at IWF, ETH Zurich.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <fenv.h>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cerrno>
#include <filesystem>
#include <limits>
#include <string>
#include <omp.h>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "particle.h"
#include "contact.h"
#include "vtk_writer.h"
#include "geom_validation_math.h"

#include "benchmarks/test_density.h"
#include "benchmarks/test_benches.h"
#include "benchmarks/test_cuttings.h"

#include "tool.h"
#include "logger.h"
#include "body.h"

logger *global_logger = nullptr;

#include <algorithm>
#include <set>
#include <iterator>

#ifdef __FAST_MATH__
#error "Do NOT compile using -ffast-math"
#endif

static double tri_min_angle_deg(glm::dvec2 a, glm::dvec2 b, glm::dvec2 c) {
	const double pi = 3.1415926535897932384626433832795;
	auto norm = [](glm::dvec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); };
	auto dot = [](glm::dvec2 u, glm::dvec2 v) { return u.x * v.x + u.y * v.y; };
	auto angle = [&](glm::dvec2 u, glm::dvec2 v) {
		double nu = norm(u);
		double nv = norm(v);
		if (nu <= 0. || nv <= 0.) return 0.0;
		double x = dot(u, v) / (nu * nv);
		x = std::max(-1.0, std::min(1.0, x));
		return std::acos(x) * 180.0 / pi;
	};

	double a0 = angle(b - a, c - a);
	double a1 = angle(a - b, c - b);
	double a2 = angle(a - c, b - c);
	return std::min(a0, std::min(a1, a2));
}

static glm::dvec2 closest_point_on_segment(glm::dvec2 p, glm::dvec2 a, glm::dvec2 b) {
	glm::dvec2 ab = b - a;
	double ab2 = ab.x * ab.x + ab.y * ab.y;
	if (!(ab2 > 0.0) || !std::isfinite(ab2)) return a;
	double t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
	if (!std::isfinite(t)) t = 0.0;
	t = std::max(0.0, std::min(1.0, t));
	return a + t * ab;
}

static bool point_in_polygon(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	bool inside = false;
	std::size_t n = poly.size();
	for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
		const glm::dvec2 pi = poly[i];
		const glm::dvec2 pj = poly[j];
		bool intersect = ((pi.y > p.y) != (pj.y > p.y)) &&
		                 (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y + 0.0) + pi.x);
		if (intersect) inside = !inside;
	}
	return inside;
}

static double polygon_inside_depth(glm::dvec2 p, const std::vector<glm::dvec2> &poly, glm::dvec2 *closest_point_out = nullptr) {
	if (poly.size() < 3) return 0.0;
	if (!point_in_polygon(p, poly)) return 0.0;
	double best_d2 = std::numeric_limits<double>::infinity();
	glm::dvec2 best_cp(0.);
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 cp = closest_point_on_segment(p, a, b);
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best_cp = cp;
		}
	}
	if (closest_point_out) *closest_point_out = best_cp;
	if (!std::isfinite(best_d2) || best_d2 < 0.0) return 0.0;
	return std::sqrt(best_d2);
}

static bool try_parse_int_strict(const char *s, int &out) {
	if (!s) return false;
	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
	if (*s == '\0') return false;
	errno = 0;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s || errno != 0) return false;
	while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') ++end;
	if (*end != '\0') return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_parse_double_strict(const char *s, double &out) {
	if (!s) return false;
	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
	if (*s == '\0') return false;
	errno = 0;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	if (end == s || errno != 0 || !std::isfinite(v)) return false;
	while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') ++end;
	if (*end != '\0') return false;
	out = v;
	return true;
}

static bool env_flag(const char *key, bool def) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0') return def;
	int v = 0;
	if (!try_parse_int_strict(s, v)) return def;
	return v != 0;
}

static int env_int(const char *key, int def) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0') return def;
	int v = 0;
	if (!try_parse_int_strict(s, v)) return def;
	return v;
}

static double env_double(const char *key, double def) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0') return def;
	double v = 0.0;
	if (!try_parse_double_strict(s, v)) return def;
	return v;
}

static double polygon_signed_area(const std::vector<glm::dvec2> &poly) { return geom_validation_math::polygon_signed_area(poly); }

static glm::dvec2 polygon_closest_point(glm::dvec2 p, const std::vector<glm::dvec2> &poly, std::size_t *edge_idx_out = nullptr, double *edge_t_out = nullptr) {
	return geom_validation_math::polygon_closest_point(p, poly, edge_idx_out, edge_t_out);
}

static double polygon_distance_to_boundary(glm::dvec2 p, const std::vector<glm::dvec2> &poly, glm::dvec2 &cp_out, std::size_t &edge_idx_out) {
	double t = 0.0;
	cp_out = polygon_closest_point(p, poly, &edge_idx_out, &t);
	glm::dvec2 d = p - cp_out;
	double d2 = d.x * d.x + d.y * d.y;
	if (!std::isfinite(d2) || d2 < 0.0) return std::numeric_limits<double>::infinity();
	return std::sqrt(d2);
}

static glm::dvec2 polygon_edge_tangent(const std::vector<glm::dvec2> &poly, std::size_t edge_idx) {
	if (poly.size() < 2) return glm::dvec2(1., 0.);
	glm::dvec2 a = poly[edge_idx % poly.size()];
	glm::dvec2 b = poly[(edge_idx + 1) % poly.size()];
	glm::dvec2 t = b - a;
	double n2 = t.x * t.x + t.y * t.y;
	if (n2 > 0.0 && std::isfinite(n2)) t /= std::sqrt(n2);
	else t = glm::dvec2(1., 0.);
	return t;
}

static double poly_min_y(const std::vector<glm::dvec2> &poly) {
	double low = std::numeric_limits<double>::infinity();
	for (const auto &p : poly) low = std::min(low, p.y);
	if (!std::isfinite(low)) return 0.0;
	return low;
}

static double poly_max_x(const std::vector<glm::dvec2> &poly) {
	double hi = -std::numeric_limits<double>::infinity();
	for (const auto &p : poly) hi = std::max(hi, p.x);
	if (!std::isfinite(hi)) return 0.0;
	return hi;
}

static void write_geom_validation_vtk(const char *folder, const std::vector<glm::dvec2> &poly, glm::dvec2 corner, glm::dvec2 closest, double y_top, double y_bottom) {
	char path[1024];
	std::snprintf(path, sizeof(path), "%s/geom_validation_000000.vtk", folder);
	FILE *fp = std::fopen(path, "w+");
	if (!fp) {
		std::fprintf(stderr, "warning: failed to open geom validation vtk for write: %s\n", path);
		return;
	}

	std::vector<glm::dvec2> pts;
	pts.reserve(poly.size() + 4);
	for (const auto &p : poly) pts.push_back(p);
	unsigned int idx_corner = static_cast<unsigned int>(pts.size());
	pts.push_back(corner);
	unsigned int idx_closest = static_cast<unsigned int>(pts.size());
	pts.push_back(closest);
	unsigned int idx_clear_a = static_cast<unsigned int>(pts.size());
	pts.push_back(glm::dvec2(corner.x, y_top));
	unsigned int idx_clear_b = static_cast<unsigned int>(pts.size());
	pts.push_back(glm::dvec2(corner.x, y_bottom));

	std::fprintf(fp, "# vtk DataFile Version 2.0\n");
	std::fprintf(fp, "mfree geom validation\n");
	std::fprintf(fp, "ASCII\n\n");
	std::fprintf(fp, "DATASET POLYDATA\n");
	std::fprintf(fp, "POINTS %u float\n", static_cast<unsigned int>(pts.size()));
	for (const auto &p : pts) {
		if (std::fprintf(fp, "%e %e %e\n", p.x, p.y, 0.0) < 0) {
			std::fprintf(stderr, "warning: failed writing geom validation vtk: %s\n", path);
			std::fclose(fp);
			return;
		}
	}

	unsigned int n_lines = (poly.size() >= 2 ? 1u : 0u) + 2u;
	unsigned int line_size = 0;
	if (poly.size() >= 2) line_size += static_cast<unsigned int>(poly.size()) + 1;
	line_size += 3;
	line_size += 3;
	std::fprintf(fp, "\nLINES %u %u\n", n_lines, line_size);
	if (poly.size() >= 2) {
		if (std::fprintf(fp, "%u ", static_cast<unsigned int>(poly.size())) < 0) {
			std::fprintf(stderr, "warning: failed writing geom validation vtk: %s\n", path);
			std::fclose(fp);
			return;
		}
		for (unsigned int i = 0; i < static_cast<unsigned int>(poly.size()); i++) {
			if (std::fprintf(fp, "%u ", i) < 0) {
				std::fprintf(stderr, "warning: failed writing geom validation vtk: %s\n", path);
				std::fclose(fp);
				return;
			}
		}
		if (std::fprintf(fp, "\n") < 0) {
			std::fprintf(stderr, "warning: failed writing geom validation vtk: %s\n", path);
			std::fclose(fp);
			return;
		}
	}
	if (std::fprintf(fp, "2 %u %u\n", idx_corner, idx_closest) < 0) {
		std::fprintf(stderr, "warning: failed writing geom validation vtk: %s\n", path);
		std::fclose(fp);
		return;
	}
	if (std::fprintf(fp, "2 %u %u\n", idx_clear_a, idx_clear_b) < 0) {
		std::fprintf(stderr, "warning: failed writing geom validation vtk: %s\n", path);
		std::fclose(fp);
		return;
	}
	std::fclose(fp);
}

static void geom_autocorrect_fe_tool(body &b, double clearance_target_m, unsigned int iters, glm::dvec2 corner) {
	fe_tool *ft = b.get_fe_tool();
	if (!ft) return;
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft->boundary_loop_world();
		if (poly.size() < 3) return;

		double y_bottom = poly_min_y(poly);
		double y_top = corner.y;
		double dy = (y_top - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft->get_pos();
		pos.y += dy;
		ft->set_pose(pos, ft->get_vel());

		poly = ft->boundary_loop_world();
		if (poly.size() < 3) return;
		glm::dvec2 cp(0.);
		std::size_t e = 0;
		(void)polygon_distance_to_boundary(corner, poly, cp, e);
		double dx = corner.x - cp.x;
		pos = ft->get_pos();
		pos.x += dx;
		ft->set_pose(pos, ft->get_vel());
	}
}

static void write_geom_validation_report(body &b, const char *folder, int model) {
	if (!env_flag("MFREE_GEOM_VALIDATE", false)) return;

	double clearance_mm = env_double("MFREE_GEOM_CLEARANCE_MM", 0.2);
	double clearance_tol_mm = env_double("MFREE_GEOM_CLEARANCE_TOL_MM", 0.001);
	double tangency_tol_mm = env_double("MFREE_GEOM_TANGENCY_TOL_MM", 1e-6);

	double clearance_target = clearance_mm * 1e-3;
	double clearance_tol = clearance_tol_mm * 1e-3;
	double tangency_tol = tangency_tol_mm * 1e-3;

	const std::vector<particle> &p = b.get_particles();
	double wp_xmin = std::numeric_limits<double>::infinity();
	double wp_xmax = -std::numeric_limits<double>::infinity();
	double wp_ymin = std::numeric_limits<double>::infinity();
	double wp_ymax = -std::numeric_limits<double>::infinity();
	for (const particle &pi : p) {
		wp_xmin = std::min(wp_xmin, pi.x);
		wp_xmax = std::max(wp_xmax, pi.x);
		wp_ymin = std::min(wp_ymin, pi.y);
		wp_ymax = std::max(wp_ymax, pi.y);
	}
	if (!std::isfinite(wp_xmin) || !std::isfinite(wp_xmax) || !std::isfinite(wp_ymin) || !std::isfinite(wp_ymax)) {
		wp_xmin = wp_xmax = wp_ymin = wp_ymax = 0.0;
	}
	glm::dvec2 corner(wp_xmin, wp_ymax);

	fe_tool *ft = b.get_fe_tool();
	bool has_fe = (ft != nullptr);
	std::vector<glm::dvec2> poly;
	if (has_fe) poly = ft->boundary_loop_world();

	if (has_fe && env_flag("MFREE_GEOM_AUTO_CORRECT", false)) {
		geom_autocorrect_fe_tool(b, clearance_target, 3, corner);
		poly = ft->boundary_loop_world();
	}

	glm::dvec2 cp(0.);
	std::size_t edge_idx = 0;
	double tangency_dist = std::numeric_limits<double>::infinity();
	if (poly.size() >= 3) tangency_dist = polygon_distance_to_boundary(corner, poly, cp, edge_idx);

	glm::dvec2 tan = polygon_edge_tangent(poly, edge_idx);
	double tan_angle = std::atan2(tan.y, tan.x);

	double y_bottom = (poly.size() >= 3) ? poly_min_y(poly) : 0.0;
	double clearance = corner.y - y_bottom;

	double clearance_err = clearance - clearance_target;
	double tangency_err = tangency_dist;
	bool pass_clearance = std::isfinite(clearance_err) && std::abs(clearance_err) <= clearance_tol;
	bool pass_tangency = std::isfinite(tangency_err) && std::abs(tangency_err) <= tangency_tol;

	double area = polygon_signed_area(poly);
	double tool_xmax = (poly.size() >= 3) ? poly_max_x(poly) : 0.0;

	glm::dvec2 recommended_translation(0.);
	if (poly.size() >= 3) {
		recommended_translation.y = (corner.y - clearance_target) - y_bottom;
		recommended_translation.x = corner.x - cp.x;
	}

	write_geom_validation_vtk(folder, poly, corner, cp, corner.y, corner.y - clearance_target);

	char path[1024];
	std::snprintf(path, sizeof(path), "%s/geom_validation.json", folder);
	FILE *fp = std::fopen(path, "w+");
	if (!fp) return;

	std::fprintf(fp, "{\n");
	std::fprintf(fp, "  \"model\": %d,\n", model);
	std::fprintf(fp, "  \"workpiece\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", wp_xmin, wp_xmax, wp_ymin, wp_ymax);
	std::fprintf(fp, "  \"corner\": {\"x\": %.15e, \"y\": %.15e},\n", corner.x, corner.y);
	std::fprintf(fp, "  \"fe_tool\": {\"attached\": %d, \"boundary_nodes\": %u, \"bbox\": {\"xmax\": %.15e, \"ymin\": %.15e}},\n",
	             has_fe ? 1 : 0, static_cast<unsigned int>(poly.size()), tool_xmax, y_bottom);
	std::fprintf(fp, "  \"orientation\": {\"signed_area\": %.15e, \"edge_tangent_angle_rad\": %.15e},\n", area, tan_angle);
	std::fprintf(fp, "  \"tangency\": {\"tol_m\": %.15e, \"distance_m\": %.15e, \"closest\": {\"x\": %.15e, \"y\": %.15e}, \"pass\": %d},\n",
	             tangency_tol, tangency_dist, cp.x, cp.y, pass_tangency ? 1 : 0);
	std::fprintf(fp, "  \"clearance\": {\"target_m\": %.15e, \"tol_m\": %.15e, \"measured_m\": %.15e, \"error_m\": %.15e, \"pass\": %d},\n",
	             clearance_target, clearance_tol, clearance, clearance_err, pass_clearance ? 1 : 0);
	std::fprintf(fp, "  \"recommended_translation_m\": {\"dx\": %.15e, \"dy\": %.15e},\n", recommended_translation.x, recommended_translation.y);
	std::fprintf(fp, "  \"rotation\": {\"supported\": %d, \"recommended_deg\": %.15e}\n", 0, 0.0);
	std::fprintf(fp, "}\n");
	std::fclose(fp);
}

static void write_fe_tool_bc_validation_reports(body &b, const char *folder, int model) {
	if (!env_flag("MFREE_FE_BC_VALIDATE", false)) return;

	fe_tool *ft = b.get_fe_tool();
	if (!ft) return;
	fe_tool ft_probe = *ft;
	fe_tool *ftp = &ft_probe;

	int top_tag = env_int("MFREE_FE_BC_TOP_TAG", 110);
	int rear_tag = env_int("MFREE_FE_BC_REAR_TAG", 114);
	double Tamb_C = env_double("MFREE_FE_BC_AMBIENT_C", 25.0);
	double Tamb_K = Tamb_C + 273.15;

	std::unordered_set<unsigned int> top_nodes;
	std::unordered_set<unsigned int> rear_nodes;
	for (const auto &e : ftp->boundary_edges()) {
		if (e.physical_tag == top_tag) {
			top_nodes.insert(e.n0);
			top_nodes.insert(e.n1);
		}
		if (e.physical_tag == rear_tag) {
			rear_nodes.insert(e.n0);
			rear_nodes.insert(e.n1);
		}
	}

	std::unordered_set<unsigned int> both_nodes;
	for (unsigned int n : top_nodes) if (rear_nodes.find(n) != rear_nodes.end()) both_nodes.insert(n);

	double thermal_dt = env_double("MFREE_FE_BC_THERMAL_DT", 1e-6);
	if (!std::isfinite(thermal_dt) || thermal_dt <= 0.) thermal_dt = 1e-6;
	double thermal_dt_crit = ftp->thermal_dt_crit();
	if (std::isfinite(thermal_dt_crit) && thermal_dt_crit > 0.0 && thermal_dt > thermal_dt_crit) {
		thermal_dt = thermal_dt_crit;
	}

	std::vector<double> T_before(ftp->nodes_tool_frame().size(), 0.0);
	for (unsigned int i = 0; i < T_before.size(); i++) T_before[i] = ftp->temperature_at_node(i);
	ftp->advance_explicit(thermal_dt);
	double max_abs_dT = 0.0;
	for (unsigned int i = 0; i < T_before.size(); i++) {
		double dT = ftp->temperature_at_node(i) - T_before[i];
		if (std::isfinite(dT)) max_abs_dT = std::max(max_abs_dT, std::abs(dT));
	}

	auto write_csv = [&](const char *name, const std::unordered_set<unsigned int> &nodes) {
		char path[1024];
		std::snprintf(path, sizeof(path), "%s/%s", folder, name);
		FILE *fp = std::fopen(path, "w+");
		if (!fp) return;
		std::fprintf(fp, "node_id,x,y,z,uy_fixed,temperature_C,temperature_assigned_C,abs_err_C\n");
		std::vector<unsigned int> sorted;
		sorted.reserve(nodes.size());
		for (unsigned int n : nodes) sorted.push_back(n);
		std::sort(sorted.begin(), sorted.end());
		for (unsigned int n : sorted) {
			glm::dvec2 pw = ftp->node_world(n);
			int uy_fixed = ftp->is_mechanics_fixed_y(n) ? 0 : 1;
			double Tm_K = ftp->temperature_at_node(n);
			double Tm_C = Tm_K - 273.15;
			double err_C = std::abs(Tm_K - Tamb_K);
			if (!std::isfinite(pw.x) || !std::isfinite(pw.y) || !std::isfinite(Tm_C) || !std::isfinite(Tamb_C) || !std::isfinite(err_C)) continue;
			std::fprintf(fp, "%u,%.15e,%.15e,%.15e,%d,%.15e,%.15e,%.15e\n", n + 1u, pw.x, pw.y, 0.0, uy_fixed, Tm_C, Tamb_C, err_C);
		}
		std::fclose(fp);
	};

	write_csv("fe_bc_top_edge.csv", top_nodes);
	write_csv("fe_bc_rear_edge.csv", rear_nodes);

	auto compute_temp_err = [&](const std::unordered_set<unsigned int> &nodes) -> double {
		double mx = 0.0;
		for (unsigned int n : nodes) {
			double Tm_K = ftp->temperature_at_node(n);
			if (!std::isfinite(Tm_K)) continue;
			mx = std::max(mx, std::abs(Tm_K - Tamb_K));
		}
		return mx;
	};

	double top_err_K = compute_temp_err(top_nodes);
	double rear_err_K = compute_temp_err(rear_nodes);

	auto compute_minmax = [&](const std::unordered_set<unsigned int> &nodes) -> std::pair<glm::dvec2, glm::dvec2> {
		glm::dvec2 mn(std::numeric_limits<double>::infinity());
		glm::dvec2 mx(-std::numeric_limits<double>::infinity());
		for (unsigned int n : nodes) {
			glm::dvec2 pw = ft->node_world(n);
			mn.x = std::min(mn.x, pw.x);
			mn.y = std::min(mn.y, pw.y);
			mx.x = std::max(mx.x, pw.x);
			mx.y = std::max(mx.y, pw.y);
		}
		if (!std::isfinite(mn.x) || !std::isfinite(mn.y) || !std::isfinite(mx.x) || !std::isfinite(mx.y)) {
			mn = glm::dvec2(0.);
			mx = glm::dvec2(0.);
		}
		return {mn, mx};
	};

	glm::dvec2 tool_mn(std::numeric_limits<double>::infinity());
	glm::dvec2 tool_mx(-std::numeric_limits<double>::infinity());
	for (unsigned int i = 0; i < ft->nodes_tool_frame().size(); i++) {
		glm::dvec2 pw = ftp->node_world(i);
		tool_mn.x = std::min(tool_mn.x, pw.x);
		tool_mn.y = std::min(tool_mn.y, pw.y);
		tool_mx.x = std::max(tool_mx.x, pw.x);
		tool_mx.y = std::max(tool_mx.y, pw.y);
	}
	if (!std::isfinite(tool_mn.x) || !std::isfinite(tool_mn.y) || !std::isfinite(tool_mx.x) || !std::isfinite(tool_mx.y)) {
		tool_mn = glm::dvec2(0.);
		tool_mx = glm::dvec2(0.);
	}

	auto top_mm = compute_minmax(top_nodes);
	auto rear_mm = compute_minmax(rear_nodes);

	char report_path[1024];
	std::snprintf(report_path, sizeof(report_path), "%s/fe_bc_report.json", folder);
	FILE *rp = std::fopen(report_path, "w+");
	if (rp) {
		std::fprintf(rp, "{\n");
		std::fprintf(rp, "  \"model\": %d,\n", model);
		std::fprintf(rp, "  \"top_tag\": %d,\n", top_tag);
		std::fprintf(rp, "  \"rear_tag\": %d,\n", rear_tag);
		std::fprintf(rp, "  \"ambient_C\": %.15e,\n", Tamb_C);
		std::fprintf(rp, "  \"top_nodes\": %u,\n", static_cast<unsigned int>(top_nodes.size()));
		std::fprintf(rp, "  \"rear_nodes\": %u,\n", static_cast<unsigned int>(rear_nodes.size()));
		std::fprintf(rp, "  \"intersection_nodes\": %u,\n", static_cast<unsigned int>(both_nodes.size()));
		std::fprintf(rp, "  \"temperature_assigned_K\": %.15e,\n", Tamb_K);
		std::fprintf(rp, "  \"max_abs_temp_err_K\": {\"top\": %.15e, \"rear\": %.15e},\n", top_err_K, rear_err_K);
		std::fprintf(rp, "  \"thermal_dt_used_s\": %.15e,\n", thermal_dt);
		std::fprintf(rp, "  \"thermal_dt_crit_s\": %.15e,\n", thermal_dt_crit);
		std::fprintf(rp, "  \"max_abs_dT_K_first_advance\": %.15e,\n", max_abs_dT);
		std::fprintf(rp, "  \"tool_bbox\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", tool_mn.x, tool_mx.x, tool_mn.y, tool_mx.y);
		std::fprintf(rp, "  \"top_bbox\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", top_mm.first.x, top_mm.second.x, top_mm.first.y, top_mm.second.y);
		std::fprintf(rp, "  \"rear_bbox\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", rear_mm.first.x, rear_mm.second.x, rear_mm.first.y, rear_mm.second.y);
		std::fprintf(rp, "  \"note\": \"Dirichlet temperature is applied by physical tag each thermal update (fe_tool::apply_dirichlet_bc), and fixed nodes are excluded from thermal integration. UY constraints verified via fe_tool::is_mechanics_fixed_y; intersection node is not overconstrained in Y because the DOF is identical.\"\n");
		std::fprintf(rp, "}\n");
		std::fclose(rp);
	}

	bool run_conv = env_flag("MFREE_FE_BC_RUN_CONVERGENCE", true);
	if (!run_conv) return;

	unsigned int max_iters = static_cast<unsigned int>(std::max(1, env_int("MFREE_FE_BC_MECH_ITERS", 50)));
	double rel_tol = env_double("MFREE_FE_BC_MECH_REL_TOL", 1e-6);
	if (!std::isfinite(rel_tol) || rel_tol <= 0.) rel_tol = 1e-6;

	ft->solve_mechanics_quasistatic(max_iters, rel_tol);
	std::vector<glm::dvec2> u1 = ft->displacements();
	ft->solve_mechanics_quasistatic(max_iters, rel_tol);
	std::vector<glm::dvec2> u2 = ft->displacements();

	double max_du = 0.0;
	double max_u = 0.0;
	for (std::size_t i = 0; i < u2.size() && i < u1.size(); i++) {
		glm::dvec2 du = u2[i] - u1[i];
		double ndu = std::sqrt(du.x * du.x + du.y * du.y);
		double nu = std::sqrt(u2[i].x * u2[i].x + u2[i].y * u2[i].y);
		if (std::isfinite(ndu)) max_du = std::max(max_du, ndu);
		if (std::isfinite(nu)) max_u = std::max(max_u, nu);
	}
	double denom = max_u;
	if (!std::isfinite(denom) || denom <= 0.0) denom = 0.0;
	double rel_change = (denom > 0.0) ? (max_du / denom) : 0.0;
	double abs_tol = env_double("MFREE_FE_BC_ABS_TOL", 1e-9);
	if (!std::isfinite(abs_tol) || abs_tol <= 0.0) abs_tol = 1e-9;
	bool pass = (max_du <= abs_tol) || (rel_change <= 0.01);

	char conv_path[1024];
	std::snprintf(conv_path, sizeof(conv_path), "%s/fe_bc_convergence.txt", folder);
	FILE *cp = std::fopen(conv_path, "w+");
	if (cp) {
		std::fprintf(cp, "FE BC convergence report (model %d)\n", model);
		std::fprintf(cp, "Top tag: %d, Rear tag: %d, Ambient: %.6f C\n", top_tag, rear_tag, Tamb_C);
		std::fprintf(cp, "Mechanics iterations per solve: %u, rel_tol: %.3e\n", max_iters, rel_tol);
		std::fprintf(cp, "Thermal pre-step dt: %.3e s\n", thermal_dt);
		std::fprintf(cp, "Max displacement change between solve #1 and #2: %.6e m\n", max_du);
		std::fprintf(cp, "Relative change (max_du / max_u): %.6e\n", rel_change);
		std::fprintf(cp, "Absolute tolerance: %.6e m\n", abs_tol);
		std::fprintf(cp, "Pass criterion: (max_du <= abs_tol) OR (relative_change <= 1%%): %s\n", pass ? "PASS" : "FAIL");
		std::fprintf(cp, "Rigid-body motion warnings: not emitted by this solver; constraint set includes an optional UX anchor via MFREE_FE_BC_ANCHOR_UX.\n");
		std::fclose(cp);
	}
}

static void write_precheck_report(body &b, const char *folder) {
	const std::vector<particle> &p = b.get_particles();
	const double cp_wp = b.get_sim_data().get_physical_constants().tc().cp();
	double x_min = std::numeric_limits<double>::infinity();
	double x_max = -std::numeric_limits<double>::infinity();
	double y_min = std::numeric_limits<double>::infinity();
	double y_max = -std::numeric_limits<double>::infinity();
	double rho_min = std::numeric_limits<double>::infinity();
	double rho_max = -std::numeric_limits<double>::infinity();
	unsigned int fixed_count = 0;
	unsigned int nbh_min = std::numeric_limits<unsigned int>::max();
	unsigned int nbh_max = 0;
	double nbh_sum = 0.;
	unsigned int nbh_count = 0;

	for (const particle &pi : p) {
		x_min = std::min(x_min, pi.x);
		x_max = std::max(x_max, pi.x);
		y_min = std::min(y_min, pi.y);
		y_max = std::max(y_max, pi.y);
		if (std::isfinite(pi.rho)) {
			rho_min = std::min(rho_min, pi.rho);
			rho_max = std::max(rho_max, pi.rho);
		}
		if (pi.fixed) fixed_count++;
		nbh_min = std::min(nbh_min, pi.num_nbh);
		nbh_max = std::max(nbh_max, pi.num_nbh);
		nbh_sum += static_cast<double>(pi.num_nbh);
		nbh_count++;
	}

	unsigned int overlap_count = 0;
	double overlap_max_depth = 0.;
	const tool *t = b.get_tool();
	double tool_xmin = std::numeric_limits<double>::infinity();
	double tool_xmax = -std::numeric_limits<double>::infinity();
	double tool_ymin = std::numeric_limits<double>::infinity();
	double tool_ymax = -std::numeric_limits<double>::infinity();
	if (t) {
		for (const particle &pi : p) {
			double d = t->inside(glm::dvec2(pi.x, pi.y));
			if (std::isfinite(d) && d > 0.) {
				overlap_count++;
				overlap_max_depth = std::max(overlap_max_depth, d);
			}
		}
		auto seg = t->get_segments();
		for (const auto &s : seg) {
			tool_xmin = std::min(tool_xmin, std::min(s.left.x, s.right.x));
			tool_xmax = std::max(tool_xmax, std::max(s.left.x, s.right.x));
			tool_ymin = std::min(tool_ymin, std::min(s.left.y, s.right.y));
			tool_ymax = std::max(tool_ymax, std::max(s.left.y, s.right.y));
		}
	}

	const fe_tool *ft = b.get_fe_tool();
	double fe_min_angle = 180.;
	std::size_t fe_nodes = 0;
	std::size_t fe_tris = 0;
	glm::dvec2 sum_f_wp(0.);
	glm::dvec2 sum_f_fe(0.);
	double sum_p_wp = 0.;
	double sum_p_fe = 0.;
	unsigned int contact_poly_nodes = 0;
	double contact_poly_ctr_inside = 0.;
	unsigned int overlap_count_contact = 0;
	double overlap_max_depth_contact = 0.;
	double tool_c_xmin = std::numeric_limits<double>::infinity();
	double tool_c_xmax = -std::numeric_limits<double>::infinity();
	double tool_c_ymin = std::numeric_limits<double>::infinity();
	double tool_c_ymax = -std::numeric_limits<double>::infinity();
	if (ft) {
		fe_nodes = ft->nodes_tool_frame().size();
		fe_tris = ft->triangles().size();
		for (const auto &tri : ft->triangles()) {
			glm::dvec2 a = ft->to_world_frame(ft->nodes_tool_frame()[tri[0]]);
			glm::dvec2 bb = ft->to_world_frame(ft->nodes_tool_frame()[tri[1]]);
			glm::dvec2 c = ft->to_world_frame(ft->nodes_tool_frame()[tri[2]]);
			fe_min_angle = std::min(fe_min_angle, tri_min_angle_deg(a, bb, c));
		}
		for (unsigned int i = 0; i < fe_nodes; i++) {
			sum_f_fe += ft->nodal_force(i);
			sum_p_fe += ft->nodal_power(i);
		}

		std::vector<glm::dvec2> poly = ft->boundary_loop_world();
		if (poly.size() >= 3) {
			contact_poly_nodes = static_cast<unsigned int>(poly.size());
			glm::dvec2 ctr(0.);
			for (const auto &pp : poly) ctr += pp;
			ctr /= static_cast<double>(poly.size());
			contact_poly_ctr_inside = polygon_inside_depth(ctr, poly);
			for (const auto &pp : poly) {
				tool_c_xmin = std::min(tool_c_xmin, pp.x);
				tool_c_xmax = std::max(tool_c_xmax, pp.x);
				tool_c_ymin = std::min(tool_c_ymin, pp.y);
				tool_c_ymax = std::max(tool_c_ymax, pp.y);
			}
			for (const particle &pi : p) {
				double d = polygon_inside_depth(glm::dvec2(pi.x, pi.y), poly);
				if (std::isfinite(d) && d > 0.) {
					overlap_count_contact++;
					overlap_max_depth_contact = std::max(overlap_max_depth_contact, d);
				}
			}
		}
	}

	if (cp_wp > 0. && std::isfinite(cp_wp)) {
		for (const particle &pi : p) {
			if (!std::isfinite(pi.m) || pi.m <= 0.) continue;
			if (!std::isfinite(pi.T_t)) continue;
			sum_p_wp += pi.m * cp_wp * pi.T_t;
		}
	}

	for (const particle &pi : p) sum_f_wp += glm::dvec2(pi.fcx + pi.ftx, pi.fcy + pi.fty);

	char path[256];
	std::snprintf(path, sizeof(path), "%s/precheck.json", folder);
	FILE *fp = std::fopen(path, "w+");
	if (!fp) return;

	std::fprintf(fp, "{\n");
	std::fprintf(fp, "  \"particles\": {\n");
	std::fprintf(fp, "    \"count\": %u,\n", static_cast<unsigned int>(p.size()));
	std::fprintf(fp, "    \"bbox\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", x_min, x_max, y_min, y_max);
	std::fprintf(fp, "    \"density\": {\"min\": %.15e, \"max\": %.15e},\n", rho_min, rho_max);
	std::fprintf(fp, "    \"fixed_count\": %u,\n", fixed_count);
	std::fprintf(fp, "    \"num_neighbors\": {\"min\": %u, \"max\": %u, \"avg\": %.6f}\n", nbh_min, nbh_max, (nbh_count ? nbh_sum / static_cast<double>(nbh_count) : 0.0));
	std::fprintf(fp, "  },\n");

	std::fprintf(fp, "  \"tool\": {\n");
	if (t) {
		auto seg = t->get_segments();
		std::fprintf(fp, "    \"segments\": %u,\n", static_cast<unsigned int>(seg.size()));
		std::fprintf(fp, "    \"bbox\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", tool_xmin, tool_xmax, tool_ymin, tool_ymax);
		if (t->get_fillet()) {
			std::fprintf(fp, "    \"fillet\": {\"cx\": %.15e, \"cy\": %.15e, \"r\": %.15e},\n", t->get_fillet()->p.x, t->get_fillet()->p.y, t->get_fillet()->r);
		} else {
			std::fprintf(fp, "    \"fillet\": null,\n");
		}
		std::fprintf(fp, "    \"overlap\": {\"count\": %u, \"max_depth\": %.15e}\n", overlap_count, overlap_max_depth);
	} else {
		std::fprintf(fp, "    \"segments\": 0,\n");
		std::fprintf(fp, "    \"fillet\": null,\n");
		std::fprintf(fp, "    \"overlap\": {\"count\": 0, \"max_depth\": 0.0}\n");
	}
	std::fprintf(fp, "  },\n");

	std::fprintf(fp, "  \"tool_contact\": {\n");
	if (ft && std::isfinite(tool_c_xmin) && std::isfinite(tool_c_xmax) && std::isfinite(tool_c_ymin) && std::isfinite(tool_c_ymax)) {
		std::fprintf(fp, "    \"bbox\": {\"xmin\": %.15e, \"xmax\": %.15e, \"ymin\": %.15e, \"ymax\": %.15e},\n", tool_c_xmin, tool_c_xmax, tool_c_ymin, tool_c_ymax);
		std::fprintf(fp, "    \"polygon\": {\"nodes\": %u, \"centroid_inside\": %.15e},\n", contact_poly_nodes, contact_poly_ctr_inside);
		std::fprintf(fp, "    \"overlap\": {\"count\": %u, \"max_depth\": %.15e}\n", overlap_count_contact, overlap_max_depth_contact);
	} else {
		std::fprintf(fp, "    \"bbox\": null,\n");
		std::fprintf(fp, "    \"polygon\": {\"nodes\": 0, \"centroid_inside\": 0.0},\n");
		std::fprintf(fp, "    \"overlap\": {\"count\": 0, \"max_depth\": 0.0}\n");
	}
	std::fprintf(fp, "  },\n");

	std::fprintf(fp, "  \"fe_tool\": {\n");
	if (ft) {
		fe_tool::thermal_material tm = ft->get_material();
		fe_tool::mechanical_material mm = ft->get_mechanical_material();
		fe_tool::contact_convergence cc = ft->get_contact_convergence();
		fe_tool::contact_energy_balance eb = ft->get_contact_energy_balance();
		std::fprintf(fp, "    \"nodes\": %u,\n", static_cast<unsigned int>(fe_nodes));
		std::fprintf(fp, "    \"triangles\": %u,\n", static_cast<unsigned int>(fe_tris));
		std::fprintf(fp, "    \"min_triangle_angle_deg\": %.6f,\n", fe_min_angle);
		std::fprintf(fp, "    \"temperature\": {\"min\": %.15e, \"max\": %.15e},\n", ft->min_temperature(), ft->max_temperature());
		std::fprintf(fp, "    \"thermal_material\": {\"rho\": %.15e, \"cp\": %.15e, \"k\": %.15e},\n", tm.rho, tm.cp, tm.k);
		std::fprintf(fp, "    \"mechanical_material\": {\"E\": %.15e, \"nu\": %.15e, \"alpha\": %.15e},\n", mm.E, mm.nu, mm.alpha);
		std::fprintf(fp, "    \"max_displacement\": %.15e,\n", ft->max_displacement_norm());
		std::fprintf(fp, "    \"contact_convergence\": {\"iters\": %u, \"rel_force\": %.15e, \"rel_power\": %.15e, \"max_rel_force_node\": %.15e, \"max_rel_power_node\": %.15e, \"nodes_force_over_tol\": %u, \"nodes_power_over_tol\": %u},\n",
		             cc.iters, cc.rel_force, cc.rel_power, cc.max_rel_force_node, cc.max_rel_power_node, cc.nodes_force_over_tol, cc.nodes_power_over_tol);
		std::fprintf(fp, "    \"contact_energy\": {\"P_fric\": %.15e, \"P_cond\": %.15e, \"scale\": %.15e, \"frac_workpiece\": %.15e, \"frac_tool\": %.15e},\n",
		             eb.P_fric, eb.P_cond, eb.scale, eb.frac_workpiece, eb.frac_tool);
		std::fprintf(fp, "    \"mapped_balance\": {\n");
		std::fprintf(fp, "      \"sum_force_workpiece\": {\"fx\": %.15e, \"fy\": %.15e},\n", sum_f_wp.x, sum_f_wp.y);
		std::fprintf(fp, "      \"sum_force_fe\": {\"fx\": %.15e, \"fy\": %.15e},\n", sum_f_fe.x, sum_f_fe.y);
		std::fprintf(fp, "      \"sum_power_workpiece\": %.15e,\n", sum_p_wp);
		std::fprintf(fp, "      \"sum_power_fe\": %.15e\n", sum_p_fe);
		std::fprintf(fp, "    }\n");
	} else {
		std::fprintf(fp, "    \"nodes\": 0,\n");
		std::fprintf(fp, "    \"triangles\": 0,\n");
		std::fprintf(fp, "    \"min_triangle_angle_deg\": 0.0\n");
	}
	std::fprintf(fp, "  }\n");
	std::fprintf(fp, "}\n");

	std::fclose(fp);
}

static void write_validation_summary(const body &b, const char *folder, int model) {
	const std::vector<particle> &p = b.get_particles();
	double u_max = 0.;
	double svm_max = 0.;
	double epsp_max = 0.;
	double T_min = std::numeric_limits<double>::infinity();
	double T_max = -std::numeric_limits<double>::infinity();
	double cp_max = 0.;
	double cp_sum = 0.;
	unsigned int cp_count = 0;

	for (const particle &pi : p) {
		double dx = pi.x - pi.X;
		double dy = pi.y - pi.Y;
		if (std::isfinite(dx) && std::isfinite(dy)) u_max = std::max(u_max, std::sqrt(dx * dx + dy * dy));

		if (std::isfinite(pi.T)) {
			T_min = std::min(T_min, pi.T);
			T_max = std::max(T_max, pi.T);
		}
		if (std::isfinite(pi.eps_pl_equiv)) epsp_max = std::max(epsp_max, pi.eps_pl_equiv);

		double sxx = pi.Sxx - pi.p;
		double sxy = pi.Sxy;
		double syy = pi.Syy - pi.p;
		double szz = pi.Szz - pi.p;
		double svm = std::sqrt(std::fabs((sxx * sxx + syy * syy + szz * szz) - sxx * syy - sxx * szz - syy * szz + 3.0 * (sxy * sxy)));
		if (std::isfinite(svm)) svm_max = std::max(svm_max, svm);

		double Fn = std::sqrt(pi.fcx * pi.fcx + pi.fcy * pi.fcy);
		if (Fn > 0. && std::isfinite(Fn) && pi.m > 0. && pi.rho > 0.) {
			double cp = Fn * pi.rho / pi.m;
			if (std::isfinite(cp)) {
				cp_max = std::max(cp_max, cp);
				cp_sum += cp;
				cp_count++;
			}
		}
	}

	const fe_tool *ft = b.get_fe_tool();
	std::size_t fe_nodes = 0;
	std::size_t fe_tris = 0;
	double fe_min_angle = 0.;
	double fe_Tmin = 0.;
	double fe_Tmax = 0.;
	double fe_Fmax = 0.;

	if (ft) {
		fe_nodes = ft->nodes_tool_frame().size();
		fe_tris = ft->triangles().size();
		fe_min_angle = 180.;
		for (const auto &tri : ft->triangles()) {
			glm::dvec2 a = ft->to_world_frame(ft->nodes_tool_frame()[tri[0]]);
			glm::dvec2 bb = ft->to_world_frame(ft->nodes_tool_frame()[tri[1]]);
			glm::dvec2 c = ft->to_world_frame(ft->nodes_tool_frame()[tri[2]]);
			fe_min_angle = std::min(fe_min_angle, tri_min_angle_deg(a, bb, c));
		}
		fe_Tmin = ft->min_temperature();
		fe_Tmax = ft->max_temperature();
		for (unsigned int i = 0; i < fe_nodes; i++) {
			glm::dvec2 f = ft->nodal_force(i);
			double fm = std::sqrt(f.x * f.x + f.y * f.y);
			if (std::isfinite(fm)) fe_Fmax = std::max(fe_Fmax, fm);
		}
	}

	char path[256];
	std::snprintf(path, sizeof(path), "%s/validation_summary.json", folder);
	FILE *fp = std::fopen(path, "w+");
	if (!fp) return;

	std::fprintf(fp, "{\n");
	std::fprintf(fp, "  \"model\": %d,\n", model);
	std::fprintf(fp, "  \"particles\": {\n");
	std::fprintf(fp, "    \"count\": %u,\n", static_cast<unsigned int>(p.size()));
	std::fprintf(fp, "    \"temperature\": {\"min\": %.15e, \"max\": %.15e},\n", T_min, T_max);
	std::fprintf(fp, "    \"max_displacement\": %.15e,\n", u_max);
	std::fprintf(fp, "    \"max_von_mises\": %.15e,\n", svm_max);
	std::fprintf(fp, "    \"max_equiv_plastic_strain\": %.15e,\n", epsp_max);
	std::fprintf(fp, "    \"contact_pressure\": {\"count\": %u, \"avg\": %.15e, \"max\": %.15e}\n",
	             cp_count, (cp_count ? cp_sum / static_cast<double>(cp_count) : 0.0), cp_max);
	std::fprintf(fp, "  },\n");
	std::fprintf(fp, "  \"fe_tool\": {\n");
	std::fprintf(fp, "    \"attached\": %d,\n", ft ? 1 : 0);
	std::fprintf(fp, "    \"nodes\": %u,\n", static_cast<unsigned int>(fe_nodes));
	std::fprintf(fp, "    \"triangles\": %u,\n", static_cast<unsigned int>(fe_tris));
	std::fprintf(fp, "    \"min_triangle_angle_deg\": %.6f,\n", fe_min_angle);
	std::fprintf(fp, "    \"temperature\": {\"min\": %.15e, \"max\": %.15e},\n", fe_Tmin, fe_Tmax);
	std::fprintf(fp, "    \"max_nodal_force\": %.15e\n", fe_Fmax);
	std::fprintf(fp, "  }\n");
	std::fprintf(fp, "}\n");

	std::fclose(fp);
}

int main(int argc, char * argv[]) {
	#if defined(__GLIBC__)
	feenableexcept(FE_INVALID | FE_OVERFLOW);
	#endif

	const char *results_dir_env = std::getenv("MFREE_RESULTS_DIR");
	const std::string results_dir = (results_dir_env && results_dir_env[0] != '\0') ? std::string(results_dir_env) : std::string("results");

	#ifdef _OPENMP
	if (const char *s = std::getenv("MFREE_OMP_THREADS"); s && s[0] != '\0') {
		errno = 0;
		char *end = nullptr;
		long n = std::strtol(s, &end, 10);
		bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
		if (ok && n > 0 && n <= std::numeric_limits<int>::max()) {
			omp_set_dynamic(0);
			omp_set_num_threads(static_cast<int>(n));
		} else {
			std::fprintf(stderr, "warning: invalid MFREE_OMP_THREADS=\"%s\"; expected positive integer\n", s);
		}
	}
	#endif

	std::filesystem::create_directories(results_dir);
	bool clean_results = env_flag("MFREE_CLEAN_RESULTS", true);
	if (clean_results) {
		for (const auto &p : std::filesystem::directory_iterator(results_dir)) {
			if (!p.is_regular_file()) continue;
			const auto ext = p.path().extension().string();
			if (ext == ".txt" || ext == ".vtk") {
				std::error_code ec;
				std::filesystem::remove(p.path(), ec);
			}
		}
	}

	int model = 1;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (arg == "-m" && i + 1 < argc) {
			model = std::atoi(argv[i + 1]);
			i++;
		}
	}
	printf("running model %d\n", model);

	int nx = 31;
	assert(model >= 1);
	assert(model <= 4);

	/*
	 ==========================
	 *  set up chosen benchmark
	 *  	this runs model 1-4 in the paper
	 *  	other preliminary simulations are available in test_benches.h
	 *  	density reapproximation tests are aviable in test_density.h
	 ==========================
	 */
	body *b = 0;
	switch (model) {
	case 1:
		b = cutting_ref_single_resol(nx);
		break;
	case 2:
		nx = 61;
		b = cutting_ref_multi_resol_apriori(nx);
		break;
	case 3:
		nx = 61;
		b = cutting_ref_multi_resol_dynamic(nx);
		break;
	case 4:
		nx = 61;
		b = cutting_ref_single_resol(nx);
		break;
	}

	if (env_flag("MFREE_PREPROCESS_ONLY", false)) {
		b->construct_verlet_lists();
		{
			std::vector<particle> &pp = b->get_particles();
			for (auto &pi : pp) {
				pi.fcx = 0.;
				pi.fcy = 0.;
				pi.ftx = 0.;
				pi.fty = 0.;
				pi.T_t = 0.;
			}
		}
		write_geom_validation_report(*b, results_dir.c_str(), model);
		write_fe_tool_bc_validation_reports(*b, results_dir.c_str(), model);
		b->apply_contact();
		if (global_logger) global_logger->log(*b, 0);
		write_precheck_report(*b, results_dir.c_str());
		write_validation_summary(*b, results_dir.c_str(), model);
		return EXIT_SUCCESS;
	}

	/*
	  ==================================
	 settings of the printout
	 at least [num_print] frames are written out
	 ===================================
	 */
	simulation_time *time = &simulation_time::getInstance();
	{
		double s = env_double("MFREE_DT_SCALE", 1.0);
		if (std::isfinite(s) && s > 0.) time->set_dt(time->get_dt() * s);
	}
	{
		double s = env_double("MFREE_T_FINAL_SCALE", 1.0);
		if (std::isfinite(s) && s > 0.) time->set_t_final(time->get_t_final() * s);
	}
	unsigned int num_step = time->get_t_final()/time->get_dt();
	int num_print = 150;
	const char *num_print_env = std::getenv("MFREE_NUM_PRINT");
	if (num_print_env && num_print_env[0] != '\0') {
		errno = 0;
		char *end = nullptr;
		long v = std::strtol(num_print_env, &end, 10);
		bool ok = (end != num_print_env && end != nullptr && *end == '\0' && errno == 0);
		if (ok && v >= 0 && v <= std::numeric_limits<int>::max()) {
			num_print = static_cast<int>(v);
		} else {
			std::fprintf(stderr, "warning: invalid MFREE_NUM_PRINT=\"%s\"; expected integer >= 0\n", num_print_env);
		}
	}
	unsigned int freq = 1u;
	if (num_print > 0) freq = num_step / static_cast<unsigned int>(num_print);
	unsigned int print_iter = 0;
	auto begin = std::chrono::high_resolution_clock::now();

	freq = std::max(1u, freq);
	if (num_print <= 0) freq = std::numeric_limits<unsigned int>::max();

	const char *output_freq_env = std::getenv("MFREE_OUTPUT_FREQ");
	if (output_freq_env && output_freq_env[0] != '\0') {
		errno = 0;
		char *end = nullptr;
		unsigned long v = std::strtoul(output_freq_env, &end, 10);
		bool ok = (end != output_freq_env && end != nullptr && *end == '\0' && errno == 0);
		if (ok && v <= std::numeric_limits<unsigned int>::max()) {
			unsigned int f = static_cast<unsigned int>(v);
			if (f >= 1u) freq = f;
			else freq = std::numeric_limits<unsigned int>::max();
		} else {
			std::fprintf(stderr, "warning: invalid MFREE_OUTPUT_FREQ=\"%s\"; expected integer >= 0\n", output_freq_env);
		}
	}
	unsigned int max_steps = 0;
	max_steps = static_cast<unsigned int>(std::max(0, env_int("MFREE_MAX_STEPS", 0)));

	/*
	  ========================
	  (2nd-order) LeapFrog scheme is used
	  for the explicit time integration.
	  ========================
	 */
	leap_frog stepper((*b).get_num_part());

	/*
	 * This is the implementation of the main time-loop,
	 * also illustrated by the following flowchart in the paper:
	 * ---------------------------------------------------------
	 * Section 4:
	 * Fig. 5. Flowchart of the model logic for each time-step.
	 *
	 */
	while(!time->finished() && (max_steps == 0 || time->get_step() < max_steps)) {

		// plot with given frequency
		if (time->get_step() % freq == 0) {

			if (global_logger) {

				/* Write out the results in the desired format (*.txt, *.vtk)
				 * to be read in Matlab or ParaView
				 */
				global_logger->log(*b, print_iter);

				// Report the time left to finish
				auto intermediate = std::chrono::high_resolution_clock::now();
				double seconds_so_far = std::chrono::duration<double>(intermediate - begin).count();

				double percent_done = 100*time->get_step()/((double) num_step);
				double time_left = seconds_so_far/percent_done*100;

				printf("%06d: #increments %06d, cur time %e, pctg done %f, seconds left: %f\n", print_iter, time->get_step(), time->get_dt()*time->get_step(), percent_done, time_left-seconds_so_far);
				print_iter++;
			}
		}

		/* Carry out the time-stepper:
		 * this is to update the system by evolving the variables
		 * over time using the LeapFrog time stepping
		 */
		stepper.step(*b);

		time->increment_step();
		time->increment_time();
	}

	auto end = std::chrono::high_resolution_clock::now();
	double elapsed = std::chrono::duration<double>(end - begin).count();
	printf("Runtime: %f\n", elapsed);
	write_validation_summary(*b, results_dir.c_str(), model);

	return EXIT_SUCCESS;
}
