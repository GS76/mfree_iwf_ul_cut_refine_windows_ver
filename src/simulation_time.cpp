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

#include "simulation_time.h"

simulation_time &simulation_time::getInstance() {
	static simulation_time instance;
	return instance;
}

double simulation_time::get_time() const { return m_time; }

double simulation_time::get_dt() const { return m_dt; }

simulation_time::simulation_time() : m_time(0.), m_dt(0.), m_t_final(0.), m_step(0) {}

void simulation_time::increment_time() { m_time += m_dt; }

void simulation_time::increment_step() { m_step++; }

void simulation_time::set_t_final(double t_final) { m_t_final = t_final; }

bool simulation_time::finished() const { return m_time > m_t_final; }

unsigned int simulation_time::get_step() const { return m_step; }

double simulation_time::get_t_final() const { return m_t_final; }

void simulation_time::set_dt(double dt) { 
	m_dt = dt; 
	// Store original timestep on first setting if not already set
	if (m_dt_original == 0.) {
		m_dt_original = dt;
	}
}

bool simulation_time::reduce_dt(double factor, double min_dt) {
	// Ensure factor is reasonable (between 0.1 and 0.9)
	if (factor < 0.1) factor = 0.1;
	if (factor > 0.9) factor = 0.9;
	
	double new_dt = m_dt * factor;
	
	// Check against absolute minimum
	if (new_dt < min_dt) {
		printf("ERROR: Timestep reduction would result in dt=%.3e, which is below minimum %.3e\n", 
			   new_dt, min_dt);
		return false;
	}
	
	m_dt = new_dt;
	m_dt_reduction_count++;
	
	printf("STABILITY: Timestep reduced by %.0f%% to dt=%.6e (reduction #%d)\n", 
		   (1.0-factor)*100.0, m_dt, m_dt_reduction_count);
	
	return true;
}

void simulation_time::restore_original_dt() {
	if (m_dt_original > 0.) {
		m_dt = m_dt_original;
		printf("STABILITY: Timestep restored to original value dt=%.6e\n", m_dt);
	}
	m_dt_reduction_count = 0;
}
