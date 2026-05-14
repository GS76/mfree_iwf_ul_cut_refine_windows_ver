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

#include "body.h"

#include "contact_iface.h"
#include "fe_tool.h"
#include "simulation_time.h"
#include "tool_adapter_poly.h"
#include <cerrno>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
static bool parse_env_bool_strict(const char *name) {
	if (!name || name[0] == '\0')
		return false;
	const char *s = std::getenv(name);
	if (!s)
		return false;

	errno = 0;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected integer 0/1\n", name, s);
		return false;
	}
	return v != 0;
}

static bool parse_env_uint_strict_min(const char *name, unsigned int min_value, unsigned int &out) {
	if (!name || name[0] == '\0')
		return false;
	const char *s = std::getenv(name);
	if (!s)
		return false;

	errno = 0;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok || v < 0) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected integer >= %u\n", name, s, min_value);
		return false;
	}
	if (static_cast<unsigned long>(v) < static_cast<unsigned long>(min_value)) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected integer >= %u\n", name, s, min_value);
		return false;
	}
	out = static_cast<unsigned int>(v);
	return true;
}

static bool parse_env_double_strict_range(const char *name, double min_value, double max_value, double &out) {
	if (!name || name[0] == '\0')
		return false;
	const char *s = std::getenv(name);
	if (!s)
		return false;

	errno = 0;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok || !std::isfinite(v) || v < min_value || v > max_value) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected finite number in [%.6g, %.6g]\n", name, s, min_value, max_value);
		return false;
	}
	out = v;
	return true;
}

static bool parse_env_double_strict_min(const char *name, double min_value, double &out) {
	if (!name || name[0] == '\0')
		return false;
	const char *s = std::getenv(name);
	if (!s)
		return false;

	errno = 0;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok || !std::isfinite(v) || v < min_value) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected finite number >= %.6g\n", name, s, min_value);
		return false;
	}
	out = v;
	return true;
}
} // namespace

void body::apply_plasticity() {
	if (m_plast == 0) {
		m_step_plastic_dissipation = 0.;
		return;
	}
	m_step_plastic_dissipation = m_plast->plastic_state_by_radial_return(*this);
}

double body::get_step_plastic_dissipation() const { return m_step_plastic_dissipation; }

void body::apply_thermal_conduction() {
	if (m_thermal == 0)
		return;
	m_thermal->conduction(*this);
}

void body::apply_contact() {
	if (m_fe_tool == nullptr)
		return;

	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	m_fe_tool->reset_thermal_energy_accounting_step(dt);

	double mu = m_fe_tool->get_mu();
	glm::dvec2 v_master = m_fe_tool->get_vel();
	parse_env_double_strict_min("MFREE_CONTACT_MU", 0.0, mu);

	bool deformable = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL");

	if (!deformable) {
		m_fe_tool->clear_sources();
		m_fe_tool->clear_forces();
		std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
		if (poly.size() >= 3) {
			poly_tool_contact_adapter tpoly(poly, mu, v_master);
			contact_apply_master_to_body_2d(tpoly, *this, m_fe_tool, dt);
		}
		return;
	}

	unsigned int max_contact_iters = 20;
	unsigned int mech_cg_iters = 4000;
	double contact_tol = 0.01;
	double mech_rel_tol = 1e-6;
	double relax = 0.2;
	bool explicit_coupled = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL_EXPLICIT");
	unsigned int explicit_max_substeps = 100;
	unsigned int explicit_substeps_override = 0;
	unsigned int thermal_substeps_override = 0;

	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_MAX_ITERS", 1u, max_contact_iters);
	parse_env_double_strict_min("MFREE_DEFORMABLE_TOOL_TOL", 0.0, contact_tol);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_MECH_CG_ITERS", 100u, mech_cg_iters);
	parse_env_double_strict_min("MFREE_DEFORMABLE_TOOL_MECH_REL_TOL", 0.0, mech_rel_tol);
	parse_env_double_strict_range("MFREE_DEFORMABLE_TOOL_RELAX", 0.0, 1.0, relax);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS", 1u, explicit_max_substeps);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS", 1u, explicit_substeps_override);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS", 1u, thermal_substeps_override);

	std::vector<particle> &particles = get_particles();
	std::vector<double> base_T_t(particles.size(), 0.);
	for (unsigned int i = 0; i < particles.size(); i++)
		base_T_t[i] = particles[i].T_t;

	const auto &nodes = m_fe_tool->nodes_tool_frame();
	std::vector<glm::dvec2> prev_forces(nodes.size(), glm::dvec2(0.));
	std::vector<double> prev_powers(nodes.size(), 0.);

	if (explicit_coupled) {
		double a0 = 0.;
		double a1 = 0.;
		parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A0", 0.0, a0);
		parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A1", 0.0, a1);
		m_fe_tool->set_mechanics_rayleigh(a0, a1);

		unsigned int mech_substeps = 1;
		double dtcrit = m_fe_tool->mechanics_dt_crit();
		if (explicit_substeps_override > 0) {
			mech_substeps = explicit_substeps_override;
		} else if (std::isfinite(dtcrit) && dtcrit > 0.) {
			double max_dt = 0.9 * dtcrit;
			mech_substeps = static_cast<unsigned int>(std::ceil(dt / max_dt));
			if (mech_substeps < 1)
				mech_substeps = 1;
		}
		if (mech_substeps > explicit_max_substeps)
			mech_substeps = explicit_max_substeps;
		unsigned int thermal_substeps = (thermal_substeps_override > 0) ? thermal_substeps_override : mech_substeps;
		unsigned int substeps = std::max(mech_substeps, thermal_substeps);
		if (substeps < 1)
			substeps = 1;

		std::vector<double> sum_fcx(particles.size(), 0.);
		std::vector<double> sum_fcy(particles.size(), 0.);
		std::vector<double> sum_ftx(particles.size(), 0.);
		std::vector<double> sum_fty(particles.size(), 0.);
		std::vector<double> sum_dTt(particles.size(), 0.);

		for (unsigned int s = 0; s < substeps; s++) {
			for (unsigned int i = 0; i < particles.size(); i++) {
				particles[i].fcx = 0.;
				particles[i].fcy = 0.;
				particles[i].ftx = 0.;
				particles[i].fty = 0.;
				particles[i].T_t = base_T_t[i];
			}
			m_fe_tool->clear_sources();
			m_fe_tool->clear_forces();

			std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
			{
				std::vector<glm::dvec2> uniq;
				uniq.reserve(poly.size());
				const double eps2 = 1e-24;
				for (const auto &p : poly) {
					if (!uniq.empty()) {
						glm::dvec2 d = p - uniq.back();
						if (d.x * d.x + d.y * d.y <= eps2)
							continue;
					}
					uniq.push_back(p);
				}
				if (uniq.size() >= 2) {
					glm::dvec2 d = uniq.front() - uniq.back();
					if (d.x * d.x + d.y * d.y <= eps2)
						uniq.pop_back();
				}
				poly.swap(uniq);
			}

			if (poly.size() >= 3) {
				poly_tool_contact_adapter tpoly(poly, mu, v_master);
				contact_apply_master_to_body_2d(tpoly, *this, m_fe_tool, dt / static_cast<double>(substeps));
			}

			double dt_th = dt / static_cast<double>(thermal_substeps);
			if (s < thermal_substeps)
				m_fe_tool->advance_explicit(dt_th);

			double dt_mech = dt / static_cast<double>(mech_substeps);
			if (s < mech_substeps)
				m_fe_tool->advance_mechanics_explicit(dt_mech);

			for (unsigned int i = 0; i < particles.size(); i++) {
				sum_fcx[i] += particles[i].fcx;
				sum_fcy[i] += particles[i].fcy;
				sum_ftx[i] += particles[i].ftx;
				sum_fty[i] += particles[i].fty;
				sum_dTt[i] += (particles[i].T_t - base_T_t[i]);
			}
		}

		double inv = 1.0 / static_cast<double>(substeps);
		for (unsigned int i = 0; i < particles.size(); i++) {
			particles[i].fcx = sum_fcx[i] * inv;
			particles[i].fcy = sum_fcy[i] * inv;
			particles[i].ftx = sum_ftx[i] * inv;
			particles[i].fty = sum_fty[i] * inv;
			particles[i].T_t = base_T_t[i] + sum_dTt[i] * inv;
		}

		fe_tool::contact_convergence cc;
		cc.iters = substeps;
		cc.rel_force = 0.;
		cc.rel_power = 0.;
		cc.max_rel_force_node = 0.;
		cc.max_rel_power_node = 0.;
		cc.nodes_force_over_tol = 0;
		cc.nodes_power_over_tol = 0;
		m_fe_tool->set_contact_convergence(cc);
		return;
	}

	for (unsigned int it = 0; it < max_contact_iters; it++) {
		for (unsigned int i = 0; i < particles.size(); i++) {
			particles[i].fcx = 0.;
			particles[i].fcy = 0.;
			particles[i].ftx = 0.;
			particles[i].fty = 0.;
			particles[i].T_t = base_T_t[i];
		}
		m_fe_tool->clear_sources();
		m_fe_tool->clear_forces();

		std::vector<glm::dvec2> u_old = m_fe_tool->displacements();

		std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
		if (poly.size() < 3) {
			break;
		}

		poly_tool_contact_adapter tpoly(poly, mu, v_master);
		m_fe_tool->reset_thermal_energy_accounting_step(dt);
		contact_apply_master_to_body_2d(tpoly, *this, m_fe_tool, dt);
		m_fe_tool->solve_mechanics_quasistatic(mech_cg_iters, mech_rel_tol);
		if (relax < 1.0) {
			std::vector<glm::dvec2> u_new = m_fe_tool->displacements();
			if (u_new.size() == u_old.size()) {
				for (unsigned int i = 0; i < u_new.size(); i++)
					u_new[i] = (1.0 - relax) * u_old[i] + relax * u_new[i];
				m_fe_tool->set_displacements(u_new);
			}
		}

		double df2 = 0.;
		double f2 = 0.;
		double dp2 = 0.;
		double p2 = 0.;
		double max_rF_node = 0.;
		double max_rP_node = 0.;
		unsigned int cnt_rF_over = 0;
		unsigned int cnt_rP_over = 0;

		for (unsigned int i = 0; i < nodes.size(); i++) {
			glm::dvec2 f = m_fe_tool->nodal_force(i);
			double p = m_fe_tool->nodal_power(i);

			glm::dvec2 df = f - prev_forces[i];
			double dp = p - prev_powers[i];

			df2 += glm::dot(df, df);
			f2 += glm::dot(f, f);
			dp2 += dp * dp;
			p2 += p * p;

			double f_norm = glm::length(f);
			double f_prev_norm = glm::length(prev_forces[i]);
			double p_norm = std::abs(p);
			double p_prev_norm = std::abs(prev_powers[i]);

			double denom_f = std::max(1e-30, std::max(f_norm, f_prev_norm));
			double denom_p = std::max(1e-30, std::max(p_norm, p_prev_norm));

			double rF_node = glm::length(df) / denom_f;
			double rP_node = std::abs(dp) / denom_p;

			bool active = (f_norm > 1e-30) || (f_prev_norm > 1e-30) || (p_norm > 1e-30) || (p_prev_norm > 1e-30);
			if (active) {
				if (std::isfinite(rF_node))
					max_rF_node = std::max(max_rF_node, rF_node);
				if (std::isfinite(rP_node))
					max_rP_node = std::max(max_rP_node, rP_node);
				if (it > 0 && std::isfinite(rF_node) && rF_node > contact_tol)
					cnt_rF_over++;
				if (it > 0 && std::isfinite(rP_node) && rP_node > contact_tol)
					cnt_rP_over++;
			}

			prev_forces[i] = f;
			prev_powers[i] = p;
		}

		double rF = std::sqrt(df2) / std::max(1e-30, std::sqrt(f2));
		double rP = std::sqrt(dp2) / std::max(1e-30, std::sqrt(p2));
		fe_tool::contact_convergence cc;
		cc.iters = it + 1;
		cc.rel_force = rF;
		cc.rel_power = rP;
		cc.max_rel_force_node = max_rF_node;
		cc.max_rel_power_node = max_rP_node;
		cc.nodes_force_over_tol = cnt_rF_over;
		cc.nodes_power_over_tol = cnt_rP_over;
		m_fe_tool->set_contact_convergence(cc);
		if (it > 0 && max_rF_node <= contact_tol && max_rP_node <= contact_tol)
			break;
	}
}

void body::advance_fe_tool_thermal() {
	if (!m_fe_tool)
		return;
	bool deformable = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL");
	bool coupled_explicit = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL_EXPLICIT");
	if (deformable && coupled_explicit)
		return;
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	m_fe_tool->advance_explicit(dt);
}

void body::advance_fe_tool_mechanics_explicit() {
	if (!m_fe_tool)
		return;
	bool deformable = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL");
	bool coupled_explicit = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL_EXPLICIT");
	if (deformable && coupled_explicit)
		return;
	bool use = parse_env_bool_strict("MFREE_FE_TOOL_MECH_EXPLICIT");
	if (!use)
		return;
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	double a0 = 0.;
	double a1 = 0.;
	parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A0", 0.0, a0);
	parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A1", 0.0, a1);
	m_fe_tool->set_mechanics_rayleigh(a0, a1);
	m_fe_tool->advance_mechanics_explicit(dt);
}

void body::apply_adaptivity() {
	if (m_adapt == 0)
		return;
	m_adapt->adapt_resolution(*this);
}

void body::set_fe_tool(fe_tool *tool) { m_fe_tool = tool; }

void body::move_tool() {
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	if (m_fe_tool)
		m_fe_tool->update_pose(dt);
}

glm::dvec2 body::speed_tool() {
	if (m_fe_tool)
		return m_fe_tool->get_vel();
	return glm::dvec2(0.);
}

glm::dvec2 body::edge_tool() {
	if (m_fe_tool) {
		std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
		if (!poly.empty()) {
			glm::dvec2 best = poly[0];
			for (const auto &p : poly) {
				if (p.y < best.y)
					best = p;
			}
			return best;
		}
	}
	return glm::dvec2(0.);
}

const fe_tool *body::get_fe_tool() const { return m_fe_tool; }
fe_tool *body::get_fe_tool() { return m_fe_tool; }

void body::set_plasticity(plasticity *plasticity) { m_plast = plasticity; }

void body::set_thermal(thermal *thermal) { m_thermal = thermal; }

void body::set_adaptivity(adaptivity *adaptivity) { m_adapt = adaptivity; }

void body::construct_verlet_lists() {
	const unsigned int num_part = m_particles.size();

	m_grid.update_geometry(m_particles, num_part, 2.);
	m_grid.assign_hashes(m_particles, num_part);

	std::sort(m_particles.begin(), m_particles.end(), [](const particle &a, const particle &b) { return a.hash < b.hash; });

	m_grid.construct_verlet_lists(m_particles, num_part, 2.);

	m_basis_fun(m_particles, num_part);
}

void body::insert_particles(const std::vector<particle> &additional_particles) {
	m_particles.insert(m_particles.end(), additional_particles.begin(), additional_particles.end());
}

void body::restore_order() {
	std::sort(m_particles.begin(), m_particles.end(), [](const particle &a, const particle &b) { return a.idx < b.idx; });
}

void body::set_basis_fun(void (*basis_fun)(std::vector<particle> &particles, unsigned int)) { m_basis_fun = basis_fun; }

simulation_data body::get_sim_data() const { return m_simulation_data; }

std::vector<particle> &body::get_particles() { return m_particles; }

const std::vector<particle> &body::get_particles() const { return m_particles; }

unsigned int body::get_num_part() const { return m_particles.size(); }

body::body(particle *particles, unsigned int n, simulation_data data) : m_simulation_data(data) {

	m_particles.resize(n);
	std::copy(particles, particles + n, m_particles.begin());
}

body::body() {}
