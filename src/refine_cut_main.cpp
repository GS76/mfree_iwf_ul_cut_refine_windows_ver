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

#include "particle.h"
#include "contact.h"
#include "vtk_writer.h"

#include "benchmarks/test_density.h"
#include "benchmarks/test_benches.h"
#include "benchmarks/test_cuttings.h"

#include "tool.h"
#include "logger.h"
#include "body.h"

logger *global_logger;

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
	bool clean_results = true;
	if (const char *s = std::getenv("MFREE_CLEAN_RESULTS"); s && std::atoi(s) == 0) clean_results = false;
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

	const char *preprocess_only = std::getenv("MFREE_PREPROCESS_ONLY");
	if (preprocess_only && std::atoi(preprocess_only) != 0) {
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
	const char *dt_scale_env = std::getenv("MFREE_DT_SCALE");
	if (dt_scale_env) {
		double s = std::atof(dt_scale_env);
		if (std::isfinite(s) && s > 0.) time->set_dt(time->get_dt() * s);
	}
	const char *t_final_scale_env = std::getenv("MFREE_T_FINAL_SCALE");
	if (t_final_scale_env) {
		double s = std::atof(t_final_scale_env);
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
	const char *max_steps_env = std::getenv("MFREE_MAX_STEPS");
	if (max_steps_env) max_steps = static_cast<unsigned int>(std::atoi(max_steps_env));

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
