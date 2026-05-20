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

#include "cont_mech.h"
#include <cmath>

void contmech_continuity(body &b) {
	std::vector<particle> &particles = b.get_particles();

	const unsigned int n = b.get_num_part();
	const double rho0 = b.get_sim_data().get_physical_constants().rho0();
	const double rho_min = (std::isfinite(rho0) && rho0 > 0.) ? rho0 : 1e-12;
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		const double rho = particles[i].rho;
		const double vx_x = particles[i].vx_x;
		const double vy_y = particles[i].vy_y;
		if (!std::isfinite(rho) || rho < rho_min || !std::isfinite(vx_x) || !std::isfinite(vy_y))
			continue;

		particles[i].rho_t -= rho * (vx_x + vy_y);
	}
}

void contmech_momentum(body &b) {
	std::vector<particle> &particles = b.get_particles();

	const unsigned int n = b.get_num_part();
	const double rho0 = b.get_sim_data().get_physical_constants().rho0();
	const double rho_min = (std::isfinite(rho0) && rho0 > 0.) ? rho0 : 1e-12;
	const double m_min = 1e-18;
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		const double Sxx_x = particles[i].Sxx_x;
		const double Sxy_y = particles[i].Sxy_y;
		const double Sxy_x = particles[i].Sxy_x;
		const double Syy_y = particles[i].Syy_y;

		const double rho = particles[i].rho;
		const double m = particles[i].m;
		if (!std::isfinite(Sxx_x) || !std::isfinite(Sxy_y) || !std::isfinite(Sxy_x) || !std::isfinite(Syy_y))
			continue;
		if (!std::isfinite(rho) || rho < rho_min || !std::isfinite(m) || m <= m_min)
			continue;
		if (!std::isfinite(particles[i].fcx) || !std::isfinite(particles[i].fcy) || !std::isfinite(particles[i].ftx) ||
			!std::isfinite(particles[i].fty))
			continue;

		const double inv_rho = 1. / rho;
		const double inv_m = 1. / m;
		particles[i].vx_t += inv_rho * (Sxx_x + Sxy_y) + (particles[i].fcx + particles[i].ftx) * inv_m;
		particles[i].vy_t += inv_rho * (Sxy_x + Syy_y) + (particles[i].fcy + particles[i].fty) * inv_m;
	}
}

void contmech_advection(body &b) {
	std::vector<particle> &particles = b.get_particles();

	const unsigned int n = b.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		if (!std::isfinite(particles[i].vx) || !std::isfinite(particles[i].vy))
			continue;
		particles[i].x_t += particles[i].vx;
		particles[i].y_t += particles[i].vy;
	}
}

void do_boundary_conditions(body &b) {
	std::vector<particle> &particles = b.get_particles();

	// this enforces the fixed boundary conditions
	// demonstrated in Fig. 10 of the manuscript

	const unsigned int n = b.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		if (particles[i].fixed) {
			particles[i].x = particles[i].X;
			particles[i].y = particles[i].Y;
			particles[i].x_t = 0.;
			particles[i].y_t = 0.;
			particles[i].vx = 0.;
			particles[i].vy = 0.;
			particles[i].vx_t = 0.;
			particles[i].vy_t = 0.;
			// Dirichlet thermal BC: fixed boundary particles (bottom row, right column)
			// represent far-field bulk material and must remain at ambient temperature.
			// Without this, the PSE heat equation freely advances T each step and the
			// boundary acts as an insulating wall, creating spurious thermal pockets.
			// T_init holds the initial/ambient temperature (300 K) set at particle
			// creation and propagated to refined child particles by copy_dad_to_son.
			particles[i].T = particles[i].T_init; // hold at ambient temperature
			particles[i].T_t = 0.;				  // zero rate so next predict step is clean
		}
	}
}
