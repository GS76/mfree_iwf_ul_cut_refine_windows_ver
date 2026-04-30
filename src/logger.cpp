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
					 "time,step,step_dt,wp_internal_E,tool_internal_E,"
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
					 "step_interface_balance_residual,step_tool_source_residual,cum_interface_balance_residual,cum_tool_source_residual\n");
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

	static int energy_cfg_init = 0;
	static bool log_energy = true;
	if (energy_cfg_init == 0) {
		energy_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_ENERGY"); s && std::atoi(s) == 0)
			log_energy = false;
	}

	if (log_energy && m_fp_energy && ft_log) {
		const fe_tool *ft = ft_log;
		fe_tool::thermal_energy_accounting ea = ft->get_thermal_energy_accounting();

		const double cp_wp = b.get_sim_data().get_physical_constants().tc().cp();
		double wp_internal_E = 0.;
		if (std::isfinite(cp_wp) && cp_wp > 0.) {
			for (unsigned int i = 0; i < b.get_num_part(); i++) {
				const particle &pi = b.get_particles()[i];
				if (!std::isfinite(pi.m) || !std::isfinite(pi.T))
					continue;
				wp_internal_E += pi.m * cp_wp * pi.T;
			}
		}

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

		const double step_interface_balance_residual =
			(ea.step_contact_E_workpiece + ea.step_contact_E_tool) - ea.step_contact_E_fric_scaled;
		const double step_tool_source_residual = ea.step_tool_E_sources - ea.step_contact_E_tool;
		const double cum_interface_balance_residual = (m_cum_contact_E_workpiece + m_cum_contact_E_tool) - m_cum_contact_E_fric_scaled;
		const double cum_tool_source_residual = m_cum_tool_E_sources - m_cum_contact_E_tool;

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
					 "%.15e,%.15e,%.15e,%.15e\n",
					 cur_time, step, ea.step_dt, wp_internal_E, ea.tool_internal_E, ea.step_contact_event_count, ea.step_contact_area_eff,
					 ea.step_contact_hA, ea.step_contact_P_cond_pos_raw, ea.step_contact_P_cond_neg_raw, ea.step_contact_P_cond_net_raw,
					 ea.step_contact_deltaT_mean, ea.step_contact_deltaT_max, ea.step_contact_h_c_mean, ea.step_contact_h_c_max,
					 ea.step_contact_max_pred_dT, ea.step_contact_E_cond_raw, ea.step_contact_E_fric_raw, ea.step_contact_E_cond_scaled,
					 ea.step_contact_E_fric_scaled, ea.step_contact_E_workpiece, ea.step_contact_E_tool,
					 ea.step_contact_E_limiter_suppressed, ea.step_tool_E_sources, ea.step_tool_E_conduction, ea.step_tool_E_convection,
					 ea.step_tool_E_dirichlet, m_cum_contact_E_cond_raw, m_cum_contact_E_fric_raw, m_cum_contact_E_cond_scaled,
					 m_cum_contact_E_fric_scaled, m_cum_contact_E_workpiece, m_cum_contact_E_tool, m_cum_contact_E_limiter_suppressed,
					 m_cum_tool_E_sources, m_cum_tool_E_conduction, m_cum_tool_E_convection, m_cum_tool_E_dirichlet,
					 step_interface_balance_residual, step_tool_source_residual, cum_interface_balance_residual, cum_tool_source_residual);
		std::fflush(m_fp_energy);
	}

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

	static int energy_cfg_init = 0;
	static bool log_energy = true;
	if (energy_cfg_init == 0) {
		energy_cfg_init = 1;
		if (const char *s = std::getenv("MFREE_LOG_ENERGY"); s && std::atoi(s) == 0)
			log_energy = false;
	}

	if (log_energy && m_fp_energy && ft_log) {
		const fe_tool *ft = ft_log;
		fe_tool::thermal_energy_accounting ea = ft->get_thermal_energy_accounting();

		const double cp_wp = b.get_sim_data().get_physical_constants().tc().cp();
		double wp_internal_E = 0.;
		if (std::isfinite(cp_wp) && cp_wp > 0.) {
			for (unsigned int i = 0; i < b.get_num_part(); i++) {
				const particle &pi = b.get_particles()[i];
				if (!std::isfinite(pi.m) || !std::isfinite(pi.T))
					continue;
				wp_internal_E += pi.m * cp_wp * pi.T;
			}
		}

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

		const double step_interface_balance_residual =
			(ea.step_contact_E_workpiece + ea.step_contact_E_tool) - ea.step_contact_E_fric_scaled;
		const double step_tool_source_residual = ea.step_tool_E_sources - ea.step_contact_E_tool;
		const double cum_interface_balance_residual = (m_cum_contact_E_workpiece + m_cum_contact_E_tool) - m_cum_contact_E_fric_scaled;
		const double cum_tool_source_residual = m_cum_tool_E_sources - m_cum_contact_E_tool;

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
					 "%.15e,%.15e,%.15e,%.15e\n",
					 cur_time, step, ea.step_dt, wp_internal_E, ea.tool_internal_E, ea.step_contact_event_count, ea.step_contact_area_eff,
					 ea.step_contact_hA, ea.step_contact_P_cond_pos_raw, ea.step_contact_P_cond_neg_raw, ea.step_contact_P_cond_net_raw,
					 ea.step_contact_deltaT_mean, ea.step_contact_deltaT_max, ea.step_contact_h_c_mean, ea.step_contact_h_c_max,
					 ea.step_contact_max_pred_dT, ea.step_contact_E_cond_raw, ea.step_contact_E_fric_raw, ea.step_contact_E_cond_scaled,
					 ea.step_contact_E_fric_scaled, ea.step_contact_E_workpiece, ea.step_contact_E_tool,
					 ea.step_contact_E_limiter_suppressed, ea.step_tool_E_sources, ea.step_tool_E_conduction, ea.step_tool_E_convection,
					 ea.step_tool_E_dirichlet, m_cum_contact_E_cond_raw, m_cum_contact_E_fric_raw, m_cum_contact_E_cond_scaled,
					 m_cum_contact_E_fric_scaled, m_cum_contact_E_workpiece, m_cum_contact_E_tool, m_cum_contact_E_limiter_suppressed,
					 m_cum_tool_E_sources, m_cum_tool_E_conduction, m_cum_tool_E_convection, m_cum_tool_E_dirichlet,
					 step_interface_balance_residual, step_tool_source_residual, cum_interface_balance_residual, cum_tool_source_residual);
		std::fflush(m_fp_energy);
	}

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
					 "time,step,step_dt,wp_internal_E,tool_internal_E,"
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
					 "step_interface_balance_residual,step_tool_source_residual,cum_interface_balance_residual,cum_tool_source_residual\n");
		std::fflush(m_fp_energy);
	}
}
