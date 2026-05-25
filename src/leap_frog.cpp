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

#include "leap_frog.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>

// Returns the density floor (kg/m³) for a given material.
// The floor is tied to each material's reference density rho0.
static double density_floor(double rho0) {
	if (!std::isfinite(rho0) || rho0 <= 0.)
		return 1e-12;
	return rho0;
}

static void zero_stress_state(particle &p, bool zero_rates) {
	p.Sxx = 0.;
	p.Sxy = 0.;
	p.Syy = 0.;
	p.Szz = 0.;
	if (zero_rates) {
		p.Sxx_t = 0.;
		p.Sxy_t = 0.;
		p.Syy_t = 0.;
		p.Szz_t = 0.;
	}
}

static bool stress_state_non_finite(const particle &p) {
	return !std::isfinite(p.Sxx) || !std::isfinite(p.Sxy) || !std::isfinite(p.Syy) || !std::isfinite(p.Szz);
}

static void sanitize_particle_state(particle &p, const particle &init, double rho_min, double rho_max, double T_min, bool &rho_clamped) {
	rho_clamped = false;

	if (!std::isfinite(p.x)) {
		p.x = init.x;
		p.x_t = 0.;
	}
	if (!std::isfinite(p.y)) {
		p.y = init.y;
		p.y_t = 0.;
	}

	const double rho_floor = (rho_min > 0.) ? rho_min : 1e-12;
	if (!std::isfinite(p.rho) || !(p.rho > 0.)) {
		if (std::isfinite(init.rho) && init.rho > rho_floor) {
			p.rho = init.rho;
		} else {
			p.rho = rho_floor;
		}
		p.rho_t = 0.;
		rho_clamped = true;
	}
	if (p.rho < rho_floor) {
		p.rho = rho_floor;
		p.rho_t = 0.;
		rho_clamped = true;
	} else if (std::isfinite(rho_max) && p.rho > rho_max) {
		p.rho = rho_max;
		p.rho_t = 0.;
		rho_clamped = true;
	}

	if (!std::isfinite(p.h) || !(p.h > 0.)) {
		if (std::isfinite(init.h) && init.h > 0.) {
			p.h = init.h;
		} else {
			p.h = 1e-12;
		}
		p.h_t = 0.;
	}

	if (!std::isfinite(p.vx)) {
		p.vx = init.vx;
		p.vx_t = 0.;
	}
	if (!std::isfinite(p.vy)) {
		p.vy = init.vy;
		p.vy_t = 0.;
	}

	if (!std::isfinite(p.T)) {
		if (std::isfinite(init.T)) {
			p.T = init.T;
		} else {
			p.T = T_min;
		}
		p.T_t = 0.;
	}
	if (p.T < T_min) {
		p.T = T_min;
	}
}

void leap_frog::init(body &body) {
	std::vector<particle> &particles = body.get_particles();

	// this can only grow the vector since now coarsening strategy is not applied.
	// resize should not cause re-allocation since the constructor reserves twice the initial particle number,
	// so while this growth strategy is not perfect it should not or rarely have a runtime impact
	if (m_init.size() < particles.size()) {
		m_init.resize(particles.size());
	}

	const unsigned int n = body.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		m_init[i] = particles[i];
	}
}

void leap_frog::predict(body &body) const {
	std::vector<particle> &particles = body.get_particles();
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();

	const double rho0 = body.get_sim_data().get_physical_constants().rho0();
	const double rho_min = density_floor(rho0);
	const double rho_max = 1.5 * rho0; // Safeguard: clamp extreme compression from adaptivity/contact anomalies
	const double T_min = body.get_sim_data().get_physical_constants().jc().Tref();

	const unsigned int n = body.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		particles[i].x = m_init[i].x + 0.5 * dt * particles[i].x_t;
		particles[i].y = m_init[i].y + 0.5 * dt * particles[i].y_t;
		particles[i].rho = m_init[i].rho + 0.5 * dt * particles[i].rho_t;
		particles[i].h = m_init[i].h + 0.5 * dt * particles[i].h_t;
		particles[i].vx = m_init[i].vx + 0.5 * dt * particles[i].vx_t;
		particles[i].vy = m_init[i].vy + 0.5 * dt * particles[i].vy_t;
		particles[i].Sxx = m_init[i].Sxx + 0.5 * dt * particles[i].Sxx_t;
		particles[i].Sxy = m_init[i].Sxy + 0.5 * dt * particles[i].Sxy_t;
		particles[i].Syy = m_init[i].Syy + 0.5 * dt * particles[i].Syy_t;
		particles[i].Szz = m_init[i].Szz + 0.5 * dt * particles[i].Szz_t;
		particles[i].T = m_init[i].T + 0.5 * dt * particles[i].T_t;

		bool rho_clamped = false;
		sanitize_particle_state(particles[i], m_init[i], rho_min, rho_max, T_min, rho_clamped);

		if (rho_clamped || stress_state_non_finite(particles[i])) {
			zero_stress_state(particles[i], true);
		}

		// Clamp extreme deviatoric stresses to prevent pathological behavior downstream
		// (thermal instability, contact divergence, etc.). Threshold 1e15 Pa is ~1e6× typical yield.
		double norm_S = sqrt(particles[i].Sxx * particles[i].Sxx + particles[i].Syy * particles[i].Syy +
							 2.0 * particles[i].Sxy * particles[i].Sxy + particles[i].Szz * particles[i].Szz);
		if (!std::isfinite(norm_S) || norm_S > 1e15) {
			zero_stress_state(particles[i], true);
		}
	}
}

void leap_frog::correct(body &body) const {
	std::vector<particle> &particles = body.get_particles();
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();

	const double rho0 = body.get_sim_data().get_physical_constants().rho0();
	const double rho_min = density_floor(rho0);
	const double rho_max = 1.5 * rho0;
	const double T_min = body.get_sim_data().get_physical_constants().jc().Tref();

	const unsigned int n = body.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		particles[i].x = m_init[i].x + dt * particles[i].x_t;
		particles[i].y = m_init[i].y + dt * particles[i].y_t;
		particles[i].rho = m_init[i].rho + dt * particles[i].rho_t;
		particles[i].h = m_init[i].h + dt * particles[i].h_t;
		particles[i].vx = m_init[i].vx + dt * particles[i].vx_t;
		particles[i].vy = m_init[i].vy + dt * particles[i].vy_t;
		particles[i].Sxx = m_init[i].Sxx + dt * particles[i].Sxx_t;
		particles[i].Sxy = m_init[i].Sxy + dt * particles[i].Sxy_t;
		particles[i].Syy = m_init[i].Syy + dt * particles[i].Syy_t;
		particles[i].Szz = m_init[i].Szz + dt * particles[i].Szz_t;
		particles[i].T = m_init[i].T + dt * particles[i].T_t;
		bool rho_clamped = false;
		sanitize_particle_state(particles[i], m_init[i], rho_min, rho_max, T_min, rho_clamped);

		if (rho_clamped || stress_state_non_finite(particles[i])) {
			zero_stress_state(particles[i], true);
		}

		// Clamp extreme deviatoric stresses to prevent pathological behavior downstream
		double norm_S = sqrt(particles[i].Sxx * particles[i].Sxx + particles[i].Syy * particles[i].Syy +
							 particles[i].Szz * particles[i].Szz + 2.0 * particles[i].Sxy * particles[i].Sxy);
		if (!std::isfinite(norm_S) || norm_S > 1e15) {
			zero_stress_state(particles[i], false);
		}
	}
}

void leap_frog::step(body &body) {

	// Update the neighbors by spatial hashing
	body.construct_verlet_lists();

	// Leapfrog predictor step
	init(body);
	predict(body);

	// compute temporal derivatives
	{
		std::vector<particle> &particles = body.get_particles();
		const unsigned int n = body.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
		for (int ii = 0; ii < static_cast<int>(n); ii++) {
			const unsigned int i = static_cast<unsigned int>(ii);
			particles[i].reset();
		}
	}

	// move the tool & do penalty contact
	body.apply_contact();
	body.advance_fe_tool_mechanics_explicit();
	body.move_tool();

	// Compute time derivatives of physical properties
	// =================================================
	material_eos(body);
	correctors_mghn_artificial_stress(body);
	derive_stress_monaghan(body);
	derive_velocity(body);
	correctors_mghn_artificial_viscosity(body);
	correctors_xsph(body);
	material_stress_rate_jaumann(body);
	contmech_continuity(body);
	contmech_momentum(body);
	contmech_advection(body);
	// =================================================

	// Solve heat equation
	body.apply_thermal_conduction();
	body.advance_fe_tool_thermal();

	// Leapfrog corrector step
	correct(body);

	// Perform plasticity by radial return
	body.apply_plasticity();

	// boundary conditions
	do_boundary_conditions(body);

	// restore particles into their original order
	// this step is not necessary for correctness but may be useful for debugging purposes
	// deactivated for performance reasons
	body.restore_order();

	body.apply_adaptivity();
}

leap_frog::leap_frog(unsigned int num_part) { m_init.reserve(2 * num_part); }
