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

#include "fe_tool.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

static bool starts_with(const std::string &s, const char *prefix) {
	return s.rfind(prefix, 0) == 0;
}

static double tri_area2(const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

fe_tool::fe_tool() {}

void fe_tool::set_mesh(const std::vector<glm::dvec2> &nodes_tool_frame,
                       const std::vector<std::array<unsigned int, 3>> &triangles,
                       const std::vector<boundary_edge> &boundary_edges) {
	m_nodes_tool = nodes_tool_frame;
	m_tris = triangles;
	m_bnd = boundary_edges;
	m_line_elements = boundary_edges;
	m_bnd_edge_to_tri.clear();

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_K_rows.assign(m_nodes_tool.size(), {});
	m_power_sources.assign(m_nodes_tool.size(), 0.);
	m_force_sources.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();
	build_conduction_operator();
	build_mechanics_operator();
}

bool fe_tool::load_gmsh_msh2(const std::string &path) {
	std::ifstream in(path);
	if (!in) return false;

	m_nodes_tool.clear();
	m_tris.clear();
	m_bnd.clear();
	m_line_elements.clear();
	m_bnd_edge_to_tri.clear();
	m_T.clear();
	m_capacity.clear();
	m_K_rows.clear();
	m_power_sources.clear();

	std::string line;
	int msh_version_major = 0;
	while (std::getline(in, line)) {
		if (line == "$MeshFormat") {
			std::getline(in, line);
			std::istringstream iss(line);
			double ver = 0.;
			int file_type = 0;
			int data_size = 0;
			iss >> ver >> file_type >> data_size;
			msh_version_major = static_cast<int>(std::floor(ver + 1e-12));
			std::getline(in, line);
		} else if (line == "$Nodes") {
			std::getline(in, line);
			std::size_t n = 0;
			{
				std::istringstream iss(line);
				iss >> n;
			}
			m_nodes_tool.resize(n);
			for (std::size_t i = 0; i < n; i++) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::size_t id = 0;
				double x = 0., y = 0., z = 0.;
				iss >> id >> x >> y >> z;
				if (id == 0 || id > n) return false;
				m_nodes_tool[id - 1] = glm::dvec2(x, y);
			}
			std::getline(in, line);
		} else if (line == "$Elements") {
			std::getline(in, line);
			std::size_t n = 0;
			{
				std::istringstream iss(line);
				iss >> n;
			}
			for (std::size_t i = 0; i < n; i++) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::size_t id = 0;
				int type = 0;
				int num_tags = 0;
				iss >> id >> type >> num_tags;

				int physical = 0;
				for (int t = 0; t < num_tags; t++) {
					int tag = 0;
					iss >> tag;
					if (t == 0) physical = tag;
				}

				if (type == 2) {
					unsigned int n0 = 0, n1 = 0, n2 = 0;
					iss >> n0 >> n1 >> n2;
					if (n0 == 0 || n1 == 0 || n2 == 0) return false;
					m_tris.push_back({n0 - 1, n1 - 1, n2 - 1});
				} else if (type == 1) {
					unsigned int n0 = 0, n1 = 0;
					iss >> n0 >> n1;
					if (n0 == 0 || n1 == 0) return false;
					boundary_edge e;
					e.n0 = n0 - 1;
					e.n1 = n1 - 1;
					e.physical_tag = physical;
					m_line_elements.push_back(e);
				}
			}
			std::getline(in, line);
		}
	}

	if (msh_version_major != 2) return false;
	if (m_nodes_tool.empty()) return false;
	if (m_tris.empty()) return false;

	build_boundary_edges_from_lines();
	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_power_sources.assign(m_nodes_tool.size(), 0.);
	m_force_sources.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_conduction_operator();
	build_mechanics_operator();

	return true;
}

void fe_tool::set_material(thermal_material mat) {
	m_mat = mat;
	build_conduction_operator();
}

fe_tool::thermal_material fe_tool::get_material() const {
	return m_mat;
}

void fe_tool::set_mechanical_material(mechanical_material mat) {
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(mat.E) || mat.E <= eps) return;
	if (!std::isfinite(mat.nu) || mat.nu <= (-1.0 + eps) || mat.nu >= (0.5 - eps)) return;
	if (!std::isfinite(mat.alpha) || mat.alpha < -eps) return;
	m_mech = mat;
	build_mechanics_operator();
}

fe_tool::mechanical_material fe_tool::get_mechanical_material() const { return m_mech; }

void fe_tool::set_reference_temperature(double T_ref) { m_T_ref = T_ref; }

double fe_tool::reference_temperature() const { return m_T_ref; }

void fe_tool::set_mechanics_fixed_on_physical(int physical_tag) { m_mech_fix_tags.insert(physical_tag); }

void fe_tool::clear_mechanics_fixed() { m_mech_fix_tags.clear(); }

void fe_tool::set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_nodes.clear();
	m_mech_fix_nodes.insert(nodes.begin(), nodes.end());
}

void fe_tool::clear_mechanics_fixed_nodes() { m_mech_fix_nodes.clear(); }

void fe_tool::set_initial_temperature(double T0) {
	for (std::size_t i = 0; i < m_T.size(); i++) m_T[i] = T0;
	m_T_ref = T0;
}

void fe_tool::set_pose(glm::dvec2 pos, glm::dvec2 vel) {
	m_pos = pos;
	m_vel = vel;
}

glm::dvec2 fe_tool::get_pos() const { return m_pos; }
glm::dvec2 fe_tool::get_vel() const { return m_vel; }

void fe_tool::update_pose(double dt) { m_pos += dt * m_vel; }

glm::dvec2 fe_tool::to_tool_frame(glm::dvec2 x_world) const { return x_world - m_pos; }
glm::dvec2 fe_tool::to_world_frame(glm::dvec2 x_tool) const { return x_tool + m_pos; }

const std::vector<glm::dvec2> &fe_tool::nodes_tool_frame() const { return m_nodes_tool; }
const std::vector<std::array<unsigned int, 3>> &fe_tool::triangles() const { return m_tris; }
const std::vector<fe_tool::boundary_edge> &fe_tool::boundary_edges() const { return m_bnd; }

double fe_tool::temperature_at_node(unsigned int i) const {
	assert(i < m_T.size());
	return m_T[i];
}

double fe_tool::temperature_at_world_point_nearest_boundary(glm::dvec2 x_world) const {
	glm::dvec2 x_tool = to_tool_frame(x_world);
	auto [edge_idx, t] = nearest_boundary_edge_barycentric(x_tool);
	if (edge_idx >= m_bnd.size()) return 0.;

	const boundary_edge &e = m_bnd[edge_idx];
	double T0 = m_T[e.n0];
	double T1 = m_T[e.n1];
	return (1. - t) * T0 + t * T1;
}

void fe_tool::clear_sources() {
	for (std::size_t i = 0; i < m_power_sources.size(); i++) m_power_sources[i] = 0.;
}

void fe_tool::add_nodal_power(unsigned int node, double power) {
	if (node >= m_power_sources.size()) return;
	m_power_sources[node] += power;
}

void fe_tool::add_boundary_point_power(glm::dvec2 x_world, double power) {
	glm::dvec2 x_tool = to_tool_frame(x_world);
	auto [edge_idx, t] = nearest_boundary_edge_barycentric(x_tool);
	if (edge_idx >= m_bnd.size()) return;

	const boundary_edge &e = m_bnd[edge_idx];
	m_power_sources[e.n0] += (1. - t) * power;
	m_power_sources[e.n1] += t * power;
}

double fe_tool::nodal_power(unsigned int node) const {
	if (node >= m_power_sources.size()) return 0.;
	return m_power_sources[node];
}

void fe_tool::clear_forces() {
	for (std::size_t i = 0; i < m_force_sources.size(); i++) m_force_sources[i] = glm::dvec2(0.);
}

void fe_tool::add_nodal_force(unsigned int node, glm::dvec2 force) {
	if (node >= m_force_sources.size()) return;
	m_force_sources[node] += force;
}

void fe_tool::add_boundary_point_force(glm::dvec2 x_world, glm::dvec2 force) {
	glm::dvec2 x_tool = to_tool_frame(x_world);
	auto [edge_idx, t] = nearest_boundary_edge_barycentric(x_tool);
	if (edge_idx >= m_bnd.size()) return;

	const boundary_edge &e = m_bnd[edge_idx];
	m_force_sources[e.n0] += (1. - t) * force;
	m_force_sources[e.n1] += t * force;
}

glm::dvec2 fe_tool::nodal_force(unsigned int node) const {
	if (node >= m_force_sources.size()) return glm::dvec2(0.);
	return m_force_sources[node];
}

glm::dvec2 fe_tool::node_world(unsigned int i) const {
	if (i >= m_nodes_tool.size()) return glm::dvec2(0.);
	glm::dvec2 x_tool = m_nodes_tool[i] + (i < m_u.size() ? m_u[i] : glm::dvec2(0.));
	return to_world_frame(x_tool);
}

const std::vector<unsigned int> &fe_tool::boundary_loop_nodes() const { return m_boundary_loop; }

std::vector<glm::dvec2> fe_tool::boundary_loop_world() const {
	if (m_boundary_loop.size() >= 3) {
		std::vector<glm::dvec2> pts;
		pts.reserve(m_boundary_loop.size());
		for (unsigned int i : m_boundary_loop) pts.push_back(node_world(i));
		return pts;
	}

	std::unordered_set<unsigned int> bnodes;
	bnodes.reserve(2 * m_bnd.size());
	for (const boundary_edge &e : m_bnd) {
		if (e.n0 < m_nodes_tool.size()) bnodes.insert(e.n0);
		if (e.n1 < m_nodes_tool.size()) bnodes.insert(e.n1);
	}
	if (bnodes.size() < 3) return {};

	std::vector<glm::dvec2> pts;
	pts.reserve(bnodes.size());
	for (unsigned int i : bnodes) pts.push_back(node_world(i));

	auto cross = [](const glm::dvec2 &o, const glm::dvec2 &a, const glm::dvec2 &b) {
		return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
	};

	std::sort(pts.begin(), pts.end(), [](const glm::dvec2 &a, const glm::dvec2 &b) {
		if (a.x != b.x) return a.x < b.x;
		return a.y < b.y;
	});
	pts.erase(std::unique(pts.begin(), pts.end(), [](const glm::dvec2 &a, const glm::dvec2 &b) { return a.x == b.x && a.y == b.y; }), pts.end());
	if (pts.size() < 3) return {};

	std::vector<glm::dvec2> hull;
	hull.reserve(2 * pts.size());

	for (const auto &p : pts) {
		while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0.) hull.pop_back();
		hull.push_back(p);
	}
	std::size_t lower_size = hull.size();
	for (std::size_t i = pts.size(); i-- > 0;) {
		const auto &p = pts[i];
		while (hull.size() > lower_size && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0.) hull.pop_back();
		hull.push_back(p);
	}
	if (!hull.empty()) hull.pop_back();
	return hull;
}

const std::vector<glm::dvec2> &fe_tool::displacements() const { return m_u; }

void fe_tool::set_displacements(const std::vector<glm::dvec2> &u) {
	m_u = u;
	if (m_u.size() != m_nodes_tool.size()) m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
}

void fe_tool::set_convection_on_physical(int physical_tag, convection_bc bc) {
	m_conv_by_tag[physical_tag] = bc;
}

void fe_tool::set_contact_convergence(contact_convergence c) { m_contact_conv = c; }

fe_tool::contact_convergence fe_tool::get_contact_convergence() const { return m_contact_conv; }

void fe_tool::set_contact_energy_balance(contact_energy_balance b) { m_contact_energy = b; }

fe_tool::contact_energy_balance fe_tool::get_contact_energy_balance() const { return m_contact_energy; }

void fe_tool::set_dirichlet_on_physical(int physical_tag, double T) {
	m_dirichlet_by_tag[physical_tag] = T;
}

void fe_tool::set_convection_air_all_exposed(convection_bc air_bc) {
	m_air_all = air_bc;
	m_use_air_all = true;
	m_use_flooded_by_y = false;
}

void fe_tool::set_convection_flooded_by_y(convection_bc air_bc, convection_bc water_bc, double y_threshold_world) {
	m_flood_air = air_bc;
	m_flood_water = water_bc;
	m_flood_y_threshold_world = y_threshold_world;
	m_use_air_all = false;
	m_use_flooded_by_y = true;
}

double fe_tool::max_temperature() const {
	double mx = -std::numeric_limits<double>::infinity();
	for (double v : m_T) mx = std::max(mx, v);
	return mx;
}

double fe_tool::min_temperature() const {
	double mn = std::numeric_limits<double>::infinity();
	for (double v : m_T) mn = std::min(mn, v);
	return mn;
}

void fe_tool::advance_explicit(double dt) {
	if (m_T.empty()) return;
	if (m_capacity.size() != m_T.size()) return;
	if (m_K_rows.size() != m_T.size()) return;
	if (m_mat.rho <= 0. || m_mat.cp <= 0. || m_mat.k < 0.) return;

	if (!m_dirichlet_by_tag.empty() && !m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			auto it = m_dirichlet_by_tag.find(e.physical_tag);
			if (it == m_dirichlet_by_tag.end()) continue;
			double T = it->second;
			m_T[e.n0] = T;
			m_T[e.n1] = T;
		}
	}

	std::vector<double> power(m_T.size(), 0.);

	for (std::size_t i = 0; i < m_T.size(); i++) {
		double pi = 0.;
		for (const auto &kv : m_K_rows[i]) {
			pi -= kv.second * m_T[kv.first];
		}
		power[i] += pi;
	}

	if (!m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			convection_bc bc;
			bool active = false;

			auto it = m_conv_by_tag.find(e.physical_tag);
			if (it != m_conv_by_tag.end()) {
				bc = it->second;
				active = bc.h > 0.;
			} else if (m_use_air_all) {
				bc = m_air_all;
				active = bc.h > 0.;
			} else if (m_use_flooded_by_y) {
				glm::dvec2 p0 = to_world_frame(m_nodes_tool[e.n0]);
				glm::dvec2 p1 = to_world_frame(m_nodes_tool[e.n1]);
				double ym = 0.5 * (p0.y + p1.y);
				bc = (ym >= m_flood_y_threshold_world) ? m_flood_water : m_flood_air;
				active = bc.h > 0.;
			}

			if (!active) continue;

			glm::dvec2 x0 = m_nodes_tool[e.n0];
			glm::dvec2 x1 = m_nodes_tool[e.n1];
			double L = glm::length(x1 - x0);
			if (L <= 0.) continue;

			double Ti = m_T[e.n0];
			double Tj = m_T[e.n1];
			double di = bc.T_inf - Ti;
			double dj = bc.T_inf - Tj;

			double pi = bc.h * L / 6.0 * (2.0 * di + dj);
			double pj = bc.h * L / 6.0 * (di + 2.0 * dj);
			power[e.n0] += pi;
			power[e.n1] += pj;
		}
	}

	for (std::size_t i = 0; i < m_T.size(); i++) power[i] += m_power_sources[i];

	for (std::size_t i = 0; i < m_T.size(); i++) {
		double cap = m_capacity[i];
		if (cap <= 0.) continue;
		m_T[i] += dt * power[i] / cap;
	}

	if (!m_dirichlet_by_tag.empty() && !m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			auto it = m_dirichlet_by_tag.find(e.physical_tag);
			if (it == m_dirichlet_by_tag.end()) continue;
			double T = it->second;
			m_T[e.n0] = T;
			m_T[e.n1] = T;
		}
	}
}

void fe_tool::build_boundary_edges_from_lines() {
	m_bnd.clear();
	std::unordered_map<edge_key, boundary_edge, edge_key_hash> best;
	best.reserve(m_line_elements.size());

	for (const boundary_edge &e : m_line_elements) {
		edge_key k;
		k.a = std::min(e.n0, e.n1);
		k.b = std::max(e.n0, e.n1);

		auto it = best.find(k);
		if (it == best.end()) {
			best.emplace(k, e);
			continue;
		}

		const int old_tag = it->second.physical_tag;
		const int new_tag = e.physical_tag;
		if (old_tag == 100 && new_tag != 100) it->second = e;
	}

	m_bnd.reserve(best.size());
	for (const auto &kv : best) m_bnd.push_back(kv.second);
}

void fe_tool::build_boundary_edge_to_adjacent_triangle() {
	m_bnd_edge_to_tri.clear();
	std::unordered_map<edge_key, unsigned int, edge_key_hash> tri_of_edge;

	for (unsigned int t = 0; t < m_tris.size(); t++) {
		const auto &tri = m_tris[t];
		unsigned int a = tri[0], b = tri[1], c = tri[2];

		auto add = [&](unsigned int i, unsigned int j) {
			edge_key k;
			k.a = std::min(i, j);
			k.b = std::max(i, j);
			if (tri_of_edge.find(k) == tri_of_edge.end()) tri_of_edge[k] = t;
		};
		add(a, b);
		add(b, c);
		add(c, a);
	}

	for (const boundary_edge &e : m_bnd) {
		edge_key k;
		k.a = std::min(e.n0, e.n1);
		k.b = std::max(e.n0, e.n1);
		auto it = tri_of_edge.find(k);
		if (it != tri_of_edge.end()) m_bnd_edge_to_tri[k] = it->second;
	}
}

void fe_tool::build_boundary_loop() {
	m_boundary_loop.clear();
	if (m_bnd.empty() || m_nodes_tool.empty()) return;

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(2 * m_bnd.size());
	std::unordered_set<unsigned int> bnodes;
	bnodes.reserve(2 * m_bnd.size());

	for (const boundary_edge &e : m_bnd) {
		if (e.n0 >= m_nodes_tool.size() || e.n1 >= m_nodes_tool.size()) continue;
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
		bnodes.insert(e.n0);
		bnodes.insert(e.n1);
	}
	if (bnodes.size() < 3) return;

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);
	for (unsigned int i : bnodes) {
		glm::dvec2 p = m_nodes_tool[i];
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = i;
			start_p = p;
		}
	}
	if (!has_start) return;

	std::unordered_set<unsigned int> visited;
	visited.reserve(bnodes.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(bnodes.size()) + 4; it++) {
		if (visited.find(cur) != visited.end()) break;
		visited.insert(cur);
		m_boundary_loop.push_back(cur);

		auto it_adj = adj.find(cur);
		if (it_adj == adj.end()) break;
		const std::vector<unsigned int> &nb = it_adj->second;
		if (nb.empty()) break;

		unsigned int next = nb[0];
		if (prev == std::numeric_limits<unsigned int>::max()) {
			next = nb[0];
		} else if (nb.size() == 1) {
			next = nb[0];
		} else if (nb.size() == 2) {
			next = (nb[0] == prev ? nb[1] : nb[0]);
		} else {
			glm::dvec2 t_prev = m_nodes_tool[cur] - m_nodes_tool[prev];
			double t_prev_n = glm::length(t_prev);
			if (t_prev_n > 0.) t_prev /= t_prev_n;
			double best = -std::numeric_limits<double>::infinity();
			next = prev;
			for (unsigned int cand : nb) {
				if (cand == prev) continue;
				glm::dvec2 t_c = m_nodes_tool[cand] - m_nodes_tool[cur];
				double t_c_n = glm::length(t_c);
				if (t_c_n > 0.) t_c /= t_c_n;
				double score = glm::dot(t_prev, t_c);
				if (score > best) {
					best = score;
					next = cand;
				}
			}
			if (next == prev) next = nb[0];
		}

		if (next == start) break;
		prev = cur;
		cur = next;
	}
}

void fe_tool::build_conduction_operator() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_mat.rho <= 0. || m_mat.cp <= 0. || m_mat.k < 0.) return;

	m_capacity.assign(m_nodes_tool.size(), 0.);

	std::vector<std::unordered_map<unsigned int, double>> rows(m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv4A = 1.0 / (4.0 * A);

		double kfac = m_mat.k * inv4A;

		double ke[3][3];
		double b[3] = {b0, b1, b2};
		double c[3] = {c0, c1, c2};
		for (int a = 0; a < 3; a++) {
			for (int bidx = 0; bidx < 3; bidx++) {
				ke[a][bidx] = kfac * (b[a] * b[bidx] + c[a] * c[bidx]);
			}
		}

		unsigned int idxs[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			unsigned int ia = idxs[a];
			for (int bb = 0; bb < 3; bb++) {
				unsigned int ib = idxs[bb];
				rows[ia][ib] += ke[a][bb];
			}
		}

		double cap = m_mat.rho * m_mat.cp * A / 3.0;
		m_capacity[i0] += cap;
		m_capacity[i1] += cap;
		m_capacity[i2] += cap;
	}

	m_K_rows.assign(m_nodes_tool.size(), {});
	for (unsigned int i = 0; i < rows.size(); i++) {
		m_K_rows[i].reserve(rows[i].size());
		for (const auto &kv : rows[i]) m_K_rows[i].push_back({kv.first, kv.second});
	}
}

void fe_tool::build_mechanics_operator() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(m_mech.E) || m_mech.E <= eps) return;
	if (!std::isfinite(m_mech.nu) || m_mech.nu <= (-1.0 + eps) || m_mech.nu >= (0.5 - eps)) return;
	if (!std::isfinite(m_mech.alpha) || m_mech.alpha < -eps) return;

	std::vector<std::unordered_map<unsigned int, double>> rows(2 * m_nodes_tool.size());

	double E = m_mech.E;
	double nu = m_mech.nu;
	double c = E / ((1.0 + nu) * (1.0 - 2.0 * nu));
	double D[3][3] = {
		{c * (1.0 - nu), c * nu, 0.0},
		{c * nu, c * (1.0 - nu), 0.0},
		{0.0, 0.0, c * 0.5 * (1.0 - 2.0 * nu)},
	};

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv2A = 1.0 / (2.0 * A);
		double dNdx[3] = {b0 * inv2A, b1 * inv2A, b2 * inv2A};
		double dNdy[3] = {c0 * inv2A, c1 * inv2A, c2 * inv2A};

		double B[3][6] = {
			{dNdx[0], 0., dNdx[1], 0., dNdx[2], 0.},
			{0., dNdy[0], 0., dNdy[1], 0., dNdy[2]},
			{dNdy[0], dNdx[0], dNdy[1], dNdx[1], dNdy[2], dNdx[2]},
		};

		double DB[3][6];
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 6; j++) {
				DB[i][j] = D[i][0] * B[0][j] + D[i][1] * B[1][j] + D[i][2] * B[2][j];
			}
		}

		double ke[6][6];
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++) {
				double v = 0.;
				for (int k = 0; k < 3; k++) v += B[k][i] * DB[k][j];
				ke[i][j] = A * v;
			}
		}

		unsigned int idxn[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			for (int b = 0; b < 3; b++) {
				for (int da = 0; da < 2; da++) {
					for (int db = 0; db < 2; db++) {
						unsigned int ia = 2 * idxn[a] + static_cast<unsigned int>(da);
						unsigned int ib = 2 * idxn[b] + static_cast<unsigned int>(db);
						rows[ia][ib] += ke[2 * a + da][2 * b + db];
					}
				}
			}
		}
	}

	m_Km_rows.assign(2 * m_nodes_tool.size(), {});
	for (unsigned int i = 0; i < rows.size(); i++) {
		m_Km_rows[i].reserve(rows[i].size());
		for (const auto &kv : rows[i]) m_Km_rows[i].push_back({kv.first, kv.second});
	}
}

void fe_tool::solve_mechanics_quasistatic(unsigned int max_iters, double rel_tol) {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_Km_rows.size() != 2 * m_nodes_tool.size()) return;
	if (m_force_sources.size() != m_nodes_tool.size()) return;

	const double eps = std::numeric_limits<double>::epsilon();

	std::vector<char> constrained(2 * m_nodes_tool.size(), 0);
	if ((!m_mech_fix_tags.empty() || !m_mech_fix_nodes.empty()) && !m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			bool fix = false;
			if (!m_mech_fix_tags.empty() && m_mech_fix_tags.find(e.physical_tag) != m_mech_fix_tags.end()) fix = true;
			if (!fix && !m_mech_fix_nodes.empty() && (m_mech_fix_nodes.find(e.n0) != m_mech_fix_nodes.end() || m_mech_fix_nodes.find(e.n1) != m_mech_fix_nodes.end())) fix = true;
			if (!fix) continue;
			if (e.n0 < m_nodes_tool.size()) {
				constrained[2 * e.n0 + 0] = 1;
				constrained[2 * e.n0 + 1] = 1;
			}
			if (e.n1 < m_nodes_tool.size()) {
				constrained[2 * e.n1 + 0] = 1;
				constrained[2 * e.n1 + 1] = 1;
			}
		}
	}

	unsigned int fixed_nodes = 0;
	unsigned int fixed_nodes_x = 0;
	unsigned int fixed_nodes_y = 0;
	unsigned int fixed_dofs = 0;
	{
		std::unordered_set<unsigned int> uniq;
		std::unordered_set<unsigned int> uniq_x;
		std::unordered_set<unsigned int> uniq_y;
		uniq.reserve(m_nodes_tool.size() / 4 + 4);
		uniq_x.reserve(m_nodes_tool.size() / 4 + 4);
		uniq_y.reserve(m_nodes_tool.size() / 4 + 4);
		for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
			if (constrained[2 * i + 0] || constrained[2 * i + 1]) uniq.insert(i);
			if (constrained[2 * i + 0]) uniq_x.insert(i);
			if (constrained[2 * i + 1]) uniq_y.insert(i);
		}
		fixed_nodes = static_cast<unsigned int>(uniq.size());
		fixed_nodes_x = static_cast<unsigned int>(uniq_x.size());
		fixed_nodes_y = static_cast<unsigned int>(uniq_y.size());
		for (unsigned int i = 0; i < constrained.size(); i++) fixed_dofs += (constrained[i] ? 1u : 0u);
	}
	if (fixed_nodes < 3 || fixed_dofs < 3 || fixed_nodes_x == 0 || fixed_nodes_y == 0) {
		std::fprintf(stderr,
		             "warning: fe_tool mechanics solve has insufficient constraints (fixed_nodes=%u fixed_dofs=%u fixed_nodes_x=%u fixed_nodes_y=%u vel_x=%g)\n",
		             fixed_nodes, fixed_dofs, fixed_nodes_x, fixed_nodes_y, m_vel.x);
		return;
	}

	std::vector<double> rhs(2 * m_nodes_tool.size(), 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		rhs[2 * i + 0] += m_force_sources[i].x;
		rhs[2 * i + 1] += m_force_sources[i].y;
	}

	if (!std::isfinite(m_mech.E) || m_mech.E <= eps) return;
	if (!std::isfinite(m_mech.nu) || m_mech.nu <= (-1.0 + eps) || m_mech.nu >= (0.5 - eps)) return;
	if (!std::isfinite(m_mech.alpha) || m_mech.alpha < -eps) return;
	if (std::abs(m_mech.alpha) > eps && !m_T.empty()) {
		double E = m_mech.E;
		double nu = m_mech.nu;
		double c = E / ((1.0 + nu) * (1.0 - 2.0 * nu));
		double D[3][3] = {
			{c * (1.0 - nu), c * nu, 0.0},
			{c * nu, c * (1.0 - nu), 0.0},
			{0.0, 0.0, c * 0.5 * (1.0 - 2.0 * nu)},
		};

		for (const auto &tri : m_tris) {
			unsigned int i0 = tri[0];
			unsigned int i1 = tri[1];
			unsigned int i2 = tri[2];
			if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;

			const glm::dvec2 &x0 = m_nodes_tool[i0];
			const glm::dvec2 &x1 = m_nodes_tool[i1];
			const glm::dvec2 &x2 = m_nodes_tool[i2];

			double area2 = tri_area2(x0, x1, x2);
			double A = 0.5 * std::abs(area2);
			if (A <= 0.) continue;

			double b0 = x1.y - x2.y;
			double c0 = x2.x - x1.x;
			double b1 = x2.y - x0.y;
			double c1 = x0.x - x2.x;
			double b2 = x0.y - x1.y;
			double c2 = x1.x - x0.x;

			double inv2A = 1.0 / (2.0 * A);
			double dNdx[3] = {b0 * inv2A, b1 * inv2A, b2 * inv2A};
			double dNdy[3] = {c0 * inv2A, c1 * inv2A, c2 * inv2A};

			double B[3][6] = {
				{dNdx[0], 0., dNdx[1], 0., dNdx[2], 0.},
				{0., dNdy[0], 0., dNdy[1], 0., dNdy[2]},
				{dNdy[0], dNdx[0], dNdy[1], dNdx[1], dNdy[2], dNdx[2]},
			};

			double Tavg = (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0;
			double dT = Tavg - m_T_ref;
			double eps_th[3] = {m_mech.alpha * dT, m_mech.alpha * dT, 0.0};

			double sig_th[3] = {
				D[0][0] * eps_th[0] + D[0][1] * eps_th[1] + D[0][2] * eps_th[2],
				D[1][0] * eps_th[0] + D[1][1] * eps_th[1] + D[1][2] * eps_th[2],
				D[2][0] * eps_th[0] + D[2][1] * eps_th[1] + D[2][2] * eps_th[2],
			};

			double fe[6] = {0., 0., 0., 0., 0., 0.};
			for (int a = 0; a < 6; a++) {
				double v = 0.;
				for (int k = 0; k < 3; k++) v += B[k][a] * sig_th[k];
				fe[a] = A * v;
			}

			unsigned int idxn[3] = {i0, i1, i2};
			for (int a = 0; a < 3; a++) {
				rhs[2 * idxn[a] + 0] += fe[2 * a + 0];
				rhs[2 * idxn[a] + 1] += fe[2 * a + 1];
			}
		}
	}

	for (unsigned int i = 0; i < rhs.size(); i++) {
		if (constrained[i]) rhs[i] = 0.;
	}

	auto matvec = [&](const std::vector<double> &x, std::vector<double> &y) {
		y.assign(x.size(), 0.);
		for (unsigned int i = 0; i < m_Km_rows.size(); i++) {
			if (constrained[i]) {
				y[i] = x[i];
				continue;
			}
			double s = 0.;
			for (const auto &kv : m_Km_rows[i]) s += kv.second * x[kv.first];
			y[i] = s;
		}
	};

	auto dot = [&](const std::vector<double> &a, const std::vector<double> &b) {
		double s = 0.;
		for (unsigned int i = 0; i < a.size(); i++) s += a[i] * b[i];
		return s;
	};

	auto norm = [&](const std::vector<double> &a) { return std::sqrt(dot(a, a)); };

	std::vector<double> x(2 * m_nodes_tool.size(), 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size() && i < m_u.size(); i++) {
		x[2 * i + 0] = m_u[i].x;
		x[2 * i + 1] = m_u[i].y;
	}
	for (unsigned int i = 0; i < x.size(); i++) if (constrained[i]) x[i] = 0.;

	std::vector<double> Ax;
	matvec(x, Ax);
	std::vector<double> r(x.size(), 0.);
	for (unsigned int i = 0; i < x.size(); i++) r[i] = rhs[i] - Ax[i];
	for (unsigned int i = 0; i < r.size(); i++) if (constrained[i]) r[i] = 0.;

	double rhs_norm = norm(rhs);
	if (!(rhs_norm > 0.)) rhs_norm = 1.0;
	double r_norm0 = norm(r);
	if (r_norm0 / rhs_norm <= rel_tol) return;

	std::vector<double> p = r;
	std::vector<double> Ap;
	double rr = dot(r, r);

	for (unsigned int it = 0; it < max_iters; it++) {
		matvec(p, Ap);
		double pAp = dot(p, Ap);
		if (!(pAp > 0.)) break;
		double alpha = rr / pAp;
		for (unsigned int i = 0; i < x.size(); i++) x[i] += alpha * p[i];
		for (unsigned int i = 0; i < r.size(); i++) r[i] -= alpha * Ap[i];
		for (unsigned int i = 0; i < r.size(); i++) if (constrained[i]) r[i] = 0.;
		double rr_new = dot(r, r);
		double rel = std::sqrt(rr_new) / rhs_norm;
		if (rel <= rel_tol) break;
		double beta = rr_new / rr;
		for (unsigned int i = 0; i < p.size(); i++) p[i] = r[i] + beta * p[i];
		rr = rr_new;
	}

	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		m_u[i].x = x[2 * i + 0];
		m_u[i].y = x[2 * i + 1];
	}
}

double fe_tool::max_displacement_norm() const {
	double mx = 0.;
	for (const auto &u : m_u) mx = std::max(mx, glm::length(u));
	return mx;
}

std::pair<unsigned int, double> fe_tool::nearest_boundary_edge_barycentric(glm::dvec2 x_tool) const {
	double best_d2 = std::numeric_limits<double>::infinity();
	unsigned int best_e = static_cast<unsigned int>(m_bnd.size());
	double best_t = 0.;

	for (unsigned int ei = 0; ei < m_bnd.size(); ei++) {
		const boundary_edge &e = m_bnd[ei];
		glm::dvec2 a = m_nodes_tool[e.n0] + (e.n0 < m_u.size() ? m_u[e.n0] : glm::dvec2(0.));
		glm::dvec2 b = m_nodes_tool[e.n1] + (e.n1 < m_u.size() ? m_u[e.n1] : glm::dvec2(0.));
		glm::dvec2 ab = b - a;
		double ab2 = glm::dot(ab, ab);
		if (ab2 <= 0.) continue;
		double t = glm::dot(x_tool - a, ab) / ab2;
		t = std::max(0.0, std::min(1.0, t));
		glm::dvec2 p = a + t * ab;
		glm::dvec2 d = x_tool - p;
		double d2 = glm::dot(d, d);
		if (d2 < best_d2) {
			best_d2 = d2;
			best_e = ei;
			best_t = t;
		}
	}

	return {best_e, best_t};
}
