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

#include "body.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <vector>

namespace {
struct wp_thermal_tables {
	std::vector<double> k_T;
	std::vector<double> k_v;
	std::vector<double> cp_T;
	std::vector<double> cp_v;
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

static const wp_thermal_tables &get_wp_thermal_tables() {
	static wp_thermal_tables tbl;
	if (!tbl.parsed) {
		parse_env_table("MFREE_WP_K_TABLE", tbl.k_T, tbl.k_v);
		parse_env_table("MFREE_WP_CP_TABLE", tbl.cp_T, tbl.cp_v);
		tbl.parsed = true;
	}
	return tbl;
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

static double workpiece_alpha_at(double T, double rho0, double k0, double cp0) {
	const wp_thermal_tables &tbl = get_wp_thermal_tables();
	double k = table_eval(T, tbl.k_T, tbl.k_v, k0);
	double cp = table_eval(T, tbl.cp_T, tbl.cp_v, cp0);
	if (!std::isfinite(k) || k < 0.)
		k = k0;
	if (!std::isfinite(cp) || cp <= 0.)
		cp = cp0;
	if (!std::isfinite(rho0) || rho0 <= 0.)
		return 0.;
	return k / (rho0 * cp);
}
} // namespace

void thermal::heat_conduction_pse(body &b) const {
	std::vector<particle> &particles = b.get_particles();
	unsigned int num_part = b.get_num_part();

	// Explicit stability for the heat equation requires V_j = m_j/rho_j to stay
	// bounded.  Cap V_j at the volume corresponding to 5% of rho0 (20x natural),
	// which keeps the stability ratio well below 1 for any realistic dt.
	// This only affects severely-expanded / failed particles; normal particles
	// (rho close to rho0) are completely unaffected.
	const double rho0 = b.get_sim_data().get_physical_constants().rho0();
	const double rho_pse_floor = 0.05 * rho0;

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(num_part); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		if (particles[i].rho < 0.5 * rho0)
			continue;
		const double Ti = particles[i].T;
		const double xi = particles[i].x;
		const double yi = particles[i].y;
		const double hi = particles[i].h;

		double T_lapl = 0.;

		for (unsigned int j = 0; j < particles[i].num_nbh; j++) {
			unsigned int jdx = particles[i].nbh[j];

			const double Tj = particles[jdx].T;
			const double xj = particles[jdx].x;
			const double yj = particles[jdx].y;
			const double mj = particles[jdx].m;
			// Use rho_pse_floor so that a low-density particle cannot inflate
			// V_j=m/rho beyond 20×V_natural and break stability.
			const double rhoj = std::max(particles[jdx].rho, rho_pse_floor);
			const double hj = particles[jdx].h;

			const double xij = xi - xj;
			const double yij = yi - yj;

			// Symmetric smoothing length: averages hi and hj so that the PSE
			// kernel is the same when evaluated from either side of a
			// refinement interface (hi != hj).  For same-resolution pairs
			// (hi == hj) h_sym == hi and the formula is unchanged.
			const double h_sym = 0.5 * (hi + hj);
			const double h_sym2 = h_sym * h_sym;

			const double r = sqrt(xij * xij + yij * yij);
			const double w_pse = 4.0 / (h_sym2 * M_PI) * exp(-r * r / h_sym2);
			T_lapl += (Tj - Ti) * w_pse * mj / rhoj / h_sym2;
		}

		const auto pc = b.get_sim_data().get_physical_constants();
		const double alpha_i = workpiece_alpha_at(Ti, pc.rho0(), pc.tc().k(), pc.tc().cp());
		particles[i].T_t += alpha_i * T_lapl;
	}
}

void thermal::heat_conduction_brookshaw(body &b) const {
	std::vector<particle> &particles = b.get_particles();
	unsigned int num_part = b.get_num_part();

	const double rho0 = b.get_sim_data().get_physical_constants().rho0();
	const double rho_pse_floor = 0.05 * rho0;

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(num_part); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		if (particles[i].rho < 0.5 * rho0)
			continue;
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
			const double rhoj = std::max(particles[jdx].rho, rho_pse_floor);

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

		const auto pc = b.get_sim_data().get_physical_constants();
		const double alpha_i = workpiece_alpha_at(Ti, pc.rho0(), pc.tc().k(), pc.tc().cp());
		particles[i].T_t += alpha_i * T_lapl;
	}
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
}

thermal::thermal(physical_constants pc) {
	assert(pc.tc().k() != 0.);
	m_alpha = pc.tc().k() / (pc.rho0() * pc.tc().cp());
}
