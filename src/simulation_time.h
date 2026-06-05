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

#ifndef SIMULATION_TIME_H_
#define SIMULATION_TIME_H_

#include <stdio.h>

/*
 In order to have one single interface through which all temporal
 attributes can be accessed globally.

 It saves:
 time, time step, time step number, t final, etc.
*/

class simulation_time {

  public:
	static simulation_time &getInstance();
	simulation_time(simulation_time const &) = delete;
	void operator=(simulation_time const &) = delete;
	double get_time() const;
	double get_dt() const;
	bool finished() const;
	unsigned int get_step() const;
	void increment_time();
	void increment_step();
	double get_t_final() const;

	void set_t_final(double t_final);
	void set_dt(double dt);

	double get_CFL() const;
	void modify_dt(double dt_adapted);

	// Adaptive timestep control for stability monitoring
	// Reduce timestep by a factor (e.g., 0.5 for 50% reduction)
	// Returns false if timestep would fall below minimum
	bool reduce_dt(double factor, double min_dt);
	
	// Restore the original timestep (used when stability is recovered)
	void restore_original_dt();
	
	// Get the original (initial) timestep
	double get_original_dt() const { return m_dt_original; }
	
	// Get the number of reductions performed
	int get_dt_reduction_count() const { return m_dt_reduction_count; }
	
	// Get the minimum timestep allowed
	double get_min_timestep() const { return m_dt_min; }
	
	// Set the minimum timestep allowed
	void set_min_timestep(double min_dt) { m_dt_min = min_dt; }

  private:
	simulation_time();
	double m_time = 0.;
	double m_dt = 0.;
	double m_t_final = 0.;         // Final simulation time
	double m_dt_original = 0.;     // Original timestep (before any reductions)
	double m_dt_min = 1e-12;       // Minimum allowed timestep
	unsigned int m_step = 0;
	int m_dt_reduction_count = 0;  // Track how many times we've reduced
};

#endif /* SIMULATION_TIME_H_ */
