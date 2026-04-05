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

	build_boundary_edge_to_adjacent_triangle();
	build_conduction_operator();
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

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_power_sources.assign(m_nodes_tool.size(), 0.);

	build_conduction_operator();

	return true;
}

void fe_tool::set_material(thermal_material mat) {
	m_mat = mat;
	build_conduction_operator();
}

fe_tool::thermal_material fe_tool::get_material() const {
	return m_mat;
}

void fe_tool::set_initial_temperature(double T0) {
	for (std::size_t i = 0; i < m_T.size(); i++) m_T[i] = T0;
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

void fe_tool::set_convection_on_physical(int physical_tag, convection_bc bc) {
	m_conv_by_tag[physical_tag] = bc;
}

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
	m_bnd = m_line_elements;
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

std::pair<unsigned int, double> fe_tool::nearest_boundary_edge_barycentric(glm::dvec2 x_tool) const {
	double best_d2 = std::numeric_limits<double>::infinity();
	unsigned int best_e = static_cast<unsigned int>(m_bnd.size());
	double best_t = 0.;

	for (unsigned int ei = 0; ei < m_bnd.size(); ei++) {
		const boundary_edge &e = m_bnd[ei];
		glm::dvec2 a = m_nodes_tool[e.n0];
		glm::dvec2 b = m_nodes_tool[e.n1];
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
