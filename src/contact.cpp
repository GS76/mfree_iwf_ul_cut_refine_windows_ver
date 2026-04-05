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

#include "fe_tool.h"

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
};

enum class env_double_status {
	not_set,
	ok,
	invalid
};

static void warn_invalid_env_double(const char *key, const char *value) {
	std::fprintf(stderr, "WARNING: invalid value for %s: '%s' (ignored)\n", key, value ? value : "");
}

static env_double_status read_env_double(const char *key, double &out, const char **raw) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0') return env_double_status::not_set;
	if (raw) *raw = s;
	char *end = nullptr;
	errno = 0;
	double v = std::strtod(s, &end);
	if (end == s || errno != 0 || !std::isfinite(v)) return env_double_status::invalid;
	while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') ++end;
	if (*end != '\0') return env_double_status::invalid;
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
			if (v >= 0.) p.h_separated_W_m2K = v;
			else warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_H_FULL", v, &raw)) {
		case env_double_status::ok:
			if (v >= 0.) p.h_full_contact_W_m2K = v;
			else warn_invalid_env_double("MFREE_THERMAL_H_FULL", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_H_FULL", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_P_REF", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.p_ref_Pa = v;
			else warn_invalid_env_double("MFREE_THERMAL_P_REF", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_P_REF", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.max_dT_per_step_K = v;
			else warn_invalid_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", raw);
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

		if (st_wp == env_double_status::invalid) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
		if (st_tool == env_double_status::invalid) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);

		bool has_wp = (st_wp == env_double_status::ok);
		bool has_tool = (st_tool == env_double_status::ok);

		if (has_wp && !has_tool) frac_tool = 1.0 - frac_wp;
		if (has_tool && !has_wp) frac_wp = 1.0 - frac_tool;

		if (has_wp || has_tool) {
			if (!std::isfinite(frac_wp) || !std::isfinite(frac_tool)) {
				if (has_wp) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
				if (has_tool) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			}
			if (has_wp && (frac_wp < 0.0 || frac_wp > 1.0)) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
			if (has_tool && (frac_tool < 0.0 || frac_tool > 1.0)) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			frac_wp = std::max(0.0, std::min(1.0, frac_wp));
			frac_tool = std::max(0.0, std::min(1.0, frac_tool));
			double s = frac_wp + frac_tool;
			if (s > 0.) {
				p.friction_heat_fraction_workpiece = frac_wp / s;
				p.friction_heat_fraction_tool = frac_tool / s;
			} else {
				if (has_wp) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
				if (has_tool) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			}
		}
	}

	if (!std::isfinite(p.h_separated_W_m2K) || p.h_separated_W_m2K < 0.) p.h_separated_W_m2K = 1000.0;
	if (!std::isfinite(p.h_full_contact_W_m2K) || p.h_full_contact_W_m2K < 0.) p.h_full_contact_W_m2K = 100000.0;
	if (!std::isfinite(p.p_ref_Pa) || p.p_ref_Pa <= 0.) p.p_ref_Pa = 1.0e9;
	if (!std::isfinite(p.max_dT_per_step_K) || p.max_dT_per_step_K <= 0.) p.max_dT_per_step_K = 1.0;

	return p;
}

static const thermal_contact_coupling_params &get_thermal_contact_coupling_params() {
	static const thermal_contact_coupling_params cached = load_thermal_contact_coupling_params();
	return cached;
}
}

static glm::dvec2 compute_contact_force_nianfei(const tool *master, double pen_depth, glm::dvec2 surf_norm, double alpha, double ms, double dt) {
	// friction force according to
	// "3D adaptive RKPM method for contact problems with elastic–plastic dynamic
	// large deformation" - Nianfei, Guangyao, Shuyao

	const glm::dvec2 n = surf_norm;
	const double gN = pen_depth;

	double dt2 = dt*dt;
	glm::dvec2 fN = -ms*gN*n/dt2*alpha;

	return fN;
}

static glm::dvec2 compute_friction_ldyna(const tool *master, glm::dvec2 fN, glm::dvec2 n, glm::dvec2 vs, glm::dvec2 fricold, double alpha, double ms, double dt, double mu) {
	if (mu == 0.) return glm::dvec2(0.);

	glm::dvec2 vm = master->get_vel();
	glm::dvec2 v = vs-vm;
	glm::dvec2 vr = v - v*n;

	glm::dvec2 kdeltae = alpha*ms*vr/dt;
	double fy = mu*glm::length(fN);
	glm::dvec2 fstar = fricold - kdeltae;

	if (glm::length(fstar) > fy) {
		return fy*fstar/glm::length(fstar);
	} else {
		return fstar;
	}
}

void contact_apply_tool_to_body_2d(const tool *master, body &slave) {
	contact_apply_tool_to_body_2d(master, slave, nullptr);
}

void contact_apply_tool_to_body_2d(const tool *master, body &slave, fe_tool *thermal_master) {
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();

	std::vector<particle> &particles = slave.get_particles();
	const double cp_wp = slave.get_sim_data().get_physical_constants().tc().cp();

	const double alpha = 0.1;

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

		glm::dvec2 xcntct(0.);
		glm::dvec2 surf_norm(0.);
		glm::dvec2 xslave(qx, qy);

		bool inside = master->contact(xslave, xcntct, surf_norm);

		/*
		 both CONTACT & TANGENTIAL forces are ZERO
		 for particles which are outside the tool
		*/
		if (!inside) {
			particles[i].fcx = 0.;
			particles[i].fcy = 0.;

			particles[i].ftx = 0.;
			particles[i].fty = 0.;

			continue;
		}

		double pen_depth = glm::dot((xslave-xcntct), surf_norm);
		glm::dvec2 fricold(particles[i].ftx, particles[i].fty);

		double ms   = particles[i].m;

		glm::dvec2 vs(particles[i].vx, particles[i].vy);

		glm::dvec2 cntc = compute_contact_force_nianfei(master, pen_depth, surf_norm, alpha, ms, dt);
		glm::dvec2 fric = compute_friction_ldyna(master, cntc, surf_norm, vs, fricold, alpha, ms, dt, master->mu());

		// X and Y components of the contact force
		particles[i].fcx = cntc.x;
		particles[i].fcy = cntc.y;
		// X and Y components of the tangential force
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
		for (const auto &v : events_tls) total += v.size();
		events.reserve(total);
		for (const auto &v : events_tls) events.insert(events.end(), v.begin(), v.end());
	}
#endif

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

		for (const contact_event &ev : events) {
			particle &p = particles[ev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T)) continue;
			if (p.m <= 0. || p.rho <= 0.) continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.) continue;

			double A_eff = p.m / p.rho;
			if (!std::isfinite(A_eff) || A_eff <= std::numeric_limits<double>::epsilon()) continue;

			double Fn = glm::length(ev.cntc);
			if (!std::isfinite(Fn) || Fn < 0.) continue;
			double pressure = Fn / A_eff;
			if (!std::isfinite(pressure)) continue;
			if (tcp.p_ref_Pa <= 0.0) continue;
			double s = pressure / tcp.p_ref_Pa;
			s = std::max(0.0, std::min(1.0, s));
			double h_c = tcp.h_separated_W_m2K + (tcp.h_full_contact_W_m2K - tcp.h_separated_W_m2K) * s;
			if (!std::isfinite(h_c) || h_c < 0.) continue;

			double T_tool = thermal_master->temperature_at_world_point_nearest_boundary(ev.xcntct);
			if (!std::isfinite(T_tool)) continue;
			double P_cond = h_c * A_eff * (p.T - T_tool);
			if (!std::isfinite(P_cond)) continue;

			glm::dvec2 vm = master->get_vel();
			glm::dvec2 vs(p.vx, p.vy);
			glm::dvec2 v = vs - vm;
			glm::dvec2 vt = v - glm::dot(v, ev.surf_norm) * ev.surf_norm;
			double slip = glm::length(vt);
			if (!std::isfinite(slip) || slip < 0.) continue;
			double P_fric = glm::length(ev.fric) * slip;
			if (!std::isfinite(P_fric) || P_fric < 0.) continue;

			double pred_dT = dt * (std::abs(P_cond) + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (!std::isfinite(pred_dT) || pred_dT < 0.) continue;
			max_pred_dT = std::max(max_pred_dT, pred_dT);

			thermal_event tev;
			tev.pidx = ev.pidx;
			tev.xcntct = ev.xcntct;
			tev.P_cond = P_cond;
			tev.P_fric = P_fric;
			thermals.push_back(tev);
		}

		double scale = 1.0;
		if (std::isfinite(max_pred_dT) && max_pred_dT > tcp.max_dT_per_step_K && max_pred_dT > 0.) scale = tcp.max_dT_per_step_K / max_pred_dT;
		if (!std::isfinite(scale) || scale <= 0.) scale = 1.0;

		for (const thermal_event &tev : thermals) {
			particle &p = particles[tev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T)) continue;
			if (p.m <= 0.) continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.) continue;

			double P_cond = scale * tev.P_cond;
			double P_fric = scale * tev.P_fric;
			if (!std::isfinite(P_cond) || !std::isfinite(P_fric)) continue;

			double dT_t = (-P_cond + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (std::isfinite(dT_t)) p.T_t += dT_t;
			double P_tool = P_cond + tcp.friction_heat_fraction_tool * P_fric;
			if (std::isfinite(P_tool)) thermal_master->add_boundary_point_power(tev.xcntct, P_tool);
		}
	}

}
