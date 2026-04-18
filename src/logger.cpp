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

#include <cstdlib>
#include <filesystem>

void logger::close() {
	if (m_fp_forces) fclose(m_fp_forces);
	if (m_fp_trace) fclose(m_fp_trace);
}

void logger::set_tool(tool *t) {
	m_t = t;
}

void logger::set_log_vtk(bool log_vtk) {
	m_emit_vtk = log_vtk;
}

void logger::set_log_forces(bool log_forces) {
	m_log_forces = log_forces;
}

void logger::add_tracer_particle(unsigned int tracer_idx) {
	m_trace_p.push_back(tracer_idx);
}

void logger::set_folder(const char* folder) {
	std::snprintf(m_folder, sizeof(m_folder), "%s", folder ? folder : "");

	if (m_fp_forces) fclose(m_fp_forces);

	std::filesystem::create_directories(m_folder);

	std::filesystem::path base(m_folder);
	std::filesystem::path forces = base / (std::string(m_case_name) + "_forces");
	m_fp_forces = fopen(forces.string().c_str(), "w+");
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
		if (const char *s = std::getenv("MFREE_LOG_VTK_WORKPIECE"); s && std::atoi(s) == 0) log_vtk_workpiece = false;
		if (const char *s = std::getenv("MFREE_LOG_VTK_TOOL"); s && std::atoi(s) == 0) log_vtk_tool = false;
		if (const char *s = std::getenv("MFREE_LOG_VTK_FE_TOOL"); s && std::atoi(s) == 0) log_vtk_fe_tool = false;
		if (const char *s = std::getenv("MFREE_LOG_FORCES"); s && std::atoi(s) == 0) log_forces = false;
		if (const char *s = std::getenv("MFREE_LOG_TRACE"); s && std::atoi(s) == 0) log_trace = false;
	}

	//log forces (if desired)
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

	//trace particles to be traced
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
		if (log_vtk_workpiece) vtk_writer_write(b.get_particles(), step, m_folder);
		const char *use_mesh_env = std::getenv("MFREE_USE_FE_TOOL_FOR_CONTACT");
		bool use_mesh_for_contact = (use_mesh_env && std::atoi(use_mesh_env) != 0);
		if (log_vtk_tool && m_t && !(use_mesh_for_contact && b.get_fe_tool())) {
			vtk_writer_write(m_t, step, m_folder);
		}
		if (b.get_fe_tool()) {
			if (log_vtk_tool && use_mesh_for_contact) vtk_writer_write(b.get_fe_tool(), step, m_folder, "tool");
			if (log_vtk_fe_tool) vtk_writer_write(b.get_fe_tool(), step, m_folder);
		}
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
	m_fp_forces = fopen(forces.string().c_str(), "w+");
	m_fp_trace = fopen(trace.string().c_str(), "w+");
}
