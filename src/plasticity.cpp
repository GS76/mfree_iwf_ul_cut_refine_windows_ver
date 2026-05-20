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

#include "plasticity.h"
#include "body.h"
#include <cmath>

double plasticity::plastic_state_by_radial_return(body &b) {
	if (!b.get_sim_data().get_physical_constants().jc().valid())
		return 0.;
	return do_radial_return(b.get_particles(), b.get_num_part(), b.get_sim_data());
}

void plasticity::set_tolerance(double tol) { m_tol = tol; }

void plasticity::set_dissipation_considered(bool consider) { m_consider_dissipation = consider; }

plasticity::plasticity(johnson_cook_Sima_2010 *plasticity_model) { m_plasticity_model = plasticity_model; }

plasticity::plasticity() {}

void plasticity::print_debug(const std::vector<particle> &particles, unsigned int num_part, unsigned int fail_idx) {
	FILE *fp = fopen("plast_debug.txt", "w+");
	fprintf(fp, "%f %f\n", particles[fail_idx].x, particles[fail_idx].y);
	for (unsigned int i = 0; i < num_part; i++) {
		fprintf(fp, "%f %f\n", particles[i].x, particles[i].y);
	}
	fclose(fp);
}

double plasticity::do_radial_return(std::vector<particle> &particles, unsigned int num_part, simulation_data data) { // 2D
	simulation_time *time = &simulation_time::getInstance();
	double delta_t = time->get_dt();
	double mu = data.get_physical_constants().G();

	double cp = data.get_physical_constants().tc().cp();
	double tq = data.get_physical_constants().tc().Taylor_Quinney();

	double step_plastic_dissipation = 0.;

	double rho0 = data.get_physical_constants().rho0();
	double rho_min = (std::isfinite(rho0) && rho0 > 0.) ? rho0 : 0.;

	for (unsigned int i = 0; i < num_part; i++) {
		// Skip plasticity entirely for density-floored particles: their stress state is artificial
		if (rho_min > 0. && std::isfinite(particles[i].rho) && particles[i].rho < rho_min) {
			// Zero the stress state for particles that hit the density floor.
			// Without this, old trial stresses (computed with pre-floor density)
			// can become NaN/Inf when later combined with the floored density.
			particles[i].Sxx = particles[i].Syy = particles[i].Szz = particles[i].Sxy = 0.;
			particles[i].Sxx_t = particles[i].Syy_t = particles[i].Szz_t = particles[i].Sxy_t = 0.;
			particles[i].eps_pl_equiv_dot = 0.;
			continue;
		}
		// deviatoric stress (trial)
		double Strialxx = particles[i].Sxx;
		double Strialyy = particles[i].Syy;
		double Strialzz = particles[i].Szz;
		double Strialxy = particles[i].Sxy;

		// Defensive guard: catch NaN/Inf or non-finite pressure before any math that would propagate NaN
		if (!std::isfinite(Strialxx) || !std::isfinite(Strialyy) || !std::isfinite(Strialzz) || !std::isfinite(Strialxy) ||
			!std::isfinite(particles[i].p)) {
			static int nan_count = 0;
			nan_count++;
			if (nan_count <= 50) {
				fprintf(stderr, "WARNING: NaN/Inf in Strial or p for particle %u, zeroing stresses and skipping plasticity\n", i);
				fprintf(stderr, "  x=(%.6e, %.6e), rho=%.6e, T=%.6e, p=%.6e\n", particles[i].x, particles[i].y, particles[i].rho,
						particles[i].T, particles[i].p);
			}
			// Zero stresses and time rates
			particles[i].Sxx = particles[i].Sxy = particles[i].Syy = particles[i].Szz = 0.;
			particles[i].Sxx_t = particles[i].Sxy_t = particles[i].Syy_t = particles[i].Szz_t = 0.;
			particles[i].eps_pl_equiv_dot = 0.;
			// Append compact diagnostic to plast_debug.txt for post-mortem
			FILE *fp = fopen("plast_debug.txt", "a");
			if (fp) {
				fprintf(fp, "%u %.6e %.6e %.6e %.6e %.6e\n", i, particles[i].x, particles[i].y, particles[i].rho, particles[i].T,
						particles[i].p);
				fclose(fp);
			}
			continue;
		}

		double norm_Strial = sqrt(Strialxx * Strialxx + Strialyy * Strialyy + Strialzz * Strialzz + 2. * Strialxy * Strialxy);

		// Skip plasticity for particles with extreme stress states (>= 1e15 Pa ~ 1e6× yield)
		// These occur when density-floored particles carry over stress from before the floor.
		if (norm_Strial > 1e15) {
			static int extreme_count = 0;
			extreme_count++;
			if (extreme_count <= 10) {
				fprintf(stderr, "WARNING: extreme norm_Strial (%.2e > 1e15) for particle %u, skipping plasticity\n", norm_Strial, i);
				fprintf(stderr, "  x=(%.6e, %.6e), rho=%.6e, T=%.6e\n", particles[i].x, particles[i].y, particles[i].rho, particles[i].T);
			}
			particles[i].eps_pl_equiv_dot = 0.;
			continue;
		}

		// cauchy stress (trial)
		double cxx = particles[i].Sxx - particles[i].p;
		double cyy = particles[i].Syy - particles[i].p;
		double czz = particles[i].Szz - particles[i].p;
		double cxy = particles[i].Sxy;

		double eps_pl_equiv_init = particles[i].eps_pl_equiv;
		double eps_pl_equiv_init_dot = particles[i].eps_pl_equiv_dot;

		double svm2 = (cxx * cxx + cyy * cyy + czz * czz) - cxx * cyy - cxx * czz - cyy * czz + 3.0 * cxy * cxy;
		if (svm2 < 0.0)
			svm2 = 0.0;
		double svm = sqrt(svm2);

		double sigmaY = m_plasticity_model->sigma_yield(eps_pl_equiv_init, eps_pl_equiv_init_dot, particles[i].T);

		if (svm < sigmaY) {
			// in other words: elastic domain!
			particles[i].eps_pl_equiv_dot = 0.;
			continue;
		}

		double delta_lambda = 0.; // delta lambda = \dot{lambda}\delta t, NOT lambda_new - lambda_old !!!1

		m_plasticity_model->set_eps_init(eps_pl_equiv_init);
		m_plasticity_model->set_temp(particles[i].T);
		m_plasticity_model->set_norm_s_trial(norm_Strial);

		bool failed = false;
		delta_lambda =
			solve_zero_secant(m_plasticity_model, fmax(particles[i].eps_pl_equiv_dot * delta_t * sqrt(2. / 3.), 1e-8), m_tol, failed);
		if (failed) {
			// Graceful handling: log warning and skip plasticity for this particle
			static int fail_count = 0;
			fail_count++;

			fprintf(stderr, "WARNING: radial return failed (count=%d) for particle %u\n", fail_count, i);
			fprintf(stderr, "  x=(%.6e, %.6e), rho=%.6e, T=%.6e\n", particles[i].x, particles[i].y, particles[i].rho, particles[i].T);
			fprintf(stderr, "  eps_pl_equiv=%.6e, eps_pl_equiv_dot=%.6e\n", particles[i].eps_pl_equiv, particles[i].eps_pl_equiv_dot);
			fprintf(stderr, "  Sxx=%.6e, Syy=%.6e, Szz=%.6e, Sxy=%.6e, p=%.6e\n", particles[i].Sxx, particles[i].Syy, particles[i].Szz,
					particles[i].Sxy, particles[i].p);
			fprintf(stderr, "  Strialxx=%.6e, Strialyy=%.6e, Strialzz=%.6e, Strialxy=%.6e\n", Strialxx, Strialyy, Strialzz, Strialxy);
			fprintf(stderr, "  norm_Strial=%.6e\n", norm_Strial);

			// Write debug info to file for post-processing
			print_debug(particles, num_part, i);

			// Skip plasticity update - keep particle in previous state
			particles[i].eps_pl_equiv_dot = 0.;

			// If too many failures, escalate warning but continue (allow data collection)
			if (fail_count > 100) {
				fprintf(stderr, "ERROR: Too many radial return failures (%d); continuing anyway for data collection\n", fail_count);
			}

			continue;
		}
		double eps_pl_new = eps_pl_equiv_init + sqrt(2.0 / 3.0) * fmax(delta_lambda, 0.);
		double delta_eps_pl = eps_pl_new - particles[i].eps_pl_equiv;

		particles[i].eps_pl_equiv = eps_pl_new;
		particles[i].eps_pl_equiv_dot = sqrt(2.0 / 3.0) * fmax(delta_lambda, 0.) / delta_t;

		particles[i].eps_plxx = Strialxx / norm_Strial * delta_lambda / delta_t;
		particles[i].eps_plxy = Strialxy / norm_Strial * delta_lambda / delta_t;
		particles[i].eps_plyy = Strialyy / norm_Strial * delta_lambda / delta_t;
		particles[i].eps_plzz = Strialzz / norm_Strial * delta_lambda / delta_t;

		particles[i].Sxx = Strialxx - Strialxx / norm_Strial * delta_lambda * 2. * mu;
		particles[i].Syy = Strialyy - Strialyy / norm_Strial * delta_lambda * 2. * mu;
		particles[i].Szz = Strialzz - Strialzz / norm_Strial * delta_lambda * 2. * mu;
		particles[i].Sxy = Strialxy - Strialxy / norm_Strial * delta_lambda * 2. * mu;

		if (m_consider_dissipation) {

			/*
			Temperature increase due to plastic dissipation (Taylor-Quinney)
			refer to --> Eq. (9) of the paper
			*/

			// Skip particles with invalid density to avoid spurious heating
			if (particles[i].rho <= 0. || !std::isfinite(particles[i].rho)) {
				continue;
			}

			double sigmaY = m_plasticity_model->sigma_yield(particles[i].eps_pl_equiv, particles[i].eps_pl_equiv_dot, particles[i].T);
			double delta_T = tq / (cp * particles[i].rho) * delta_eps_pl * sigmaY;
			particles[i].T += delta_T;

			// Accumulate dissipation energy: E = delta_T * m * cp
			if (std::isfinite(delta_T) && std::isfinite(particles[i].m) && particles[i].m > 0.)
				step_plastic_dissipation += delta_T * particles[i].m * cp;
		}
	}
	return step_plastic_dissipation;
}
