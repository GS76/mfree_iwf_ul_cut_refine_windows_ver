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

#ifndef FE_TOOL_H_
#define FE_TOOL_H_

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>

class fe_tool {
public:
	struct thermal_material {
		double rho = 0.;
		double cp = 0.;
		double k = 0.;
	};

	struct mechanical_material {
		double E = 0.;     // Pa - Young's modulus
		double nu = 0.;    // - - Poisson's ratio
		double alpha = 0.; // 1/K - thermal expansion coefficient
	};

	struct boundary_edge {
		unsigned int n0 = 0;
		unsigned int n1 = 0;
		int physical_tag = 0;
	};

	struct convection_bc {
		double h = 0.;
		double T_inf = 0.;
	};

	bool load_gmsh_msh2(const std::string &path);
	void set_mesh(const std::vector<glm::dvec2> &nodes_tool_frame,
	              const std::vector<std::array<unsigned int, 3>> &triangles,
	              const std::vector<boundary_edge> &boundary_edges);

	void set_material(thermal_material mat);
	thermal_material get_material() const;

	void set_mechanical_material(mechanical_material mat);
	mechanical_material get_mechanical_material() const;
	void set_reference_temperature(double T_ref);
	double reference_temperature() const;
	void set_mechanics_fixed_on_physical(int physical_tag);
	void clear_mechanics_fixed();
	void set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes);
	void clear_mechanics_fixed_nodes();

	void set_initial_temperature(double T0);

	void set_pose(glm::dvec2 pos, glm::dvec2 vel);
	glm::dvec2 get_pos() const;
	glm::dvec2 get_vel() const;
	void update_pose(double dt);

	glm::dvec2 to_tool_frame(glm::dvec2 x_world) const;
	glm::dvec2 to_world_frame(glm::dvec2 x_tool) const;

	const std::vector<glm::dvec2> &nodes_tool_frame() const;
	const std::vector<std::array<unsigned int, 3>> &triangles() const;
	const std::vector<boundary_edge> &boundary_edges() const;

	double temperature_at_node(unsigned int i) const;
	double temperature_at_world_point_nearest_boundary(glm::dvec2 x_world) const;

	void clear_sources();
	void add_nodal_power(unsigned int node, double power);
	void add_boundary_point_power(glm::dvec2 x_world, double power);
	double nodal_power(unsigned int node) const;

	void clear_forces();
	void add_nodal_force(unsigned int node, glm::dvec2 force);
	void add_boundary_point_force(glm::dvec2 x_world, glm::dvec2 force);
	glm::dvec2 nodal_force(unsigned int node) const;

	glm::dvec2 node_world(unsigned int i) const;
	const std::vector<unsigned int> &boundary_loop_nodes() const;
	std::vector<glm::dvec2> boundary_loop_world() const;
	const std::vector<glm::dvec2> &displacements() const;
	void set_displacements(const std::vector<glm::dvec2> &u);

	void set_convection_on_physical(int physical_tag, convection_bc bc);
	void set_dirichlet_on_physical(int physical_tag, double T);
	void set_convection_air_all_exposed(convection_bc air_bc);
	void set_convection_flooded_by_y(convection_bc air_bc, convection_bc water_bc, double y_threshold_world);

	double max_temperature() const;
	double min_temperature() const;

	void advance_explicit(double dt);
	void solve_mechanics_quasistatic(unsigned int max_iters, double rel_tol);
	double max_displacement_norm() const;

	struct contact_convergence {
		unsigned int iters = 0;
		double rel_force = 0.;
		double rel_power = 0.;
		double max_rel_force_node = 0.;
		double max_rel_power_node = 0.;
		unsigned int nodes_force_over_tol = 0;
		unsigned int nodes_power_over_tol = 0;
	};
	void set_contact_convergence(contact_convergence c);
	contact_convergence get_contact_convergence() const;

	struct contact_energy_balance {
		double P_fric = 0.;
		double P_cond = 0.;
		double scale = 1.;
		double frac_workpiece = 0.;
		double frac_tool = 0.;
	};
	void set_contact_energy_balance(contact_energy_balance b);
	contact_energy_balance get_contact_energy_balance() const;

	fe_tool();

private:
	struct edge_key {
		unsigned int a = 0;
		unsigned int b = 0;
		bool operator==(const edge_key &o) const { return a == o.a && b == o.b; }
	};

	struct edge_key_hash {
		std::size_t operator()(const edge_key &k) const noexcept {
			return (static_cast<std::size_t>(k.a) << 32) ^ static_cast<std::size_t>(k.b);
		}
	};

	void build_conduction_operator();
	void build_boundary_edges_from_lines();
	void build_boundary_edge_to_adjacent_triangle();
	void build_boundary_loop();
	void build_mechanics_operator();

	std::pair<unsigned int, double> nearest_boundary_edge_barycentric(glm::dvec2 x_tool) const;

	thermal_material m_mat;
	mechanical_material m_mech;
	double m_T_ref = 0.;

	glm::dvec2 m_pos = glm::dvec2(0.);
	glm::dvec2 m_vel = glm::dvec2(0.);

	std::vector<glm::dvec2> m_nodes_tool;
	std::vector<std::array<unsigned int, 3>> m_tris;
	std::vector<boundary_edge> m_bnd;

	std::unordered_map<int, convection_bc> m_conv_by_tag;
	std::unordered_map<int, double> m_dirichlet_by_tag;
	convection_bc m_air_all;
	bool m_use_air_all = false;
	bool m_use_flooded_by_y = false;
	convection_bc m_flood_air;
	convection_bc m_flood_water;
	double m_flood_y_threshold_world = 0.;

	std::vector<double> m_T;
	std::vector<double> m_capacity;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_K_rows;
	std::vector<double> m_power_sources;
	std::vector<glm::dvec2> m_force_sources;
	std::vector<glm::dvec2> m_u;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_Km_rows;
	std::unordered_set<int> m_mech_fix_tags;
	std::unordered_set<unsigned int> m_mech_fix_nodes;
	std::vector<unsigned int> m_boundary_loop;
	contact_convergence m_contact_conv;
	contact_energy_balance m_contact_energy;

	std::unordered_map<edge_key, unsigned int, edge_key_hash> m_bnd_edge_to_tri;

	std::vector<boundary_edge> m_line_elements;
};

#endif
