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

#include "thermal.h"
#include <cmath>
#include "body.h"
#include <omp.h>

void thermal::set_convection(double h_W_m2K, double T_ambient_K) {
	m_h_W_m2K = h_W_m2K;
	m_T_ambient_K = T_ambient_K;
}

void thermal::set_convection_enabled(bool enabled) { m_convection_enabled = enabled; }

void thermal::set_max_cooling_rate(double max_rate_K_per_s) { m_max_rate_K_per_s = max_rate_K_per_s; }

double thermal::last_max_abs_rate_K_per_s() const { return m_last_max_abs_rate_K_per_s; }

double thermal::last_convection_ramp() const { return m_last_convection_ramp; }

void thermal::heat_conduction_pse(body &b) const {
	std::vector<particle> &particles = b.get_particles();
	unsigned int num_part = b.get_num_part();
	const auto &phys_const = b.get_sim_data().get_physical_constants();

#pragma omp parallel for
	for (unsigned int i = 0; i < num_part; i++) {
		const double Ti = particles[i].T;
		const double xi = particles[i].x;
		const double yi = particles[i].y;
		const double hi = particles[i].h;
		const double hi2 = hi * hi;

		double T_lapl = 0.;

		for (unsigned int j = 0; j < particles[i].num_nbh; j++) {
			unsigned int jdx = particles[i].nbh[j];

			const double Tj = particles[jdx].T;
			const double xj = particles[jdx].x;
			const double yj = particles[jdx].y;
			const double mj = particles[jdx].m;
			const double rhoj = particles[jdx].rho;

			const double xij = xi - xj;
			const double yij = yi - yj;

			const double r = sqrt(xij * xij + yij * yij);
			const double w_pse = 4.0 / (hi2 * M_PI) * exp(-r * r / (hi2));
			T_lapl += (Tj - Ti) * w_pse * mj / rhoj / (hi2);
		}

		double denominator = phys_const.rho0(Ti) * phys_const.tc().cp(Ti);
		if (denominator < 1e-12) {
			particles[i].T_t = 0;
			continue;
		}
		double alpha = phys_const.tc().k(Ti) / denominator;
		particles[i].T_t += alpha * T_lapl;
	}
}

void thermal::heat_conduction_brookshaw(body &b) const {
	std::vector<particle> &particles = b.get_particles();
	unsigned int num_part = b.get_num_part();
	const auto &phys_const = b.get_sim_data().get_physical_constants();

#pragma omp parallel for
	for (unsigned int i = 0; i < num_part; i++) {
		double Ti = particles[i].T;
		double xi = particles[i].x;
		double yi = particles[i].y;

		double T_lapl = 0.;

		for (unsigned int j = 0; j < particles[i].num_nbh; j++) {
			unsigned int jdx = particles[i].nbh[j];
			kernel_result w = particles[i].w[j];

			if (particles[i].idx == particles[jdx].idx) {
				continue;
			}

			double Tj = particles[jdx].T;
			double xj = particles[jdx].x;
			double yj = particles[jdx].y;
			double mj = particles[jdx].m;
			double rhoj = particles[jdx].rho;

			double xij = xi - xj;
			double yij = yi - yj;

			double rij = sqrt(xij * xij + yij * yij);
			double eijx = xij / rij;
			double eijy = yij / rij;

			if (rij < 1e-12)
				continue;

			double rij1 = 1. / rij;

			T_lapl += 2.0 * (mj / rhoj) * (Ti - Tj) * rij1 * (eijx * w.w_x + eijy * w.w_y);
		}

		double alpha = phys_const.tc().k(Ti) / (phys_const.rho0(Ti) * phys_const.tc().cp(Ti));
		particles[i].T_t += alpha * T_lapl;
	}
}

void thermal::apply_convection(body &b) const {
	if (!m_convection_enabled)
		return;
	if (m_h_W_m2K <= 0.0)
		return;

	std::vector<particle> &particles = b.get_particles();
	unsigned int num_part = b.get_num_part();
	const auto &phys_const = b.get_sim_data().get_physical_constants();

	double max_abs_rate = 0.0;
#pragma omp parallel for reduction(max : max_abs_rate)
	for (unsigned int i = 0; i < num_part; i++) {
		const double Ti = particles[i].T;
		const double rho0 = phys_const.rho0(Ti);
		const double cp = phys_const.tc().cp(Ti);
		const double denom = rho0 * cp;
		if (denom <= 1e-12)
			continue;

		const double dV = particles[i].m / rho0;
		if (dV <= 1e-18)
			continue;

		const double A_over_V = 4.0 / std::sqrt(dV);
		const double beta = m_h_W_m2K * A_over_V / denom;
		const double rate = std::abs(beta * (Ti - m_T_ambient_K));
		if (rate > max_abs_rate)
			max_abs_rate = rate;
	}

	double ramp = 1.0;
	if (m_max_rate_K_per_s > 0.0 && max_abs_rate > m_max_rate_K_per_s) {
		ramp = m_max_rate_K_per_s / max_abs_rate;
	}

	m_last_convection_ramp = ramp;

#pragma omp parallel for
	for (unsigned int i = 0; i < num_part; i++) {
		const double Ti = particles[i].T;
		const double rho0 = phys_const.rho0(Ti);
		const double cp = phys_const.tc().cp(Ti);
		const double denom = rho0 * cp;
		if (denom <= 1e-12)
			continue;

		const double dV = particles[i].m / rho0;
		if (dV <= 1e-18)
			continue;

		const double A_over_V = 4.0 / std::sqrt(dV);
		const double beta = m_h_W_m2K * A_over_V / denom;
		particles[i].T_t += -ramp * beta * (Ti - m_T_ambient_K);
	}
}

void thermal::enforce_rate_limit(body &b) const {
	if (m_max_rate_K_per_s <= 0.0)
		return;

	std::vector<particle> &particles = b.get_particles();
	unsigned int num_part = b.get_num_part();

	double max_abs_rate = 0.0;
#pragma omp parallel for reduction(max : max_abs_rate)
	for (unsigned int i = 0; i < num_part; i++) {
		const double rate = std::abs(particles[i].T_t);
		if (rate > max_abs_rate)
			max_abs_rate = rate;
	}

	m_last_max_abs_rate_K_per_s = max_abs_rate;

	if (max_abs_rate <= m_max_rate_K_per_s)
		return;

	const double scale = m_max_rate_K_per_s / max_abs_rate;
#pragma omp parallel for
	for (unsigned int i = 0; i < num_part; i++) {
		particles[i].T_t *= scale;
	}

	m_last_max_abs_rate_K_per_s = m_max_rate_K_per_s;
}

void thermal::set_method(thermal_solver solver) { m_thermal_solver = solver; }

void thermal::conduction(body &body) const {

	switch (m_thermal_solver) {
	case thermal_pse:
		heat_conduction_pse(body);
		break;
	case thermal_brookshaw:
		heat_conduction_brookshaw(body);
		break;
	}

	apply_convection(body);
	enforce_rate_limit(body);
}

thermal::thermal(physical_constants pc) { assert(pc.tc().k() != 0.); }
