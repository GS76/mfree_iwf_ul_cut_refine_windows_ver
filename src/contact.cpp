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

#include "contact.h"

#include "contact_iface.h"
#include "fe_tool.h"
#include "tool_iface.h"
#include "particle.h"
#include "body.h"

#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {
struct thermal_contact_coupling_params {
	double h_separated_W_m2K = 1000.0;
	double h_full_contact_W_m2K = 100000.0;
	double p_ref_Pa = 1.0e9;
	double friction_heat_fraction_workpiece = 0.8;
	double friction_heat_fraction_tool = 0.2;
	double max_dT_per_step_K = 1.0;
	double plane_strain_thickness_m = 1.0;
	double contact_length_factor = 1.0;
};

struct contact_penalty_params {
	double alpha0 = 0.1;
	double alpha_min = 1.0e-4;
	double alpha_max = 10.0;
	double pen_depth_ref_m = 1.0e-6;
	bool adaptive = false;
	bool use_lagrange_multiplier = false;
};

enum class env_double_status { not_set, ok, invalid };

static void warn_invalid_env_double(const char *key, const char *value) {
	std::fprintf(stderr, "WARNING: invalid value for %s: '%s' (ignored)\n", key, value ? value : "");
}

static env_double_status read_env_double(const char *key, double &out, const char **raw) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0')
		return env_double_status::not_set;
	if (raw)
		*raw = s;
	char *end = nullptr;
	errno = 0;
	double v = std::strtod(s, &end);
	if (end == s || errno != 0 || !std::isfinite(v))
		return env_double_status::invalid;
	while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')
		++end;
	if (*end != '\0')
		return env_double_status::invalid;
	out = v;
	return env_double_status::ok;
}

static thermal_contact_coupling_params load_thermal_contact_coupling_params() {
	thermal_contact_coupling_params p;

	{
		double v = 0.;
		const char *raw = nullptr;

		switch (read_env_double("MFREE_THERMAL_H_SEP", v, &raw)) {
		case env_double_status::ok:
			if (v >= 0.)
				p.h_separated_W_m2K = v;
			else
				warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_H_FULL", v, &raw)) {
		case env_double_status::ok:
			if (v >= 0.)
				p.h_full_contact_W_m2K = v;
			else
				warn_invalid_env_double("MFREE_THERMAL_H_FULL", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_H_FULL", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_P_REF", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.p_ref_Pa = v;
			else
				warn_invalid_env_double("MFREE_THERMAL_P_REF", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_P_REF", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.max_dT_per_step_K = v;
			else
				warn_invalid_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_PLANE_STRAIN_THICKNESS", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.plane_strain_thickness_m = v;
			else
				warn_invalid_env_double("MFREE_PLANE_STRAIN_THICKNESS", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_PLANE_STRAIN_THICKNESS", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_CONTACT_LENGTH_FACTOR", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.contact_length_factor = v;
			else
				warn_invalid_env_double("MFREE_THERMAL_CONTACT_LENGTH_FACTOR", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_CONTACT_LENGTH_FACTOR", raw);
			break;
		default:
			break;
		}
	}

	{
		double frac_wp = 0.;
		double frac_tool = 0.;
		const char *raw_wp = nullptr;
		const char *raw_tool = nullptr;
		env_double_status st_wp = read_env_double("MFREE_THERMAL_FRAC_WP", frac_wp, &raw_wp);
		env_double_status st_tool = read_env_double("MFREE_THERMAL_FRAC_TOOL", frac_tool, &raw_tool);

		if (st_wp == env_double_status::invalid)
			warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
		if (st_tool == env_double_status::invalid)
			warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);

		bool has_wp = (st_wp == env_double_status::ok);
		bool has_tool = (st_tool == env_double_status::ok);

		if (has_wp && !has_tool)
			frac_tool = 1.0 - frac_wp;
		if (has_tool && !has_wp)
			frac_wp = 1.0 - frac_tool;

		if (has_wp || has_tool) {
			if (!std::isfinite(frac_wp) || !std::isfinite(frac_tool)) {
				if (has_wp)
					warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
				if (has_tool)
					warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			}
			if (has_wp && (frac_wp < 0.0 || frac_wp > 1.0))
				warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
			if (has_tool && (frac_tool < 0.0 || frac_tool > 1.0))
				warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			frac_wp = std::max(0.0, std::min(1.0, frac_wp));
			frac_tool = std::max(0.0, std::min(1.0, frac_tool));
			double s = frac_wp + frac_tool;
			if (s > 0.) {
				p.friction_heat_fraction_workpiece = frac_wp / s;
				p.friction_heat_fraction_tool = frac_tool / s;
			} else {
				if (has_wp)
					warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
				if (has_tool)
					warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			}
		}
	}

	if (!std::isfinite(p.h_separated_W_m2K) || p.h_separated_W_m2K < 0.)
		p.h_separated_W_m2K = 1000.0;
	if (!std::isfinite(p.h_full_contact_W_m2K) || p.h_full_contact_W_m2K < 0.)
		p.h_full_contact_W_m2K = 100000.0;
	if (!std::isfinite(p.p_ref_Pa) || p.p_ref_Pa <= 0.)
		p.p_ref_Pa = 1.0e9;
	if (!std::isfinite(p.max_dT_per_step_K) || p.max_dT_per_step_K <= 0.)
		p.max_dT_per_step_K = 1.0;
	if (!std::isfinite(p.plane_strain_thickness_m) || p.plane_strain_thickness_m <= 0.)
		p.plane_strain_thickness_m = 1.0;
	if (!std::isfinite(p.contact_length_factor) || p.contact_length_factor <= 0.)
		p.contact_length_factor = 1.0;

	return p;
}

static const thermal_contact_coupling_params &get_thermal_contact_coupling_params() {
	static const thermal_contact_coupling_params cached = load_thermal_contact_coupling_params();
	return cached;
}

static contact_penalty_params load_contact_penalty_params() {
	contact_penalty_params p;

	{
		double v = 0.;
		const char *raw = nullptr;

		switch (read_env_double("MFREE_CONTACT_ALPHA", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.alpha0 = v;
			else
				warn_invalid_env_double("MFREE_CONTACT_ALPHA", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_ALPHA", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_CONTACT_ALPHA_MIN", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.alpha_min = v;
			else
				warn_invalid_env_double("MFREE_CONTACT_ALPHA_MIN", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_ALPHA_MIN", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_CONTACT_ALPHA_MAX", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.alpha_max = v;
			else
				warn_invalid_env_double("MFREE_CONTACT_ALPHA_MAX", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_ALPHA_MAX", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_CONTACT_PEN_DEPTH_REF", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.)
				p.pen_depth_ref_m = v;
			else
				warn_invalid_env_double("MFREE_CONTACT_PEN_DEPTH_REF", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_PEN_DEPTH_REF", raw);
			break;
		default:
			break;
		}
	}

	{
		const char *s = std::getenv("MFREE_CONTACT_ADAPTIVE_PENALTY");
		if (s && s[0] != '\0')
			p.adaptive = (std::atoi(s) != 0);
	}
	{
		const char *s = std::getenv("MFREE_CONTACT_USE_LM");
		if (s && s[0] != '\0')
			p.use_lagrange_multiplier = (std::atoi(s) != 0);
	}

	if (!(p.alpha_min > 0.))
		p.alpha_min = 1.0e-4;
	if (!(p.alpha_max > 0.))
		p.alpha_max = 10.0;
	if (p.alpha_max < p.alpha_min)
		std::swap(p.alpha_min, p.alpha_max);
	if (!(p.pen_depth_ref_m > 0.))
		p.pen_depth_ref_m = 1.0e-6;

	return p;
}

static const contact_penalty_params &get_contact_penalty_params() {
	static const contact_penalty_params cached = load_contact_penalty_params();
	return cached;
}
} // namespace

static glm::dvec2 compute_contact_force_nianfei(double pen_depth, glm::dvec2 surf_norm, double alpha, double ms, double dt) {
	// friction force according to
	// "3D adaptive RKPM method for contact problems with elastic–plastic dynamic
	// large deformation" - Nianfei, Guangyao, Shuyao

	const glm::dvec2 n = surf_norm;
	const double gN = pen_depth;

	double dt2 = dt * dt;
	glm::dvec2 fN = -ms * gN * n / dt2 * alpha;

	return fN;
}

static glm::dvec2 compute_friction_ldyna(const tool_contact_2d &master, glm::dvec2 fN, glm::dvec2 n, glm::dvec2 vs, glm::dvec2 fricold,
										 double alpha, double ms, double dt, double mu) {
	if (mu == 0.)
		return glm::dvec2(0.);

	glm::dvec2 vm = master.velocity_world();
	glm::dvec2 v = vs - vm;
	glm::dvec2 vr = v - v * n;

	glm::dvec2 kdeltae = alpha * ms * vr / dt;
	double fy = mu * glm::length(fN);
	glm::dvec2 fstar = fricold - kdeltae;

	if (glm::length(fstar) > fy) {
		return fy * fstar / glm::length(fstar);
	} else {
		return fstar;
	}
}

void contact_apply_master_to_body_2d(const tool_contact_2d &master, body &slave, fe_tool *thermal_master, double accounting_dt) {
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	double accounting_dt_safe = accounting_dt;
	if (!std::isfinite(accounting_dt_safe) || accounting_dt_safe <= 0.)
		accounting_dt_safe = dt;

	std::vector<particle> &particles = slave.get_particles();
	const double cp_wp = slave.get_sim_data().get_physical_constants().tc().cp();
	const contact_penalty_params &cpp = get_contact_penalty_params();

	struct contact_event {
		unsigned int pidx = 0;
		glm::dvec2 xcntct = glm::dvec2(0.);
		glm::dvec2 surf_norm = glm::dvec2(0.);
		glm::dvec2 cntc = glm::dvec2(0.);
		glm::dvec2 fric = glm::dvec2(0.);
		double pen_depth = 0.;
	};

	std::vector<contact_event> events;
	events.reserve(slave.get_num_part() / 8);

#ifdef _OPENMP
	int omp_threads = omp_get_max_threads();

	std::vector<std::vector<contact_event>> events_tls(static_cast<std::size_t>(omp_threads));

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		std::vector<contact_event> &local_events = events_tls[static_cast<std::size_t>(tid)];
		local_events.clear();
		local_events.reserve(256);
#pragma omp for schedule(static)
		for (int ii = 0; ii < static_cast<int>(slave.get_num_part()); ii++) {
			unsigned int i = static_cast<unsigned int>(ii);
#else
	int omp_threads = 1;
	for (unsigned int i = 0; i < slave.get_num_part(); i++) {
#endif

			double qx = particles[i].x;
			double qy = particles[i].y;

			glm::dvec2 xslave(qx, qy);
			tool_contact_hit_2d hit;
			bool inside = master.contact(xslave, hit);

			if (!inside) {
				particles[i].fcx = 0.;
				particles[i].fcy = 0.;

				particles[i].ftx = 0.;
				particles[i].fty = 0.;
				particles[i].contact_lambda_n = 0.;

				continue;
			}

			glm::dvec2 xcntct = hit.x_contact;
			glm::dvec2 surf_norm = hit.normal;

			double pen_depth = glm::dot((xslave - xcntct), surf_norm);
			glm::dvec2 fricold(particles[i].ftx, particles[i].fty);

			double ms = particles[i].m;

			glm::dvec2 vs(particles[i].vx, particles[i].vy);

			double alpha = cpp.alpha0;
			if (cpp.adaptive) {
				double g = std::abs(pen_depth);
				double s = g / cpp.pen_depth_ref_m;
				if (std::isfinite(s) && s > 1.0)
					alpha *= s;
				alpha = std::max(cpp.alpha_min, std::min(cpp.alpha_max, alpha));
			}

			glm::dvec2 cntc(0.);
			if (cpp.use_lagrange_multiplier) {
				double dt2 = dt * dt;
				double rho = (dt2 > 0. && std::isfinite(dt2)) ? (alpha * ms / dt2) : 0.;
				if (std::isfinite(rho) && rho > 0.) {
					double lambda = particles[i].contact_lambda_n;
					if (!std::isfinite(lambda) || lambda < 0.)
						lambda = 0.;
					lambda = std::max(0.0, lambda - rho * pen_depth);
					particles[i].contact_lambda_n = lambda;
					cntc = lambda * surf_norm;
				} else {
					particles[i].contact_lambda_n = 0.;
					cntc = compute_contact_force_nianfei(pen_depth, surf_norm, alpha, ms, dt);
				}
			} else {
				particles[i].contact_lambda_n = 0.;
				cntc = compute_contact_force_nianfei(pen_depth, surf_norm, alpha, ms, dt);
			}
			double mu = master.mu();
			glm::dvec2 fric = compute_friction_ldyna(master, cntc, surf_norm, vs, fricold, alpha, ms, dt, mu);

			particles[i].fcx = cntc.x;
			particles[i].fcy = cntc.y;
			particles[i].ftx = fric.x;
			particles[i].fty = fric.y;

			if (thermal_master) {
				contact_event ev;
				ev.pidx = i;
				ev.xcntct = xcntct;
				ev.surf_norm = surf_norm;
				ev.cntc = cntc;
				ev.fric = fric;
				ev.pen_depth = pen_depth;
#ifdef _OPENMP
				local_events.push_back(ev);
#else
			events.push_back(ev);
#endif
			}
		}
#ifdef _OPENMP
	}

	if (thermal_master) {
		std::size_t total = 0;
		for (const auto &v : events_tls)
			total += v.size();
		events.reserve(total);
		for (const auto &v : events_tls)
			events.insert(events.end(), v.begin(), v.end());
	}
#endif

	if (thermal_master) {
		for (const contact_event &ev : events) {
			glm::dvec2 F_tool = -(ev.cntc + ev.fric);
			if (std::isfinite(F_tool.x) && std::isfinite(F_tool.y))
				thermal_master->add_boundary_point_force(ev.xcntct, F_tool);
		}
	}

	if (thermal_master && cp_wp > 0. && std::isfinite(cp_wp)) {
		const thermal_contact_coupling_params &tcp = get_thermal_contact_coupling_params();

		double max_pred_dT = 0.;

		struct thermal_event {
			unsigned int pidx = 0;
			glm::dvec2 xcntct = glm::dvec2(0.);
			double P_cond = 0.;
			double P_fric = 0.;
		};

		std::vector<thermal_event> thermals;
		thermals.reserve(events.size());
		double sum_P_cond_raw = 0.;
		double sum_P_fric_raw = 0.;
		unsigned int contact_diag_count = 0;
		double sum_A_eff = 0.;
		double sum_hA = 0.;
		double sum_P_cond_pos_raw = 0.;
		double sum_P_cond_neg_raw = 0.;
		double sum_deltaT = 0.;
		double max_deltaT = 0.;
		double sum_h_c = 0.;
		double max_h_c = 0.;

		for (const contact_event &ev : events) {
			particle &p = particles[ev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T))
				continue;
			if (p.m <= 0. || p.rho <= 0.)
				continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.)
				continue;

			double particle_area_per_depth = p.m / p.rho;
			if (!std::isfinite(particle_area_per_depth) || particle_area_per_depth <= std::numeric_limits<double>::epsilon())
				continue;

			// In this 2D cutting model, p.m / p.rho is an area per unit out-of-plane depth (approximately dx^2), not
			// the thermal contact area.  The conductive interface measure is contact length times plane-strain thickness.
			double contact_length = std::sqrt(particle_area_per_depth) * tcp.contact_length_factor;
			if (!std::isfinite(contact_length) || contact_length <= std::numeric_limits<double>::epsilon())
				continue;
			double A_eff = contact_length * tcp.plane_strain_thickness_m;
			if (!std::isfinite(A_eff) || A_eff <= std::numeric_limits<double>::epsilon())
				continue;

			double Fn = glm::length(ev.cntc);
			if (!std::isfinite(Fn) || Fn < 0.)
				continue;
			double pressure = Fn / A_eff;
			if (!std::isfinite(pressure))
				continue;
			if (tcp.p_ref_Pa <= 0.0)
				continue;
			double s = pressure / tcp.p_ref_Pa;
			s = std::max(0.0, std::min(1.0, s));
			double h_c = tcp.h_separated_W_m2K + (tcp.h_full_contact_W_m2K - tcp.h_separated_W_m2K) * s;
			if (!std::isfinite(h_c) || h_c < 0.)
				continue;

			double T_tool = thermal_master->temperature_at_world_point_nearest_boundary(ev.xcntct);
			if (!std::isfinite(T_tool))
				continue;
			double deltaT = p.T - T_tool;
			if (!std::isfinite(deltaT))
				continue;
			double P_cond = h_c * A_eff * deltaT;
			if (!std::isfinite(P_cond))
				continue;

			glm::dvec2 vm = master.velocity_world();
			glm::dvec2 vs(p.vx, p.vy);
			glm::dvec2 v = vs - vm;
			glm::dvec2 vt = v - glm::dot(v, ev.surf_norm) * ev.surf_norm;
			double slip = glm::length(vt);
			if (!std::isfinite(slip) || slip < 0.)
				continue;
			double P_fric = glm::length(ev.fric) * slip;
			if (!std::isfinite(P_fric) || P_fric < 0.)
				continue;

			double pred_dT = accounting_dt_safe * (std::abs(P_cond) + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (!std::isfinite(pred_dT) || pred_dT < 0.)
				continue;
			max_pred_dT = std::max(max_pred_dT, pred_dT);

			thermal_event tev;
			tev.pidx = ev.pidx;
			tev.xcntct = ev.xcntct;
			tev.P_cond = P_cond;
			tev.P_fric = P_fric;
			thermals.push_back(tev);
			sum_P_cond_raw += P_cond;
			sum_P_fric_raw += P_fric;
			contact_diag_count++;
			sum_A_eff += A_eff;
			sum_hA += h_c * A_eff;
			if (P_cond >= 0.)
				sum_P_cond_pos_raw += P_cond;
			else
				sum_P_cond_neg_raw += P_cond;
			sum_deltaT += deltaT;
			max_deltaT = std::max(max_deltaT, deltaT);
			sum_h_c += h_c;
			max_h_c = std::max(max_h_c, h_c);
		}

		double scale = 1.0;
		if (std::isfinite(max_pred_dT) && max_pred_dT > tcp.max_dT_per_step_K && max_pred_dT > 0.)
			scale = tcp.max_dT_per_step_K / max_pred_dT;
		if (!std::isfinite(scale) || scale <= 0.)
			scale = 1.0;

		{
			thermal_master->add_contact_thermal_diagnostics(accounting_dt_safe, contact_diag_count, sum_A_eff, sum_hA, sum_P_cond_pos_raw,
															sum_P_cond_neg_raw, sum_P_cond_raw, sum_deltaT, max_deltaT, sum_h_c, max_h_c,
															max_pred_dT);

			fe_tool::contact_energy_balance eb;
			eb.P_cond = scale * sum_P_cond_raw;
			eb.P_fric = scale * sum_P_fric_raw;
			eb.scale = scale;
			eb.frac_workpiece = tcp.friction_heat_fraction_workpiece;
			eb.frac_tool = tcp.friction_heat_fraction_tool;
			thermal_master->set_contact_energy_balance(eb);
			thermal_master->add_contact_energy_accounting(accounting_dt_safe, sum_P_cond_raw, sum_P_fric_raw, scale,
														  tcp.friction_heat_fraction_workpiece, tcp.friction_heat_fraction_tool);
		}

		for (const thermal_event &tev : thermals) {
			particle &p = particles[tev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T))
				continue;
			if (p.m <= 0.)
				continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.)
				continue;

			double P_cond = scale * tev.P_cond;
			double P_fric = scale * tev.P_fric;
			if (!std::isfinite(P_cond) || !std::isfinite(P_fric))
				continue;

			double dT_t = (-P_cond + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (std::isfinite(dT_t))
				p.T_t += dT_t;
			double P_tool = P_cond + tcp.friction_heat_fraction_tool * P_fric;
			if (std::isfinite(P_tool))
				thermal_master->add_boundary_point_power(tev.xcntct, P_tool);
		}
	}
}
