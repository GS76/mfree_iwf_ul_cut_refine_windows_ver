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

#include "logger.h"

#include "fe_tool.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <limits>

void logger::close() {
	if (m_fp_forces)
		fclose(m_fp_forces);
	if (m_fp_trace)
		fclose(m_fp_trace);
	if (m_fp_thermal)
		fclose(m_fp_thermal);
	if (m_fp_metrics)
		fclose(m_fp_metrics);
	if (m_fp_energy)
		fclose(m_fp_energy);
}

void logger::set_fe_tool(fe_tool *t) { m_t = t; }

void logger::set_log_vtk(bool log_vtk) { m_emit_vtk = log_vtk; }

void logger::set_log_forces(bool log_forces) { m_log_forces = log_forces; }

void logger::add_tracer_particle(unsigned int tracer_idx) { m_trace_p.push_back(tracer_idx); }

void logger::set_folder(const char *folder) {
	std::snprintf(m_folder, sizeof(m_folder), "%s", folder ? folder : "");

	m_wp_internal_E_init = -1.;
	m_tool_internal_E_init = -1.;
	m_cum_plastic_dissipation = 0.;
	m_suppression_warned = false;
	m_cum_contact_E_cond_raw = 0.;
	m_cum_contact_E_fric_raw = 0.;
	m_cum_contact_E_cond_scaled = 0.;
	m_cum_contact_E_fric_scaled = 0.;
	m_cum_contact_E_workpiece = 0.;
	m_cum_contact_E_tool = 0.;
	m_cum_contact_E_limiter_suppressed = 0.;
	m_cum_tool_E_sources = 0.;
	m_cum_tool_E_conduction = 0.;
	m_cum_tool_E_convection = 0.;
	m_cum_tool_E_dirichlet = 0.;

	if (m_fp_forces)
		fclose(m_fp_forces);

	std::filesystem::create_directories(m_folder);

	std::filesystem::path base(m_folder);
	std::filesystem::path forces = base / (std::string(m_case_name) + "_forces");
	m_fp_forces = fopen(forces.string().c_str(), "w+");

	if (m_fp_thermal)
		fclose(m_fp_thermal);
	std::filesystem::path thermal = base / (std::string(m_case_name) + "_thermal.csv");
	m_fp_thermal = fopen(thermal.string().c_str(), "w+");
	if (m_fp_thermal) {
		std::fprintf(m_fp_thermal, "time,step,P_cond_W,P_fric_W,scale,frac_wp,frac_tool,tool_pos_x,tool_pos_y,tool_vel_x,tool_vel_y,tool_"
								   "Tmin,tool_Tmax,wp_Tmin,wp_Tmax,wp_Tavg,contact_iters,rel_force,rel_power\n");
		std::fflush(m_fp_thermal);
	}

	if (m_fp_metrics)
		fclose(m_fp_metrics);
	std::filesystem::path metrics = base / (std::string(m_case_name) + "_metrics.csv");
	m_fp_metrics = fopen(metrics.string().c_str(), "w+");
	if (m_fp_metrics) {
		std::fprintf(m_fp_metrics, "time,step,wp_Tmin,wp_Tmax,wp_Tavg,wp_umax,wp_svm_max,wp_epspl_max,wp_contact_pmax,wp_contact_count\n");
		std::fflush(m_fp_metrics);
	}

	if (m_fp_energy)
		fclose(m_fp_energy);
	std::filesystem::path energy = base / (std::string(m_case_name) + "_energy.csv");
	m_fp_energy = fopen(energy.string().c_str(), "w+");
	if (m_fp_energy) {
		std::fprintf(m_fp_energy,
					 "time,step,step_dt,wp_internal_E_above_ref,tool_internal_E_above_ref,"
					 "step_contact_event_count,step_contact_area_eff,step_contact_hA,"
					 "step_contact_P_cond_pos_raw,step_contact_P_cond_neg_raw,step_contact_P_cond_net_raw,"
					 "step_contact_deltaT_mean,step_contact_deltaT_max,step_contact_h_c_mean,step_contact_h_c_max,"
					 "step_contact_max_pred_dT,"
					 "step_contact_E_cond_raw,step_contact_E_fric_raw,step_contact_E_cond_scaled,step_contact_E_fric_scaled,"
					 "step_contact_E_workpiece,step_contact_E_tool,step_contact_E_limiter_suppressed,"
					 "step_tool_E_sources,step_tool_E_conduction,step_tool_E_convection,step_tool_E_dirichlet,"
					 "cum_contact_E_cond_raw,cum_contact_E_fric_raw,cum_contact_E_cond_scaled,cum_contact_E_fric_scaled,"
					 "cum_contact_E_workpiece,cum_contact_E_tool,cum_contact_E_limiter_suppressed,"
					 "cum_tool_E_sources,cum_tool_E_conduction,cum_tool_E_convection,cum_tool_E_dirichlet,"
					 "step_suppression_ratio,step_tool_source_residual,cum_suppression_ratio,cum_tool_source_residual,"
					 "T_ref,step_plastic_dissipation,cum_plastic_dissipation,"
					 "delta_wp_internal_E,delta_tool_internal_E,closure_residual,closure_residual_pct,"
					 "step_contact_E_tool_frac,cum_contact_E_tool_frac\n");
		std::fflush(m_fp_energy);
	}
}

void logger::log_time_step_data(const body &b, unsigned int step) {
	static int thermal_cfg_init = 0;
	static bool log_thermal = true;
	if (thermal_cfg_init == 0) {
		thermal_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_THERMAL"); s && std::atoi(s) == 0)
			log_thermal = false;
	}

	const fe_tool *ft_log = b.get_fe_tool();
	if (log_thermal && m_fp_thermal && ft_log) {
		const fe_tool *ft = ft_log;
		fe_tool::contact_energy_balance eb = ft->get_contact_energy_balance();
		fe_tool::contact_convergence cc = ft->get_contact_convergence();
		glm::dvec2 tool_pos = ft->get_pos();
		glm::dvec2 tool_vel = ft->get_vel();

		double wp_Tmin = std::numeric_limits<double>::infinity();
		double wp_Tmax = -std::numeric_limits<double>::infinity();
		double wp_Tsum = 0.0;
		unsigned int wp_n = 0;
		for (unsigned int i = 0; i < b.get_num_part(); i++) {
			double T = b.get_particles()[i].T;
			if (!std::isfinite(T))
				continue;
			wp_Tmin = std::min(wp_Tmin, T);
			wp_Tmax = std::max(wp_Tmax, T);
			wp_Tsum += T;
			wp_n++;
		}
		if (!std::isfinite(wp_Tmin))
			wp_Tmin = 0.0;
		if (!std::isfinite(wp_Tmax))
			wp_Tmax = 0.0;
		double wp_Tavg = (wp_n > 0) ? (wp_Tsum / static_cast<double>(wp_n)) : 0.0;

		simulation_time *time = &simulation_time::getInstance();
		double cur_time = time->get_time();

		std::fprintf(
			m_fp_thermal,
			"%.15e,%u,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%u,%.15e,%.15e\n",
			cur_time, step, eb.P_cond, eb.P_fric, eb.scale, eb.frac_workpiece, eb.frac_tool, tool_pos.x, tool_pos.y, tool_vel.x, tool_vel.y,
			ft->min_temperature(), ft->max_temperature(), wp_Tmin, wp_Tmax, wp_Tavg, cc.iters, cc.rel_force, cc.rel_power);
		std::fflush(m_fp_thermal);
	}

	log_energy_block(b, step, ft_log);

	static int metrics_cfg_init = 0;
	static bool log_metrics = true;
	if (metrics_cfg_init == 0) {
		metrics_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_METRICS"); s && std::atoi(s) == 0)
			log_metrics = false;
	}

	if (log_metrics && m_fp_metrics) {
		double wp_Tmin = std::numeric_limits<double>::infinity();
		double wp_Tmax = -std::numeric_limits<double>::infinity();
		double wp_Tsum = 0.0;
		unsigned int wp_n = 0;
		double umax = 0.0;
		double svm_max = 0.0;
		double epspl_max = 0.0;
		double pmax = 0.0;
		unsigned int pcount = 0;
		for (unsigned int i = 0; i < b.get_num_part(); i++) {
			const particle &pi = b.get_particles()[i];
			if (std::isfinite(pi.T)) {
				wp_Tmin = std::min(wp_Tmin, pi.T);
				wp_Tmax = std::max(wp_Tmax, pi.T);
				wp_Tsum += pi.T;
				wp_n++;
			}
			double dx = pi.x - pi.X;
			double dy = pi.y - pi.Y;
			double u = std::sqrt(dx * dx + dy * dy);
			if (std::isfinite(u))
				umax = std::max(umax, u);
			double sxx = pi.Sxx - pi.p;
			double sxy = pi.Sxy;
			double syy = pi.Syy - pi.p;
			double szz = pi.Szz - pi.p;
			double svm = std::sqrt(std::abs((sxx * sxx + syy * syy + szz * szz) - sxx * syy - sxx * szz - syy * szz + 3.0 * (sxy * sxy)));
			if (std::isfinite(svm))
				svm_max = std::max(svm_max, svm);
			if (std::isfinite(pi.eps_pl_equiv))
				epspl_max = std::max(epspl_max, pi.eps_pl_equiv);
			double Fn = std::sqrt(pi.fcx * pi.fcx + pi.fcy * pi.fcy);
			double p = 0.0;
			if (Fn > 0.0 && pi.m > 0.0 && pi.rho > 0.0) {
				p = Fn * pi.rho / pi.m;
			}
			if (std::isfinite(p) && p > 0.0) {
				pmax = std::max(pmax, p);
				pcount++;
			}
		}
		if (!std::isfinite(wp_Tmin))
			wp_Tmin = 0.0;
		if (!std::isfinite(wp_Tmax))
			wp_Tmax = 0.0;
		double wp_Tavg = (wp_n > 0) ? (wp_Tsum / static_cast<double>(wp_n)) : 0.0;

		simulation_time *time = &simulation_time::getInstance();
		double cur_time = time->get_time();
		std::fprintf(m_fp_metrics, "%.15e,%u,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%u\n", cur_time, step, wp_Tmin,
					 wp_Tmax, wp_Tavg, umax, svm_max, epspl_max, pmax, pcount);
		std::fflush(m_fp_metrics);
	}
}

void logger::log(const body &b, unsigned int step) {
	static int cfg_init = 0;
	static bool log_vtk_workpiece = true;
	static bool log_vtk_tool = true;
	static bool log_vtk_fe_tool = true;
	static bool log_forces = true;
	static bool log_trace = true;
	if (cfg_init == 0) {
		cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_VTK_WORKPIECE"); s && std::atoi(s) == 0)
			log_vtk_workpiece = false;
		if (const char *s = std::getenv("MFREE_LOG_VTK_TOOL"); s && std::atoi(s) == 0)
			log_vtk_tool = false;
		if (const char *s = std::getenv("MFREE_LOG_VTK_FE_TOOL"); s && std::atoi(s) == 0)
			log_vtk_fe_tool = false;
		if (const char *s = std::getenv("MFREE_LOG_FORCES"); s && std::atoi(s) == 0)
			log_forces = false;
		if (const char *s = std::getenv("MFREE_LOG_TRACE"); s && std::atoi(s) == 0)
			log_trace = false;
	}

	// log forces (if desired)
	if (m_log_forces && log_forces && m_fp_forces) {
		double fx = 0.;
		double fy = 0.;

		// sum of X and Y components of both contact & tangential forces
		for (unsigned int i = 0; i < b.get_num_part(); i++) {
			fx += b.get_particles()[i].fcx + b.get_particles()[i].ftx;
			fy += b.get_particles()[i].fcy + b.get_particles()[i].fty;
		}

		simulation_time *time = &simulation_time::getInstance();
		double cur_time = time->get_time();

		fprintf(m_fp_forces, "%e %f %f\n", cur_time, fx, fy);
		fflush(m_fp_forces);
	}

	// trace particles to be traced
	if (log_trace) {
		if (m_fp_trace) {
			for (const auto it : m_trace_p) {
				fprintf(m_fp_trace, "%f %f ", b.get_particles()[it].x, b.get_particles()[it].y);
			}
			if (m_trace_p.size() != 0) {
				fprintf(m_fp_trace, "\n");
			}
		}
	}

	if (m_emit_vtk) {
		if (log_vtk_workpiece)
			vtk_writer_write(b.get_particles(), step, m_folder);
		if (b.get_fe_tool()) {
			if (log_vtk_tool)
				vtk_writer_write(b.get_fe_tool(), step, m_folder, "tool");
			if (log_vtk_fe_tool)
				vtk_writer_write(b.get_fe_tool(), step, m_folder);
		}
	}

	static int step_data_cfg_init = 0;
	static bool log_time_step_data_every_step = true;
	if (step_data_cfg_init == 0) {
		step_data_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_TIME_STEP_DATA_EVERY_STEP"); s && std::atoi(s) == 0)
			log_time_step_data_every_step = false;
	}
	if (log_time_step_data_every_step)
		return;

	static int thermal_cfg_init = 0;
	static bool log_thermal = true;
	if (thermal_cfg_init == 0) {
		thermal_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_THERMAL"); s && std::atoi(s) == 0)
			log_thermal = false;
	}

	const fe_tool *ft_log = b.get_fe_tool();
	if (log_thermal && m_fp_thermal && ft_log) {
		const fe_tool *ft = ft_log;
		fe_tool::contact_energy_balance eb = ft->get_contact_energy_balance();
		fe_tool::contact_convergence cc = ft->get_contact_convergence();
		glm::dvec2 tool_pos = ft->get_pos();
		glm::dvec2 tool_vel = ft->get_vel();

		double wp_Tmin = std::numeric_limits<double>::infinity();
		double wp_Tmax = -std::numeric_limits<double>::infinity();
		double wp_Tsum = 0.0;
		unsigned int wp_n = 0;
		for (unsigned int i = 0; i < b.get_num_part(); i++) {
			double T = b.get_particles()[i].T;
			if (!std::isfinite(T))
				continue;
			wp_Tmin = std::min(wp_Tmin, T);
			wp_Tmax = std::max(wp_Tmax, T);
			wp_Tsum += T;
			wp_n++;
		}
		if (!std::isfinite(wp_Tmin))
			wp_Tmin = 0.0;
		if (!std::isfinite(wp_Tmax))
			wp_Tmax = 0.0;
		double wp_Tavg = (wp_n > 0) ? (wp_Tsum / static_cast<double>(wp_n)) : 0.0;

		simulation_time *time = &simulation_time::getInstance();
		double cur_time = time->get_time();

		std::fprintf(
			m_fp_thermal, "%.15e,%u,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%u,%.15e,%.15e\n",
			cur_time, step, eb.P_cond, eb.P_fric, eb.scale, eb.frac_workpiece, eb.frac_tool, tool_pos.x, tool_pos.y, tool_vel.x, tool_vel.y,
			ft->min_temperature(), ft->max_temperature(), wp_Tmin, wp_Tmax, wp_Tavg, cc.iters, cc.rel_force, cc.rel_power);
		std::fflush(m_fp_thermal);
	}

	log_energy_block(b, step, ft_log);

	static int metrics_cfg_init = 0;
	static bool log_metrics = true;
	if (metrics_cfg_init == 0) {
		metrics_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_METRICS"); s && std::atoi(s) == 0)
			log_metrics = false;
	}

	if (log_metrics && m_fp_metrics) {
		double wp_Tmin = std::numeric_limits<double>::infinity();
		double wp_Tmax = -std::numeric_limits<double>::infinity();
		double wp_Tsum = 0.0;
		unsigned int wp_n = 0;
		double umax = 0.0;
		double svm_max = 0.0;
		double epspl_max = 0.0;
		double pmax = 0.0;
		unsigned int pcount = 0;
		for (unsigned int i = 0; i < b.get_num_part(); i++) {
			const particle &pi = b.get_particles()[i];
			if (std::isfinite(pi.T)) {
				wp_Tmin = std::min(wp_Tmin, pi.T);
				wp_Tmax = std::max(wp_Tmax, pi.T);
				wp_Tsum += pi.T;
				wp_n++;
			}
			double dx = pi.x - pi.X;
			double dy = pi.y - pi.Y;
			double u = std::sqrt(dx * dx + dy * dy);
			if (std::isfinite(u))
				umax = std::max(umax, u);
			double sxx = pi.Sxx - pi.p;
			double sxy = pi.Sxy;
			double syy = pi.Syy - pi.p;
			double szz = pi.Szz - pi.p;
			double svm = std::sqrt(std::abs((sxx * sxx + syy * syy + szz * szz) - sxx * syy - sxx * szz - syy * szz + 3.0 * (sxy * sxy)));
			if (std::isfinite(svm))
				svm_max = std::max(svm_max, svm);
			if (std::isfinite(pi.eps_pl_equiv))
				epspl_max = std::max(epspl_max, pi.eps_pl_equiv);
			double Fn = std::sqrt(pi.fcx * pi.fcx + pi.fcy * pi.fcy);
			double p = 0.0;
			if (Fn > 0.0 && pi.m > 0.0 && pi.rho > 0.0) {
				p = Fn * pi.rho / pi.m;
			}
			if (std::isfinite(p) && p > 0.0) {
				pmax = std::max(pmax, p);
				pcount++;
			}
		}
		if (!std::isfinite(wp_Tmin))
			wp_Tmin = 0.0;
		if (!std::isfinite(wp_Tmax))
			wp_Tmax = 0.0;
		double wp_Tavg = (wp_n > 0) ? (wp_Tsum / static_cast<double>(wp_n)) : 0.0;

		simulation_time *time = &simulation_time::getInstance();
		double cur_time = time->get_time();
		std::fprintf(m_fp_metrics, "%.15e,%u,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%.15e,%u\n", cur_time, step, wp_Tmin, wp_Tmax, wp_Tavg,
					 umax, svm_max, epspl_max, pmax, pcount);
		std::fflush(m_fp_metrics);
	}
}

void logger::log_energy_block(const body &b, unsigned int step, const fe_tool *ft_log) {
	static int energy_cfg_init = 0;
	static bool log_energy = true;
	if (energy_cfg_init == 0) {
		energy_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_ENERGY"); s && std::atoi(s) == 0)
			log_energy = false;
	}

	// Read reference temperature once (default 298.15 K = 25 degC).
	static int T_ref_cfg_init = 0;
	if (T_ref_cfg_init == 0) {
		T_ref_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_THERMAL_T_REF")) {
			double v = std::atof(s);
			if (std::isfinite(v) && v >= 0.)
				m_T_ref = v;
		}
	}

	if (!log_energy || !m_fp_energy || !ft_log)
		return;

	const fe_tool *ft = ft_log;
	fe_tool::thermal_energy_accounting ea = ft->get_thermal_energy_accounting();

	// Workpiece internal energy above T_ref: sum(m_i * cp * (T_i - T_ref))
	const double cp_wp = b.get_sim_data().get_physical_constants().tc().cp();
	double wp_internal_E = 0.;
	if (std::isfinite(cp_wp) && cp_wp > 0.) {
		for (unsigned int i = 0; i < b.get_num_part(); i++) {
			const particle &pi = b.get_particles()[i];
			if (!std::isfinite(pi.m) || !std::isfinite(pi.T))
				continue;
			wp_internal_E += pi.m * cp_wp * (pi.T - m_T_ref);
		}
	}

	// Tool internal energy above T_ref: sum(capacity_i * (T_i - T_ref))
	const double tool_internal_E = ft->thermal_internal_energy_above_ref(m_T_ref);

	// Capture initial-state baselines on the very first logged step.
	if (m_wp_internal_E_init < 0.) {
		m_wp_internal_E_init = wp_internal_E;
		m_tool_internal_E_init = tool_internal_E;
	}

	// Accumulate step values.
	m_cum_contact_E_cond_raw += ea.step_contact_E_cond_raw;
	m_cum_contact_E_fric_raw += ea.step_contact_E_fric_raw;
	m_cum_contact_E_cond_scaled += ea.step_contact_E_cond_scaled;
	m_cum_contact_E_fric_scaled += ea.step_contact_E_fric_scaled;
	m_cum_contact_E_workpiece += ea.step_contact_E_workpiece;
	m_cum_contact_E_tool += ea.step_contact_E_tool;
	m_cum_contact_E_limiter_suppressed += ea.step_contact_E_limiter_suppressed;
	m_cum_tool_E_sources += ea.step_tool_E_sources;
	m_cum_tool_E_conduction += ea.step_tool_E_conduction;
	m_cum_tool_E_convection += ea.step_tool_E_convection;
	m_cum_tool_E_dirichlet += ea.step_tool_E_dirichlet;
	// m_cum_plastic_dissipation is accumulated per solver step via
	// accumulate_plastic_dissipation() so it is correct regardless of
	// whether log() / log_time_step_data() are called every step or not.

	// Limiter suppression ratio: fraction of raw interface exchange discarded by
	// the 1-degC/step safety limiter.  Equals (1 - scale) when contact is active,
	// 0 when the limiter is inactive.  The previous 'step_interface_balance_residual'
	// was algebraically zero always ((frac_wp+frac_tool-1)*E_fric_scaled = 0) and
	// has been replaced with this non-trivial diagnostic.
	const double step_denom_raw =
		std::abs(ea.step_contact_E_cond_raw) + ea.step_contact_E_fric_raw;
	const double step_suppression_ratio =
		(step_denom_raw > 1e-30) ? ea.step_contact_E_limiter_suppressed / step_denom_raw : 0.;

	// Warn once per results folder when the limiter is discarding a significant
	// fraction (> 10%) of the raw interface exchange.  This usually means the
	// SPH timestep is too large for reliable thermal coupling, or
	// MFREE_THERMAL_MAX_DT_PER_STEP should be relaxed.
	if (!m_suppression_warned && step_suppression_ratio > 0.10) {
		m_suppression_warned = true;
		std::fprintf(stderr,
					 "[energy WARNING] step_suppression_ratio=%.3f at step %u: the "
					 "1-degC/step limiter is suppressing >10%% of the interface exchange.\n"
					 "  Check MFREE_THERMAL_MAX_DT_PER_STEP (current cap=%.2f K) and/or "
					 "the global SPH timestep.\n"
					 "  See docs/coupling_thermal_mechanical.md section "
					 "'Limiter Suppression of Interface Exchange' for guidance.\n",
					 step_suppression_ratio, step,
					 ea.step_contact_max_pred_dT > 0. ? ea.step_contact_max_pred_dT : 1.0);
		std::fflush(stderr);
	}

	const double step_tool_source_residual = ea.step_tool_E_sources - ea.step_contact_E_tool;
	const double cum_denom_raw =
		std::abs(m_cum_contact_E_cond_raw) + m_cum_contact_E_fric_raw;
	const double cum_suppression_ratio =
		(cum_denom_raw > 1e-30) ? m_cum_contact_E_limiter_suppressed / cum_denom_raw : 0.;
	const double cum_tool_source_residual = m_cum_tool_E_sources - m_cum_contact_E_tool;

	// Full-system energy closure.
	// Identity: cum_plastic_dissipation + cum_fric_scaled
	//         = delta_wp + delta_tool + cum_convection + cum_suppressed
	const double delta_wp = wp_internal_E - m_wp_internal_E_init;
	const double delta_tool = tool_internal_E - m_tool_internal_E_init;
	const double total_input = m_cum_plastic_dissipation + m_cum_contact_E_fric_scaled;
	const double total_stored_and_lost =
		delta_wp + delta_tool + m_cum_tool_E_convection + m_cum_contact_E_limiter_suppressed;
	const double closure_residual = total_input - total_stored_and_lost;
	const double closure_residual_pct =
		(total_input > 1e-30) ? (closure_residual / total_input * 100.) : 0.;

	// Tool fraction of total interface exchange.
	//
	// Denominator = |E_cond_scaled| + E_fric_scaled  (total energy exchanged
	// at the interface: conduction magnitude + friction).
	//
	// The previously used denominator (E_tool + E_workpiece) algebraically
	// equals E_fric_scaled only (conduction cancels in the sum), so the
	// fraction exceeds 1 whenever P_cond >> P_fric — which is exactly the
	// physically important regime at high contact conductance.
	//
	// With the corrected denominator the fraction is bounded in [0, 1]:
	//   frac -> frac_tool          when P_cond -> 0 (friction-dominated)
	//   frac -> 1                  when P_cond >> P_fric (conduction-dominated)
	const double step_iface_denom =
		std::abs(ea.step_contact_E_cond_scaled) + ea.step_contact_E_fric_scaled;
	const double step_contact_E_tool_frac =
		(step_iface_denom > 1e-30) ? ea.step_contact_E_tool / step_iface_denom : 0.;
	const double cum_iface_denom =
		std::abs(m_cum_contact_E_cond_scaled) + m_cum_contact_E_fric_scaled;
	const double cum_contact_E_tool_frac =
		(cum_iface_denom > 1e-30) ? m_cum_contact_E_tool / cum_iface_denom : 0.;

	simulation_time *time = &simulation_time::getInstance();
	double cur_time = time->get_time();

	std::fprintf(m_fp_energy,
				 "%.15e,%u,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,"
				 "%.15e,%.15e,%.15e,%.15e,"
				 "%.15e,%.15e\n",
				 cur_time, step, ea.step_dt, wp_internal_E, tool_internal_E,
				 ea.step_contact_event_count, ea.step_contact_area_eff, ea.step_contact_hA,
				 ea.step_contact_P_cond_pos_raw, ea.step_contact_P_cond_neg_raw, ea.step_contact_P_cond_net_raw,
				 ea.step_contact_deltaT_mean, ea.step_contact_deltaT_max, ea.step_contact_h_c_mean, ea.step_contact_h_c_max,
				 ea.step_contact_max_pred_dT,
				 ea.step_contact_E_cond_raw, ea.step_contact_E_fric_raw, ea.step_contact_E_cond_scaled, ea.step_contact_E_fric_scaled,
				 ea.step_contact_E_workpiece, ea.step_contact_E_tool, ea.step_contact_E_limiter_suppressed,
				 ea.step_tool_E_sources, ea.step_tool_E_conduction, ea.step_tool_E_convection, ea.step_tool_E_dirichlet,
				 m_cum_contact_E_cond_raw, m_cum_contact_E_fric_raw, m_cum_contact_E_cond_scaled, m_cum_contact_E_fric_scaled,
				 m_cum_contact_E_workpiece, m_cum_contact_E_tool, m_cum_contact_E_limiter_suppressed,
				 m_cum_tool_E_sources, m_cum_tool_E_conduction, m_cum_tool_E_convection, m_cum_tool_E_dirichlet,
				 step_suppression_ratio, step_tool_source_residual, cum_suppression_ratio, cum_tool_source_residual,
				 m_T_ref, b.get_step_plastic_dissipation(), m_cum_plastic_dissipation,
				 delta_wp, delta_tool, closure_residual, closure_residual_pct,
				 step_contact_E_tool_frac, cum_contact_E_tool_frac);
	std::fflush(m_fp_energy);
}

void logger::accumulate_plastic_dissipation(const body &b) {
	// Accumulate Taylor-Quinney plastic dissipation every solver step so the
	// cumulative total in the energy CSV is correct regardless of whether
	// log_time_step_data() is called every step or only at output_freq intervals.
	m_cum_plastic_dissipation += b.get_step_plastic_dissipation();
}

logger::logger(const char *case_name, const char *foldername) {
	const char *results_dir_env = std::getenv("MFREE_RESULTS_DIR");
	const char *folder = (results_dir_env && results_dir_env[0] != '\0') ? results_dir_env : foldername;
	std::filesystem::create_directories(folder);
	std::snprintf(m_folder, sizeof(m_folder), "%s", folder ? folder : "");
	std::snprintf(m_case_name, sizeof(m_case_name), "%s", case_name ? case_name : "");

	std::filesystem::path base(m_folder);
	std::filesystem::path forces = base / (std::string(m_case_name) + "_forces");
	std::filesystem::path trace = base / "trace.txt";
	std::filesystem::path thermal = base / (std::string(m_case_name) + "_thermal.csv");
	std::filesystem::path metrics = base / (std::string(m_case_name) + "_metrics.csv");
	std::filesystem::path energy = base / (std::string(m_case_name) + "_energy.csv");
	m_fp_forces = fopen(forces.string().c_str(), "w+");
	m_fp_trace = fopen(trace.string().c_str(), "w+");
	m_fp_thermal = fopen(thermal.string().c_str(), "w+");
	if (m_fp_thermal) {
		std::fprintf(m_fp_thermal, "time,step,P_cond_W,P_fric_W,scale,frac_wp,frac_tool,tool_pos_x,tool_pos_y,tool_vel_x,tool_vel_y,tool_"
								   "Tmin,tool_Tmax,wp_Tmin,wp_Tmax,wp_Tavg,contact_iters,rel_force,rel_power\n");
		std::fflush(m_fp_thermal);
	}
	m_fp_metrics = fopen(metrics.string().c_str(), "w+");
	if (m_fp_metrics) {
		std::fprintf(m_fp_metrics, "time,step,wp_Tmin,wp_Tmax,wp_Tavg,wp_umax,wp_svm_max,wp_epspl_max,wp_contact_pmax,wp_contact_count\n");
		std::fflush(m_fp_metrics);
	}
	m_fp_energy = fopen(energy.string().c_str(), "w+");
	if (m_fp_energy) {
		std::fprintf(m_fp_energy,
					 "time,step,step_dt,wp_internal_E_above_ref,tool_internal_E_above_ref,"
					 "step_contact_event_count,step_contact_area_eff,step_contact_hA,"
					 "step_contact_P_cond_pos_raw,step_contact_P_cond_neg_raw,step_contact_P_cond_net_raw,"
					 "step_contact_deltaT_mean,step_contact_deltaT_max,step_contact_h_c_mean,step_contact_h_c_max,"
					 "step_contact_max_pred_dT,"
					 "step_contact_E_cond_raw,step_contact_E_fric_raw,step_contact_E_cond_scaled,step_contact_E_fric_scaled,"
					 "step_contact_E_workpiece,step_contact_E_tool,step_contact_E_limiter_suppressed,"
					 "step_tool_E_sources,step_tool_E_conduction,step_tool_E_convection,step_tool_E_dirichlet,"
					 "cum_contact_E_cond_raw,cum_contact_E_fric_raw,cum_contact_E_cond_scaled,cum_contact_E_fric_scaled,"
					 "cum_contact_E_workpiece,cum_contact_E_tool,cum_contact_E_limiter_suppressed,"
					 "cum_tool_E_sources,cum_tool_E_conduction,cum_tool_E_convection,cum_tool_E_dirichlet,"
					 "step_suppression_ratio,step_tool_source_residual,cum_suppression_ratio,cum_tool_source_residual,"
					 "T_ref,step_plastic_dissipation,cum_plastic_dissipation,"
					 "delta_wp_internal_E,delta_tool_internal_E,closure_residual,closure_residual_pct,"
					 "step_contact_E_tool_frac,cum_contact_E_tool_frac\n");
		std::fflush(m_fp_energy);
	}
}
