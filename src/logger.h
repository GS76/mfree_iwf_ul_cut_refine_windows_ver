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

#ifndef LOGGER_H_
#define LOGGER_H_

#include "fe_tool.h"
#include "body.h"
#include "vtk_writer.h"

#include <vector>

/*
  Logging for visualization purposes
  ------------------------------------------------
  The logger file supports:
	1. simple text representation of tool
	2. forces on tool
	3. textual "vtk" files for particle attributes
  ------------------------------------------------
*/

class logger {

  private:
	bool m_log_forces = true;
	bool m_emit_vtk = true;

	fe_tool *m_t = 0;
	FILE *m_fp_forces = 0;
	FILE *m_fp_trace = 0;
	FILE *m_fp_thermal = 0;
	FILE *m_fp_metrics = 0;
	FILE *m_fp_energy = 0;
	// Reference temperature for above-ref energy computations (K).
	// Read once from MFREE_THERMAL_T_REF; default 298.15 K (25 degC).
	double m_T_ref = 298.15;
	// Baseline above-ref internal energies captured on the first logged step.
	// Negative sentinel means "not yet captured".
	double m_wp_internal_E_init = -1.;
	double m_tool_internal_E_init = -1.;
	// Running total of Taylor-Quinney plastic dissipation energy (J).
	double m_cum_plastic_dissipation = 0.;
	// Set to true after the first step where step_suppression_ratio > 0.10 so
	// the console warning is emitted at most once per results folder.
	bool m_suppression_warned = false;
	double m_cum_contact_E_cond_raw = 0.;
	double m_cum_contact_E_fric_raw = 0.;
	double m_cum_contact_E_cond_scaled = 0.;
	double m_cum_contact_E_fric_scaled = 0.;
	double m_cum_contact_E_workpiece = 0.;
	double m_cum_contact_E_tool = 0.;
	double m_cum_contact_E_limiter_suppressed = 0.;
	double m_cum_tool_E_sources = 0.;
	double m_cum_tool_E_conduction = 0.;
	double m_cum_tool_E_convection = 0.;
	double m_cum_tool_E_dirichlet = 0.;
	std::vector<unsigned int> m_trace_p;
	char m_folder[256] = "results";
	char m_case_name[256] = "case";

  public:
	logger(const char *case_name, const char *foldername = "results");
	void close();

	void set_fe_tool(fe_tool *t);
	void set_log_forces(bool log_forces);
	void set_log_vtk(bool log_vtk);
	void add_tracer_particle(unsigned int tracer_idx);
	void set_folder(const char *folder);

	void log(const body &body, unsigned int step);
	void log_time_step_data(const body &body, unsigned int step);

	// Must be called every solver step (after apply_plasticity) regardless of
	// logging frequency.  Accumulates Taylor-Quinney plastic dissipation so
	// cum_plastic_dissipation in the energy CSV is always correct.
	void accumulate_plastic_dissipation(const body &b);

  private:
	// Shared energy-accounting block called by both log() and log_time_step_data().
	void log_energy_block(const body &b, unsigned int step, const fe_tool *ft_log);
};

#endif /* LOGGER_H_ */
