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

#include "glm/glm.hpp"

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>

class fe_tool {
public:
	struct bbox {
		double bbmin_x = 0.;
		double bbmax_x = 0.;
		double bbmin_y = 0.;
		double bbmax_y = 0.;

		bool in(glm::dvec2 qp);
		bool valid() const;

		bbox();
		bbox(glm::dvec2 p1, glm::dvec2 p2);
		bbox(double bbmin_x, double bbmax_x, double bbmin_y, double bbmax_y);
	};

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
	void set_material_table_rho(std::vector<double> T, std::vector<double> rho);
	void set_material_table_cp(std::vector<double> T, std::vector<double> cp);
	void set_material_table_k(std::vector<double> T, std::vector<double> k);

	void set_mechanical_material(mechanical_material mat);
	mechanical_material get_mechanical_material() const;
	void set_mechanical_table_E(std::vector<double> T, std::vector<double> E);
	void set_mechanical_table_nu(std::vector<double> T, std::vector<double> nu);
	void set_mechanical_table_alpha(std::vector<double> T, std::vector<double> alpha);
	void set_reference_temperature(double T_ref);
	double reference_temperature() const;
	void set_mechanics_fixed_on_physical(int physical_tag);
	/**
	 * @brief Constrain the X displacement DOF (UX) of all boundary nodes that belong to a given physical tag.
	 * @param physical_tag Gmsh physical tag of boundary line elements.
	 */
	void set_mechanics_fixed_x_on_physical(int physical_tag);
	/**
	 * @brief Constrain the Y displacement DOF (UY) of all boundary nodes that belong to a given physical tag.
	 * @param physical_tag Gmsh physical tag of boundary line elements.
	 */
	void set_mechanics_fixed_y_on_physical(int physical_tag);
	void clear_mechanics_fixed();
	void set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes);
	/**
	 * @brief Constrain the X displacement DOF (UX) for an explicit list of node indices (0-based).
	 * @param nodes Node indices in the tool mesh (0-based).
	 */
	void set_mechanics_fixed_x_nodes(const std::vector<unsigned int> &nodes);
	/**
	 * @brief Constrain the Y displacement DOF (UY) for an explicit list of node indices (0-based).
	 * @param nodes Node indices in the tool mesh (0-based).
	 */
	void set_mechanics_fixed_y_nodes(const std::vector<unsigned int> &nodes);
	void clear_mechanics_fixed_nodes();

	/**
	 * @brief Query whether a node has its X displacement DOF (UX) constrained by the current mechanics constraints.
	 * @param node Node index in the tool mesh (0-based).
	 * @return true if UX is fixed, otherwise false.
	 */
	bool is_mechanics_fixed_x(unsigned int node) const;
	/**
	 * @brief Query whether a node has its Y displacement DOF (UY) constrained by the current mechanics constraints.
	 * @param node Node index in the tool mesh (0-based).
	 * @return true if UY is fixed, otherwise false.
	 */
	bool is_mechanics_fixed_y(unsigned int node) const;

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

	void set_mu(double mu);
	double get_mu() const;

	fe_tool::bbox get_bbox_world() const;
	glm::dvec2 get_edge_coord() const;

	// returns distance from qp to tool if qp is inside tool
	// returns -1 otherwise
	double inside(glm::dvec2 qp) const;

	void advance_explicit(double dt);
	void set_mechanics_rayleigh(double a0, double a1);
	void advance_mechanics_explicit(double dt);
	double mechanics_dt_crit() const;
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

	struct thermal_energy_accounting {
		double step_dt = 0.;
		double step_contact_E_cond_raw = 0.;
		double step_contact_E_fric_raw = 0.;
		double step_contact_E_cond_scaled = 0.;
		double step_contact_E_fric_scaled = 0.;
		double step_contact_E_workpiece = 0.;
		double step_contact_E_tool = 0.;
		double step_contact_E_limiter_suppressed = 0.;
		double step_tool_E_sources = 0.;
		double step_tool_E_conduction = 0.;
		double step_tool_E_convection = 0.;
		double step_tool_E_dirichlet = 0.;
		double tool_internal_E = 0.;
		double cumulative_contact_E_cond_raw = 0.;
		double cumulative_contact_E_fric_raw = 0.;
		double cumulative_contact_E_cond_scaled = 0.;
		double cumulative_contact_E_fric_scaled = 0.;
		double cumulative_contact_E_workpiece = 0.;
		double cumulative_contact_E_tool = 0.;
		double cumulative_contact_E_limiter_suppressed = 0.;
		double cumulative_tool_E_sources = 0.;
		double cumulative_tool_E_conduction = 0.;
		double cumulative_tool_E_convection = 0.;
		double cumulative_tool_E_dirichlet = 0.;
	};
	void reset_thermal_energy_accounting_step(double dt);
	void add_contact_energy_accounting(double dt, double P_cond_raw, double P_fric_raw, double scale, double frac_workpiece, double frac_tool);
	thermal_energy_accounting get_thermal_energy_accounting() const;
	double thermal_internal_energy() const;
	double min_thermal_nodal_capacity() const;

	fe_tool();
	virtual ~fe_tool() = default;

	double thermal_dt_crit() const;

private:
	double m_mu = 0.0;

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
	void build_conduction_operator_from_temperature();
	void build_boundary_edges_from_lines();
	void build_boundary_edge_to_adjacent_triangle();
	void build_boundary_loop();
	void build_mechanics_operator();
	void build_mechanics_operator_from_temperature();
	void apply_dirichlet_bc(std::vector<char> &is_fixed);
	void build_mech_constrained(std::vector<char> &constrained) const;
	void ensure_mech_fix_cache() const;
	void add_thermoelastic_rhs(std::vector<double> &rhs) const;
	void matvec_mechanics(const std::vector<char> &constrained, const std::vector<double> &x, std::vector<double> &y) const;
	void ensure_mechanics_lumped_mass();
	static double table_eval(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback);
	double rho_at(double T) const;
	double cp_at(double T) const;
	double k_at(double T) const;
	double E_at(double T) const;
	double nu_at(double T) const;
	double alpha_at(double T) const;

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
	std::vector<double> m_rho_T;
	std::vector<double> m_rho_val;
	std::vector<double> m_cp_T;
	std::vector<double> m_cp_val;
	std::vector<double> m_k_T;
	std::vector<double> m_k_val;
	std::vector<double> m_E_T;
	std::vector<double> m_E_val;
	std::vector<double> m_nu_T;
	std::vector<double> m_nu_val;
	std::vector<double> m_alpha_T;
	std::vector<double> m_alpha_val;

	std::vector<double> m_T;
	std::vector<double> m_capacity;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_K_rows;
	std::vector<double> m_power_sources;
	std::vector<glm::dvec2> m_force_sources;
	std::vector<glm::dvec2> m_u;
	std::vector<double> m_mech_mass;
	std::vector<double> m_mech_v_half;
	double m_mech_rayleigh_a0 = 0.;
	double m_mech_rayleigh_a1 = 0.;
	bool m_mech_mass_scaled = false;
	bool m_mech_v_half_initialized = false;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_Km_rows;
	std::unordered_set<int> m_mech_fix_tags;
	std::unordered_set<unsigned int> m_mech_fix_nodes;
	std::unordered_set<int> m_mech_fix_x_tags;
	std::unordered_set<int> m_mech_fix_y_tags;
	std::unordered_set<unsigned int> m_mech_fix_x_nodes;
	std::unordered_set<unsigned int> m_mech_fix_y_nodes;
	mutable bool m_mech_fix_cache_valid = false;
	mutable std::unordered_set<unsigned int> m_mech_fix_cache_x_nodes;
	mutable std::unordered_set<unsigned int> m_mech_fix_cache_y_nodes;
	std::vector<unsigned int> m_boundary_loop;
	contact_convergence m_contact_conv;
	contact_energy_balance m_contact_energy;
	thermal_energy_accounting m_thermal_energy;

	std::unordered_map<edge_key, unsigned int, edge_key_hash> m_bnd_edge_to_tri;

	std::vector<boundary_edge> m_line_elements;
};

#endif
