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
#include <algorithm>
#include <cctype>
#include <vector>

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

struct wp_mech_tables {
	std::vector<double> E_T;
	std::vector<double> E_v;
	std::vector<double> G_T;
	std::vector<double> G_v;
	bool parsed = false;
};

static bool parse_env_table(const char *key, std::vector<double> &Tout, std::vector<double> &Vout) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0')
		return false;
	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p && (std::isspace(static_cast<unsigned char>(*p)) || *p == ',' || *p == ';'))
			++p;
		if (!*p)
			break;
		char *end = nullptr;
		double T = std::strtod(p, &end);
		if (end == p || !std::isfinite(T))
			return false;
		p = end;
		while (*p && std::isspace(static_cast<unsigned char>(*p)))
			++p;
		if (*p != ':' && *p != '=')
			return false;
		++p;
		while (*p && std::isspace(static_cast<unsigned char>(*p)))
			++p;
		end = nullptr;
		double v = std::strtod(p, &end);
		if (end == p || !std::isfinite(v))
			return false;
		p = end;
		pairs.push_back({T, v});
	}
	if (pairs.size() < 2)
		return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
	Tout.clear();
	Vout.clear();
	for (const auto &kv : pairs) {
		if (!Tout.empty() && kv.first == Tout.back()) {
			Vout.back() = kv.second;
			continue;
		}
		Tout.push_back(kv.first);
		Vout.push_back(kv.second);
	}
	return Tout.size() >= 2;
}

static double table_eval(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback) {
	if (T_tab.size() < 2 || T_tab.size() != v_tab.size() || !std::isfinite(T))
		return fallback;
	if (T <= T_tab.front())
		return v_tab.front();
	if (T >= T_tab.back())
		return v_tab.back();
	auto it = std::upper_bound(T_tab.begin(), T_tab.end(), T);
	std::size_t i1 = static_cast<std::size_t>(it - T_tab.begin());
	if (i1 == 0 || i1 >= T_tab.size())
		return fallback;
	std::size_t i0 = i1 - 1;
	double T0 = T_tab[i0];
	double T1 = T_tab[i1];
	double v0 = v_tab[i0];
	double v1 = v_tab[i1];
	double dT = T1 - T0;
	if (!(dT > 0.))
		return fallback;
	double a = (T - T0) / dT;
	return (1.0 - a) * v0 + a * v1;
}

static const wp_mech_tables &get_wp_mech_tables() {
	static wp_mech_tables tbl;
	if (!tbl.parsed) {
		parse_env_table("MFREE_WP_E_TABLE", tbl.E_T, tbl.E_v);
		parse_env_table("MFREE_WP_G_TABLE", tbl.G_T, tbl.G_v);
		tbl.parsed = true;
	}
	return tbl;
}

static double workpiece_G_at(double T, double G0, double nu0) {
	const wp_mech_tables &tbl = get_wp_mech_tables();
	double G = table_eval(T, tbl.G_T, tbl.G_v, G0);
	if (!std::isfinite(G) || G <= 0.) {
		double E = table_eval(T, tbl.E_T, tbl.E_v, 2.0 * (1.0 + nu0) * G0);
		if (std::isfinite(E) && E > 0. && std::isfinite(nu0) && (1.0 + nu0) > 0.)
			G = E / (2.0 * (1.0 + nu0));
	}
	if (!std::isfinite(G) || G <= 0.)
		G = G0;
	return G;
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
	const auto pc = b.get_sim_data().get_physical_constants();
	const double G0 = pc.G();
	const double nu0 = pc.nu();

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

		const double G = workpiece_G_at(particles[i].T, G0, nu0);
		const glm::dmat3x3 S_t = 2 * G * (epsdot - 1. / 3. * trace_epsdot * I) + omega * S + S * glm::transpose(omega); // Belytschko
																														// (3.7.9)

		particles[i].Sxx_t += S_t[0][0];
		particles[i].Sxy_t += S_t[0][1];
		particles[i].Syy_t += S_t[1][1];
		particles[i].Szz_t += S_t[2][2];
	}
}
