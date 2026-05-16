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

#include "material.h"

#include <cstdlib>
#include <cstdio>

namespace {
static double env_positive_double_or(const char *name, double fallback) {
	const char *s = std::getenv(name);
	if (!s || s[0] == '\0')
		return fallback;

	char *end = nullptr;
	double v = std::strtod(s, &end);
	if (end == s || !std::isfinite(v) || v <= 0.)
		return fallback;
	return v;
}
} // namespace

void material_eos(body &b) {
	// Tension cutoff: read the limit once at first call (Pa; 0 = disabled).
	// When MFREE_TENSION_CUTOFF is set the EOS pressure is clamped so that
	// p >= -p_tensile_cutoff.  This breaks the positive-feedback loop that
	// occurs at free surfaces and chip boundaries:
	//   small density undershoot -> huge tensile p -> particle diverges ->
	//   lower density -> even larger tensile p -> divergent runaway.
	// Physically equivalent to a maximum-tensile-stress / tension-cutoff
	// fracture criterion.  For Ti-6Al-4V a value of 3e9 Pa (3 GPa, ~4xJC_A)
	// is above the UTS (~950 MPa) so normal tensile yielding is unaffected.
	static const double p_tensile_cutoff = []() -> double {
		const char *s = std::getenv("MFREE_TENSION_CUTOFF");
		if (!s || s[0] == '\0')
			return 0.0;
		char *end = nullptr;
		double v = std::strtod(s, &end);
		if (end == s || !std::isfinite(v) || v <= 0.)
			return 0.0;
		std::printf("[material_eos] tension cutoff enabled: %.6g Pa (%.4g GPa)\n", v, v * 1e-9);
		std::fflush(stdout);
		return v;
	}();

	std::vector<particle> &particles = b.get_particles();
	double K = b.get_sim_data().get_physical_constants().K();
	double rho0 = b.get_sim_data().get_physical_constants().rho0();

	const unsigned int n = b.get_num_part();
	const double c0 = sqrt(K / rho0);
	const double p_compress_cutoff = env_positive_double_or("MFREE_COMPRESSION_CUTOFF", 1.0e11);
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		const double raw_p = c0 * c0 * (particles[i].rho - rho0);
		particles[i].p = raw_p;
		// Clamp: no material sustains hundreds of GPa in tension.
		if (p_tensile_cutoff > 0. && particles[i].p < -p_tensile_cutoff)
			particles[i].p = -p_tensile_cutoff;

		if (p_compress_cutoff > 0. && particles[i].p > p_compress_cutoff) {
			particles[i].p = p_compress_cutoff;
			particles[i].Sxx = 0.;
			particles[i].Sxy = 0.;
			particles[i].Syy = 0.;
			particles[i].Szz = 0.;
			particles[i].Sxx_t = 0.;
			particles[i].Sxy_t = 0.;
			particles[i].Syy_t = 0.;
			particles[i].Szz_t = 0.;
			static int compression_warn_count = 0;
			if (compression_warn_count < 20) {
				compression_warn_count++;
				std::fprintf(stderr,
							 "[material_eos] WARNING: extreme pressure clamped for particle %u: "
							 "raw_p=%.6e Pa, p=%.6e Pa, rho=%.6e kg/m3 (rho_ratio=%.1e)\n",
							 i, raw_p, particles[i].p, particles[i].rho, particles[i].rho / rho0);
				std::fflush(stderr);
			}
		}
	}
}

void material_stress_rate_jaumann(body &b) {
	std::vector<particle> &particles = b.get_particles();
	double G = b.get_sim_data().get_physical_constants().G();

	const unsigned int n = b.get_num_part();
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		const glm::dmat3x3 epsdot = glm::dmat3x3(particles[i].vx_x, 0.5 * (particles[i].vx_y + particles[i].vy_x), 0.,
												 0.5 * (particles[i].vx_y + particles[i].vy_x), particles[i].vy_y, 0., 0., 0., 0.);
		const glm::dmat3x3 omega = glm::dmat3x3(0., 0.5 * (particles[i].vy_x - particles[i].vx_y), 0.,
												0.5 * (particles[i].vx_y - particles[i].vy_x), 0., 0., 0., 0., 0.);
		const glm::dmat3x3 S =
			glm::dmat3x3(particles[i].Sxx, particles[i].Sxy, 0., particles[i].Sxy, particles[i].Syy, 0., 0., 0., particles[i].Szz);
		const glm::dmat3x3 I = glm::dmat3x3(1.);

		const double trace_epsdot = epsdot[0][0] + epsdot[1][1] + epsdot[2][2];

		const glm::dmat3x3 S_t = 2 * G * (epsdot - 1. / 3. * trace_epsdot * I) + omega * S + S * glm::transpose(omega); // Belytschko
																														// (3.7.9)

		particles[i].Sxx_t += S_t[0][0];
		particles[i].Sxy_t += S_t[0][1];
		particles[i].Syy_t += S_t[1][1];
		particles[i].Szz_t += S_t[2][2];
	}
}
