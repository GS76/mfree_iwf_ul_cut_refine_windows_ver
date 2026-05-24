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

#include "test_cuttings.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <cerrno>

static bool try_read_env_double(const char *key, double &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0')
		return false;
	char *end = nullptr;
	double v = strtod(s, &end);
	if (end == s || !std::isfinite(v))
		return false;
	out = v;
	return true;
}

static bool try_read_env_int(const char *key, int &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0')
		return false;
	while (*s && std::isspace(static_cast<unsigned char>(*s)))
		++s;
	if (!*s)
		return false;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s)
		return false;
	while (*end && std::isspace(static_cast<unsigned char>(*end)))
		++end;
	if (*end != '\0')
		return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max())
		return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_read_env_table(const char *key, std::vector<double> &T_out, std::vector<double> &v_out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0')
		return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r')
			++p;
		if (!*p)
			break;

		char *end = nullptr;
		double T = strtod(p, &end);
		if (end == p || !std::isfinite(T))
			return false;
		p = end;

		while (*p == ' ' || *p == '\t')
			++p;
		if (*p != ':' && *p != '=')
			return false;
		++p;
		while (*p == ' ' || *p == '\t')
			++p;

		end = nullptr;
		double v = strtod(p, &end);
		if (end == p || !std::isfinite(v))
			return false;
		p = end;

		pairs.push_back({T, v});
	}

	if (pairs.size() < 2)
		return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

	T_out.clear();
	v_out.clear();
	for (const auto &kv : pairs) {
		if (!T_out.empty() && kv.first == T_out.back()) {
			v_out.back() = kv.second;
			continue;
		}
		T_out.push_back(kv.first);
		v_out.push_back(kv.second);
	}
	return T_out.size() >= 2;
}

static void adjust_workpiece_y_bounds_for_feed(double base_lo_y, double base_hi_y, unsigned int base_ny, double target_feed,
											   unsigned int safety_layers, double &lo_y, double &hi_y, unsigned int &ny, double &dy) {
	hi_y = base_hi_y;
	double base_thickness = base_hi_y - base_lo_y;
	dy = base_thickness / (base_ny - 1);
	double required_thickness = target_feed + safety_layers * dy;

	unsigned int extra_layers = 0;
	if (required_thickness > base_thickness) {
		double raw = (required_thickness - base_thickness) / dy;
		extra_layers = (unsigned int)std::ceil(raw - 1e-12);
	}
	ny = base_ny + extra_layers;
	lo_y = hi_y - (ny - 1) * dy;
}

static double read_coupled_motion_ratio() {
	const char *enable_env = getenv("MFREE_COUPLED_MOTION");
	bool enabled = (enable_env && atoi(enable_env) != 0);
	if (!enabled)
		return 1.0;

	double ratio = 1.0;
	if (try_read_env_double("MFREE_COUPLED_MOTION_RATIO", ratio)) {
		if (!std::isfinite(ratio))
			ratio = 1.0;
		ratio = std::max(0.0, std::min(1.0, ratio));
		return ratio;
	}

	const char *primary = getenv("MFREE_PRIMARY_MOVING_BODY");
	if (!primary || primary[0] == '\0')
		return 1.0;
	std::string s(primary);
	for (char &c : s)
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s == "workpiece" || s == "wp")
		return 0.0;
	if (s == "both" || s == "coupled")
		return 0.5;
	return 1.0;
}

static double estimate_dt_for_cutting(const physical_constants &pc, double dx, double hdx, double relative_speed, double empirical_cap,
									  const fe_tool *ft) {
	coupled_timestep_config cfg;
	cfg.particle_spacing = dx;
	cfg.smoothing_length_ratio = hdx;
	cfg.max_relative_speed = relative_speed;
	cfg.empirical_dt_cap = empirical_cap;
	double plane_strain_thickness = 1.0;
	try_read_env_double("MFREE_PLANE_STRAIN_THICKNESS", plane_strain_thickness);
	if (!std::isfinite(plane_strain_thickness) || plane_strain_thickness <= 0.)
		plane_strain_thickness = 1.0;
	double contact_length_factor = 1.0;
	try_read_env_double("MFREE_THERMAL_CONTACT_LENGTH_FACTOR", contact_length_factor);
	if (!std::isfinite(contact_length_factor) || contact_length_factor <= 0.)
		contact_length_factor = 1.0;
	cfg.interface_contact_area = dx * contact_length_factor * plane_strain_thickness;
	try_read_env_double("MFREE_TIMESTEP_WP_MECH_SAFETY", cfg.workpiece_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_WP_THERM_SAFETY", cfg.workpiece_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_MECH_SAFETY", cfg.tool_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_THERM_SAFETY", cfg.tool_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_SAFETY", cfg.interface_thermal_safety);
	try_read_env_double("MFREE_THERMAL_H_FULL", cfg.contact_conductance_full);
	double area_factor = 1.0;
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_AREA_FACTOR", area_factor);
	if (std::isfinite(area_factor) && area_factor > 0.)
		cfg.interface_contact_area *= area_factor;

	coupled_timestep_limits limits = estimate_coupled_timestep(pc, cfg, ft);
	int print_limits = 1;
	try_read_env_int("MFREE_TIMESTEP_PRINT", print_limits);
	if (print_limits != 0)
		print_coupled_timestep_limits(limits);
	return (std::isfinite(limits.maximum_dt) && limits.maximum_dt > 0.) ? limits.maximum_dt : empirical_cap;
}

static void apply_mech_fix_tags_from_env(fe_tool &ft) {
	auto apply_tag_list = [&](const char *env_key, auto apply_tag) -> bool {
		const char *tags = getenv(env_key);
		if (!tags || tags[0] == '\0')
			return false;
		std::string s(tags);
		std::size_t i = 0;
		bool any = false;
		while (i < s.size()) {
			while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ','))
				i++;
			if (i >= s.size())
				break;
			std::size_t j = i;
			while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t')
				j++;
			std::string tok = s.substr(i, j - i);
			errno = 0;
			char *end = nullptr;
			long v = std::strtol(tok.c_str(), &end, 10);
			if (end != tok.c_str() && end != nullptr && *end == '\0' && errno == 0) {
				if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max() && v != 0) {
					apply_tag(static_cast<int>(v));
					any = true;
				}
			}
			i = j;
		}
		return any;
	};

	bool any = false;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_Y_TAGS", [&](int tag) { ft.set_mechanics_fixed_y_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_X_TAGS", [&](int tag) { ft.set_mechanics_fixed_x_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_TAGS", [&](int tag) { ft.set_mechanics_fixed_on_physical(tag); }) || any;
	if (any) {
		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_UX", v))
				anchor_ux = (v != 0);
		}
		int anchor_tag = 0;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_TAG", v))
				anchor_tag = v;
		}
		if (anchor_ux && anchor_tag != 0) {
			std::unordered_set<unsigned int> nodes;
			for (const auto &e : ft.boundary_edges()) {
				if (e.physical_tag != anchor_tag)
					continue;
				nodes.insert(e.n0);
				nodes.insert(e.n1);
			}
			if (!nodes.empty()) {
				unsigned int anchor = *nodes.begin();
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : nodes) {
					glm::dvec2 pw = ft.node_world(n);
					if (!std::isfinite(pw.x))
						continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft.set_mechanics_fixed_x_nodes({anchor});
			}
		}
		return;
	}

	const char *tags = getenv("MFREE_FE_TOOL_FIX_TAGS");
	if (!tags || tags[0] == '\0') {
		std::unordered_set<unsigned int> bnodes;
		for (const auto &e : ft.boundary_edges()) {
			bnodes.insert(e.n0);
			bnodes.insert(e.n1);
		}
		if (bnodes.empty())
			return;

		double x_max = -std::numeric_limits<double>::infinity();
		double x_min = std::numeric_limits<double>::infinity();
		for (unsigned int i : bnodes)
			x_max = std::max(x_max, ft.nodes_tool_frame()[i].x);
		for (unsigned int i : bnodes)
			x_min = std::min(x_min, ft.nodes_tool_frame()[i].x);

		std::vector<unsigned int> fixed;
		double width = x_max - x_min;
		double tol = 0.01 * width;
		try_read_env_double("MFREE_FE_TOOL_FIX_X_TOL", tol);
		if (!std::isfinite(tol) || tol <= 0.)
			tol = 0.01 * width;

		for (int attempt = 0; attempt < 4; attempt++) {
			fixed.clear();
			for (unsigned int i : bnodes) {
				if (ft.nodes_tool_frame()[i].x >= x_max - tol)
					fixed.push_back(i);
			}
			if (fixed.size() >= 2)
				break;
			tol *= 5.0;
		}
		ft.set_mechanics_fixed_nodes(fixed);
		return;
	}
	std::string s(tags);
	std::size_t i = 0;
	while (i < s.size()) {
		while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ','))
			i++;
		if (i >= s.size())
			break;
		std::size_t j = i;
		while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t')
			j++;
		int tag = std::atoi(s.substr(i, j - i).c_str());
		if (tag != 0)
			ft.set_mechanics_fixed_on_physical(tag);
		i = j;
	}
}

static std::vector<glm::dvec2> extract_boundary_loop_world(const fe_tool &ft) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &edges = ft.boundary_edges();

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(edges.size());

	for (const auto &e : edges) {
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
	}

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);

	for (const auto &kv : adj) {
		unsigned int idx = kv.first;
		if (idx >= nodes.size())
			continue;
		glm::dvec2 p = ft.to_world_frame(nodes[idx]);
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = idx;
			start_p = p;
		}
	}

	if (!has_start)
		return {};

	std::vector<unsigned int> loop;
	loop.reserve(adj.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(adj.size()) + 2; it++) {
		loop.push_back(cur);
		const auto &nb = adj[cur];
		if (nb.empty())
			break;
		unsigned int next = nb[0];
		if (nb.size() >= 2 && next == prev)
			next = nb[1];
		if (next == start)
			break;
		prev = cur;
		cur = next;
	}

	std::vector<glm::dvec2> pts;
	pts.reserve(loop.size());
	for (unsigned int idx : loop) {
		if (idx >= nodes.size())
			continue;
		pts.push_back(ft.to_world_frame(nodes[idx]));
	}
	return pts;
}

static glm::dvec2 compute_nominal_tool_center(glm::dvec2 tl, double length, double height, double rake_angle, double clearance_angle) {
	glm::dvec2 tr(tl.x + length, tl.y);
	glm::dvec2 bl(tl.x, tl.y - height);

	double alpha_rake = rake_angle * M_PI / 180.;
	double alpha_free = (180 - 90 - clearance_angle) * M_PI / 180.;

	glm::dmat2x2 rot_rake(cos(alpha_rake), -sin(alpha_rake), sin(alpha_rake), cos(alpha_rake));
	glm::dmat2x2 rot_free(cos(alpha_free), -sin(alpha_free), sin(alpha_free), cos(alpha_free));

	glm::dvec2 down(0., -1.);
	glm::dvec2 trc = tr + down * rot_rake;
	glm::dvec2 blc = bl + down * rot_free;

	double x1 = tr.x, y1 = tr.y;
	double x2 = trc.x, y2 = trc.y;
	double x3 = bl.x, y3 = bl.y;
	double x4 = blc.x, y4 = blc.y;
	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	glm::dvec2 br = tr;
	if (std::isfinite(denom) && std::abs(denom) > 1e-30) {
		double d12 = x1 * y2 - y1 * x2;
		double d34 = x3 * y4 - y3 * x4;
		double px = (d12 * (x3 - x4) - (x1 - x2) * d34) / denom;
		double py = (d12 * (y3 - y4) - (y1 - y2) * d34) / denom;
		if (std::isfinite(px) && std::isfinite(py))
			br = glm::dvec2(px, py);
	}

	return 0.25 * (tl + tr + br + bl);
}

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

static glm::dvec2 closest_point_on_polyline(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	glm::dvec2 best(0.);
	double best_d2 = std::numeric_limits<double>::infinity();
	if (poly.size() < 2)
		return best;
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 cp = closest_point_on_segment(p, a, b);
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best = cp;
		}
	}
	return best;
}

static double poly_min_y(const std::vector<glm::dvec2> &poly) {
	double low = std::numeric_limits<double>::infinity();
	for (const auto &p : poly)
		low = std::min(low, p.y);
	if (!std::isfinite(low))
		return 0.0;
	return low;
}

static void enforce_fe_tool_corner_clearance(fe_tool &ft, glm::dvec2 wp_corner, double clearance_target_m, unsigned int iters) {
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft.boundary_loop_world();
		if (poly.size() < 3)
			return;

		double y_bottom = poly_min_y(poly);
		double dy = (wp_corner.y - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft.get_pos();
		pos.y += dy;
		ft.set_pose(pos, ft.get_vel());

		poly = ft.boundary_loop_world();
		if (poly.size() < 3)
			return;
		glm::dvec2 cp = closest_point_on_polyline(wp_corner, poly);
		double dx = wp_corner.x - cp.x;
		pos = ft.get_pos();
		pos.x += dx;
		ft.set_pose(pos, ft.get_vel());
	}
}

static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
										glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def))
			msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
			exit(1);
		}
	}

	fe_tool *ft = new fe_tool();
	if (!ft->load_gmsh_msh2(msh)) {
		delete ft;
		std::fprintf(stderr, "Failed to load MFREE_FE_TOOL_MSH\n");
		exit(1);
	}

	fe_tool::thermal_material mat;
	mat.rho = 14500.0;
	mat.cp = 200.0;
	mat.k = 80.0;
	try_read_env_double("MFREE_FE_TOOL_RHO", mat.rho);
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_RHO_TABLE", T, v))
			ft->set_material_table_rho(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_CP_TABLE", T, v))
			ft->set_material_table_cp(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_K_TABLE", T, v))
			ft->set_material_table_k(std::move(T), std::move(v));
	}

	fe_tool::mechanical_material mech;
	mech.E = 600e9;
	mech.nu = 0.22;
	mech.alpha = 4.5e-6;
	try_read_env_double("MFREE_FE_TOOL_E", mech.E);
	try_read_env_double("MFREE_FE_TOOL_NU", mech.nu);
	try_read_env_double("MFREE_FE_TOOL_ALPHA", mech.alpha);
	ft->set_mechanical_material(mech);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_E_TABLE", T, v))
			ft->set_mechanical_table_E(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_NU_TABLE", T, v))
			ft->set_mechanical_table_nu(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_ALPHA_TABLE", T, v))
			ft->set_mechanical_table_alpha(std::move(T), std::move(v));
	}
	ft->set_reference_temperature(T0);
	bool bc_validate = false;
	{
		int v = 0;
		if (try_read_env_int("MFREE_FE_BC_VALIDATE", v) && v != 0)
			bc_validate = true;
	}
	if (!bc_validate) {
		apply_mech_fix_tags_from_env(*ft);
	}

	ft->set_initial_temperature(T0);
	glm::dvec2 pos(0.);
	const char *align_env = getenv("MFREE_FE_TOOL_ALIGN_CENTER");
	bool align = true;
	if (align_env)
		align = (atoi(align_env) != 0);
	if (align) {
		glm::dvec2 mesh_center(0.);
		const auto &nodes = ft->nodes_tool_frame();
		for (const auto &p : nodes)
			mesh_center += p;
		if (!nodes.empty())
			mesh_center /= static_cast<double>(nodes.size());
		pos = desired_center - mesh_center;
	} else {
		try_read_env_double("MFREE_FE_TOOL_POS_X", pos.x);
		try_read_env_double("MFREE_FE_TOOL_POS_Y", pos.y);
	}
	ft->set_pose(pos, desired_vel);

	fe_tool::convection_bc air;
	air.h = 20.0;
	air.T_inf = 298.0;

	fe_tool::convection_bc water;
	water.h = 5000.0;
	water.T_inf = 293.15;

	double y_thresh = desired_edge_y;
	const char *y_env = getenv("MFREE_COOLANT_Y_THRESHOLD");
	if (y_env)
		y_thresh = atof(y_env);
	ft->set_convection_flooded_by_y(air, water, y_thresh);

	enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target_m, 5);

	// Always apply thermal Dirichlet BCs on tool top and rear boundaries.
	// These surfaces represent far-field bulk tool material held at the
	// reference temperature (298 K).  Applied unconditionally so the tool
	// never acts as a thermal sink/source at its edges.
	{
		int top_tag = 110;
		int rear_tag = 114;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_TOP_TAG", v))
				top_tag = v;
			if (try_read_env_int("MFREE_FE_BC_REAR_TAG", v))
				rear_tag = v;
		}
		bool top_found = false;
		bool rear_found = false;
		for (const auto &e : ft->boundary_edges()) {
			if (e.physical_tag == top_tag)
				top_found = true;
			if (e.physical_tag == rear_tag)
				rear_found = true;
		}
		if (top_found)
			ft->set_dirichlet_on_physical(top_tag, T0);
		if (rear_found)
			ft->set_dirichlet_on_physical(rear_tag, T0);
	}

	if (bc_validate) {
		int top_tag = 110;
		int rear_tag = 114;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_TOP_TAG", v))
				top_tag = v;
			if (try_read_env_int("MFREE_FE_BC_REAR_TAG", v))
				rear_tag = v;
		}

		ft->clear_mechanics_fixed();
		ft->clear_mechanics_fixed_nodes();
		bool top_found = false;
		bool rear_found = false;
		for (const auto &e : ft->boundary_edges()) {
			if (e.physical_tag == top_tag)
				top_found = true;
			if (e.physical_tag == rear_tag)
				rear_found = true;
		}
		if (top_found)
			ft->set_mechanics_fixed_y_on_physical(top_tag);
		else
			std::fprintf(stderr, "warning: FE BC validation top_tag=%d not found in FE tool boundary edges\n", top_tag);
		if (rear_found)
			ft->set_mechanics_fixed_y_on_physical(rear_tag);
		else
			std::fprintf(stderr, "warning: FE BC validation rear_tag=%d not found in FE tool boundary edges\n", rear_tag);

		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_ANCHOR_UX", v))
				anchor_ux = (v != 0);
		}
		if (anchor_ux && rear_found) {
			std::unordered_set<unsigned int> rear_nodes;
			for (const auto &e : ft->boundary_edges()) {
				if (e.physical_tag != rear_tag)
					continue;
				rear_nodes.insert(e.n0);
				rear_nodes.insert(e.n1);
			}
			if (rear_nodes.empty()) {
				std::fprintf(stderr, "warning: FE BC validation rear_tag=%d has no nodes; skipping UX anchor\n", rear_tag);
			} else {
				unsigned int anchor = 0;
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : rear_nodes) {
					glm::dvec2 pw = ft->node_world(n);
					if (!std::isfinite(pw.x))
						continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft->set_mechanics_fixed_x_nodes({anchor});
			}
		}

		if (top_found)
			ft->set_dirichlet_on_physical(top_tag, T0);
		if (rear_found)
			ft->set_dirichlet_on_physical(rear_tag, T0);
	}

	return ft;
}

body *cutting_ref_mr(unsigned int ny) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_cm_musec_g();

	double speed = 83.333328 * 1e-5;
	double mu_fric = 0.35;

	double hi_x = 0.100;
	double hi_y = 0.060;
	double lo_x = 0.000;
	double lo_y = 0.030;

	double dy = (hi_y - lo_y) / (ny - 1);
	double dx = dy;
	unsigned int nx = (hi_x - lo_x) / dx;
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1 * dx * hdx / (speed + c0);
	double t_final = 0.1 / speed * 0.5; // 1mm of cut

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx * ny);

	particle *particles = new particle[nx * ny];

	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i * dx;
			double py = j * dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	double rho0 = physical_constants.rho0();
	double T0 = physical_constants.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0))
		wp_T0 = T0;
	if (!std::isfinite(tool_T0))
		tool_T0 = T0;

	unsigned int n = nx * ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx * dx;
		particles[i].m = dx * dx * rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

		// fix bottom
		particles[i].fixed = (particles[i].y < lo_y + 0.5 * dy) ? true : false;
	}

	// correction constants
	double alpha = 1.;
	double beta = 1.;
	double eta = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx * dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants correction_constants(constants_monaghan(wdeltap, stress_exponent, art_stress_eps, hdx),
											  constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(physical_constants, correction_constants);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(physical_constants)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.)
		feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	return b;
}

body *cutting_ref_model5_fe_only(unsigned int nbox) {
	// Model 5 is derived from Model 2 with FE-tool-only coupled thermo-mechanical behavior.
	return cutting_ref_multi_resol_apriori(nbox);
}

body *cutting_ref_single_resol(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 1 & 4 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = pc.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0))
		wp_T0 = T0;
	if (!std::isfinite(tool_T0))
		tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k() / (rho0 * pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.)
		feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.)
		base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.)
		thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;

	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double dx = dy;
	unsigned int nx = (hi_x - lo_x) / dx + 1;
	double v_m_min = 500.;
	if (!try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min) || !std::isfinite(v_m_min) || v_m_min <= 0.) {
		v_m_min = 500.;
		std::fprintf(stderr, "WARNING: MFREE_CUTTING_SPEED_M_MIN not set or invalid; using default %.0f m/min\n", v_m_min);
	}
	double vc = v_m_min / 60.; // m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final = lc / vc;
	// Velocity-adaptive empirical dt cap: derived from the acoustic CFL at the
	// user-provided cutting speed so dt automatically tightens for higher speeds
	// and coarser resolutions.  Safety factor 0.20 (below workpiece_mechanical_safety
	// 0.25) keeps this cap as the binding constraint when both converge.
	// Override at runtime with MFREE_TIMESTEP_EMPIRICAL_CAP.
	double dt_empirical = 0.20 * hdx * dx / (pc.c0() + vc);
	try_read_env_double("MFREE_TIMESTEP_EMPIRICAL_CAP", dt_empirical);
	if (!std::isfinite(dt_empirical) || dt_empirical <= 0.)
		dt_empirical = 0.20 * hdx * dx / (pc.c0() + vc);
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles = new particle[nx * ny];

	srand(0);
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i * dx;
			double py = j * dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	unsigned int n = nx * ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx * dx;
		particles[i].m = dx * dx * rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5 * dy) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5 * dx);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta = 1.;
	double eta = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx * dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps, hdx),
							constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse); // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410 + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool = 0.000986074 - 0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}

	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction)
		b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly)
				low = std::min(low, p.y);
			if (!std::isfinite(low))
				low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}

		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly)
			low = std::min(low, p.y);
		if (!std::isfinite(low))
			low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, nx * ny);
	printf("<<< single-resolution simulation >>>\n");

	return b;
}

body *cutting_ref_multi_resol_apriori(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper (a-priori refinement)
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = pc.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0))
		wp_T0 = T0;
	if (!std::isfinite(tool_T0))
		tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k() / (rho0 * pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.)
		feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.)
		base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.)
		thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx / dx + 1;
	double v_m_min = 500.;
	if (!try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min) || !std::isfinite(v_m_min) || v_m_min <= 0.) {
		v_m_min = 500.;
		std::fprintf(stderr, "WARNING: MFREE_CUTTING_SPEED_M_MIN not set or invalid; using default %.0f m/min\n", v_m_min);
	}
	double vc = v_m_min / 60.; // m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5 * ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx / dxl + 1;
	unsigned int nyl = ly / dxl + 1;
	double dVl = dxl * dxl;
	double dVh = dxh * dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final = lc / vc;
	// Velocity-adaptive empirical dt cap: derived from the acoustic CFL at the
	// user-provided cutting speed so dt automatically tightens for higher speeds
	// and coarser resolutions.  Safety factor 0.20 (below workpiece_mechanical_safety
	// 0.25) keeps this cap as the binding constraint when both converge.
	// Override at runtime with MFREE_TIMESTEP_EMPIRICAL_CAP.
	double dt_empirical = 0.20 * hdx * dx / (pc.c0() + vc);
	try_read_env_double("MFREE_TIMESTEP_EMPIRICAL_CAP", dt_empirical);
	if (!std::isfinite(dt_empirical) || dt_empirical <= 0.)
		dt_empirical = 0.20 * hdx * dx / (pc.c0() + vc);
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles = new particle[nxh * nyh];

	srand(0);
	unsigned int part_iter = 0;

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i * dxh;
			double pyh = j * dxh;

			if ((pyh + lo_y) < (py_split - 1.1 * dxh))
				continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +++++++++++++++++
			   +               +
			   +-------CL------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i * dxl;
			double pyl = j * dxl;

			if ((pyl + lo_y) >= py_split)
				continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			/*
			 * low res
			 *

			   +-------CL------+
			   +               +
			   +++++++++++++++++

			 */

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	printf("n_single_resolution=%d   n_multi_resolution=%d   \n", nxh * nyh, n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step != 0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step != 0) ? dVh * rho0 : dVl * rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5 * dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5 * dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta = 1.;
	double eta = 0.1;

	double art_stress_eps = 0.2;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx * dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps, hdx),
							constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse); // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410 + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool = 0.000986074 - 0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction)
		b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly)
				low = std::min(low, p.y);
			if (!std::isfinite(low))
				low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}
		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly)
			low = std::min(low, p.y);
		if (!std::isfinite(low))
			low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

	return b;
}

body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = pc.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0))
		wp_T0 = T0;
	if (!std::isfinite(tool_T0))
		tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k() / (rho0 * pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.)
		feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.)
		base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.)
		thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx / dx + 1;
	double v_m_min = 500.;
	if (!try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min) || !std::isfinite(v_m_min) || v_m_min <= 0.) {
		v_m_min = 500.;
		std::fprintf(stderr, "WARNING: MFREE_CUTTING_SPEED_M_MIN not set or invalid; using default %.0f m/min\n", v_m_min);
	}
	double vc = v_m_min / 60.; // m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5 * ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx / dxl + 1;
	unsigned int nyl = ly / dxl + 1;
	double dVl = dxl * dxl;
	double dVh = dxh * dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final = lc / vc;
	// Velocity-adaptive empirical dt cap: derived from the acoustic CFL at the
	// user-provided cutting speed so dt automatically tightens for higher speeds
	// and coarser resolutions.  Safety factor 0.20 (below workpiece_mechanical_safety
	// 0.25) keeps this cap as the binding constraint when both converge.
	// Override at runtime with MFREE_TIMESTEP_EMPIRICAL_CAP.
	double dt_empirical = 0.20 * hdx * dx / (pc.c0() + vc);
	try_read_env_double("MFREE_TIMESTEP_EMPIRICAL_CAP", dt_empirical);
	if (!std::isfinite(dt_empirical) || dt_empirical <= 0.)
		dt_empirical = 0.20 * hdx * dx / (pc.c0() + vc);
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	std::vector<particle> particles;
	particles.reserve(nxh * nyh);

	srand(0);
	unsigned int part_iter = 0;

	// Keep Model 3's seeded refined block on the same lattice as the high-resolution particles.
	// The old literal limit (0.000117 m) landed between columns for the default 61-layer setup and
	// left a visible clearance strip at the refinement front. Snap to an integer dxh column instead.
	double initial_refined_x_max = dxh * std::ceil((0.000117 - 1e-12) / dxh);

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i * dxh;
			double pyh = j * dxh;

			if ((pyh + lo_y) < (py_split - 1.9 * dxh) || pxh > initial_refined_x_max)
				continue;

			particles.emplace_back(part_iter);
			particle &p = particles.back();
			p.x = pxh + lo_x;
			p.y = pyh + lo_y;
			p.X = pxh + lo_x;
			p.Y = pyh + lo_y;
			p.refine_step = 1;
			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i * dxl;
			double pyl = j * dxl;

			if ((pyl + lo_y) >= (py_split - 1.9 * dxh) && pxl <= initial_refined_x_max)
				continue;

			particles.emplace_back(part_iter);
			particle &p = particles.back();
			p.x = pxl + lo_x;
			p.y = pyl + lo_y;
			p.X = pxl + lo_x;
			p.Y = pyl + lo_y;
			p.refine_step = 0;
			part_iter++;
		}
	}

	// total #particles
	unsigned int n = particles.size();

	printf("n_single_resolution=%d   n_current=%d  \n", nxh * nyh, n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step != 0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step != 0) ? dVh * rho0 : dVl * rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5 * dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5 * dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta = 1.;
	double eta = 0.1;

	double art_stress_eps = 0.2;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx * dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps, hdx),
							constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(std::move(particles), sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse); // optional: thermal_brookshaw

	// adaptivity settings

	// default settings +-+-++-+-+-+-+-+-+-+-+-+-+
	double alpha_dx = 0.50;
	double beta_h = 0.50;
	double v_cr = 0.40;
	double div_v_cr = 2e+5;
	double SvM_cr = 1e+7;
	double eps_cr = 110;
	double T_cr = 700.;
	glm::dvec2 xy_min = {0.25, 0.25};
	glm::dvec2 xy_max = {0.75, 0.75};
	double frame_width = 0.000350;
	double frame_height = 0.000060;

	// Model 3 refinement-frame controls.  The historical moving frame was shallow
	// (60 µm), which can place the coarse/fine boundary inside the active shear
	// and chip-formation zone.  These environment controls allow moving that
	// boundary below/ahead of the high-gradient region without recompilation.
	// Defaults preserve the historical geometry unless the run script opts in.
	double refine_depth_factor = 0.;
	if (try_read_env_double("MFREE_REFINE_DEPTH_FACTOR", refine_depth_factor) && std::isfinite(refine_depth_factor) &&
		refine_depth_factor > 0.) {
		refine_depth_factor = std::max(1.5, std::min(3.0, refine_depth_factor));
		// Base depth on the actual cut/feed depth (feed_per_rev_mm), not target_feed
		// (which includes the base-target offset and would reach full workpiece depth).
		frame_height = refine_depth_factor * (feed_per_rev_mm * 1e-3);
	}
	// Cap: never allow the moving frame to reach the fixed bottom boundary.
	// Leave at least 2 coarse layers (2*dxl) below the lowest frame position.
	// Lowest frame y = tool_tip_y - frame_height >= lo_y + 2*dxl.
	// A conservative upper bound on tool_tip_y is hi_y, so:
	const double max_frame_height = (hi_y - lo_y) - 2.0 * dxl;
	if (frame_height > max_frame_height) {
		std::printf("[adaptivity] frame_height capped %.4f mm -> %.4f mm (2 coarse layers from bottom)\n", frame_height * 1e3,
					max_frame_height * 1e3);
		frame_height = max_frame_height;
	}
	double frame_width_mm = frame_width * 1e3;
	if (try_read_env_double("MFREE_REFINE_FRAME_WIDTH_MM", frame_width_mm) && std::isfinite(frame_width_mm) && frame_width_mm > 0.) {
		frame_width = frame_width_mm * 1e-3;
	}
	double frame_height_mm = frame_height * 1e3;
	if (try_read_env_double("MFREE_REFINE_FRAME_HEIGHT_MM", frame_height_mm) && std::isfinite(frame_height_mm) && frame_height_mm > 0.) {
		frame_height = frame_height_mm * 1e-3;
	}
	int refine_halo_layers = 0;
	if (try_read_env_int("MFREE_REFINE_HALO_LAYERS", refine_halo_layers)) {
		refine_halo_layers = std::max(0, std::min(10, refine_halo_layers));
	}
	frame_width += static_cast<double>(refine_halo_layers) * dxl;
	frame_height += static_cast<double>(refine_halo_layers) * dxl;
	if (dxl > 0.) {
		frame_width = std::ceil((frame_width - 1e-12) / dxl) * dxl;
		frame_height = std::ceil((frame_height - 1e-12) / dxl) * dxl;
	}
	std::printf("refinement frame: width=%.6e m height=%.6e m depth_factor=%.3f halo_layers=%d coarse_dx=%.6e m\n", frame_width,
				frame_height, refine_depth_factor, refine_halo_layers, dxl);

	unsigned int n_nbh = 10;
	double l_eff = lc + 0.1 * lx;
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	adaptivity *adapt = new adaptivity(alpha_dx, beta_h, v_cr, div_v_cr, SvM_cr, eps_cr, T_cr, xy_min, xy_max, frame_width, frame_height,
									   n_nbh, l_eff, true);

	adapt->set_refine_criterion(adaptivity::refine_criteria::moving_frame);
	adapt->set_refine_pattern(adaptivity::pattern::cubic_basic);

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410 + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool = 0.000986074 - 0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction)
		b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);
	b->set_adaptivity(adapt);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly)
				low = std::min(low, p.y);
			if (!std::isfinite(low))
				low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}
		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly)
			low = std::min(low, p.y);
		if (!std::isfinite(low))
			low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< dynamic refinement model >>>\n");

	return b;
}
