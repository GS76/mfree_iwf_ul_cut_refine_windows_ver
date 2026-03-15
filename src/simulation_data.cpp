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

#include "simulation_data.h"

//---------------------------------------------------------------------------
// Getter

double johnson_cook_constants::A() const {
	return m_A;
}

double johnson_cook_constants::B() const {
	return m_B;
}

double johnson_cook_constants::C() const {
	return m_C;
}

double johnson_cook_constants::m() const {
	return m_m;
}

double johnson_cook_constants::n() const {
	return m_n;
}

double johnson_cook_constants::a() const {	// tanh- Erweiterung, Calamaz 2008
	return m_a;
}

double johnson_cook_constants::b() const {	// tanh- Erweiterung, Calamaz 2008
	return m_b;
}

double johnson_cook_constants::c() const {	// tanh- Erweiterung, Calamaz 2008
	return m_c;
}

double johnson_cook_constants::d() const {	// tanh- Erweiterung, Calamaz 2008
	return m_d;
}

double johnson_cook_constants::s() const {			// JC-tanh- Erweiterung, Sima 2010
	return m_s;
}

double johnson_cook_constants::Tmelt() const {
	return m_Tmelt;
}

double johnson_cook_constants::Tref() const {
	return m_Tref;
}

double johnson_cook_constants::eps_ref() const {
	return m_eps_ref;
}

double johnson_cook_constants::eps_pl_0() const {
    // Reference plastic strain rate (s⁻¹)
    // This is used in Johnson-Cook strain rate sensitivity term: (1 + C * ln(eps_dot_pl/eps_dot_0))
    return m_eps_ref;
}

bool johnson_cook_constants::valid() const {
	bool all_zero = m_A == 0. && m_B == 0. && m_C == 0. && m_m == 0. && m_n == 0. && m_Tmelt == 0. && m_Tref == 0. && m_eps_ref == 0.;
	return !all_zero;
}

johnson_cook_constants::johnson_cook_constants(double A, double B, double C, double m, double n, double Tmelt, double Tref, double eps_ref)
: m_A(A), m_B(B), m_C(C), m_m(m), m_n(n), m_Tmelt(Tmelt), m_Tref(Tref), m_eps_ref(eps_ref) {}

johnson_cook_constants::johnson_cook_constants(double A, double B, double C, double m, double n,  double a, double b, double c, double d, double s, double Tmelt, double Tref, double eps_ref)
: m_A(A), m_B(B), m_C(C), m_m(m), m_n(n),  m_a(a), m_b(b), m_c(c), m_d(d), m_s(s), m_Tmelt(Tmelt), m_Tref(Tref), m_eps_ref(eps_ref) {}	// JC-tanh Modell, nach Calamaz 2008

johnson_cook_constants::johnson_cook_constants() {}

//---------------------------------------------------------------------------
// Setter

thermal_constants::thermal_constants(double cp, double Taylor_Quinney, double k) : m_cp(cp), m_Taylor_Quinney(Taylor_Quinney), m_k(k) {}

thermal_constants::thermal_constants() {}

double thermal_constants::cp(double T) const {
	return m_cp.get(T);
}

double thermal_constants::Taylor_Quinney() const {
	return m_Taylor_Quinney;
}

double thermal_constants::k(double T) const {
	return m_k.get(T);
}

//---------------------------------------------------------------------------

physical_constants::physical_constants(double nu, double E, double rho0) : m_nu(nu), m_E(E), m_rho0(rho0), m_alpha(0.0) {}

physical_constants::physical_constants(double nu, double E, double rho0, johnson_cook_constants jc)
: m_nu(nu), m_E(E), m_rho0(rho0), m_alpha(0.0), m_jc(jc) {}

physical_constants::physical_constants(double nu, double E, double rho0, johnson_cook_constants jc, thermal_constants tc)
: m_nu(nu), m_E(E), m_rho0(rho0), m_alpha(0.0), m_jc(jc), m_tc(tc) {}

physical_constants::physical_constants() {}

double physical_constants::nu(double T) const {
	return m_nu.get(T);
}

double physical_constants::E(double T) const {
	return m_E.get(T);
}

double physical_constants::G(double T) const {
	return E(T)/(2.*(1.+nu(T)));
}

double physical_constants::K(double T)  const {
	double G_val = this->G(T);
	return 2.0*G_val*(1+nu(T))/(3*(1-2*nu(T)));
}

double physical_constants::mu_lame(double T) const {
	return E(T)/(2.0+2.0*nu(T));
}

double physical_constants::lambda_lame(double T) const {
	return  nu(T) * E(T) / ((1.0+nu(T))*(1.0-2.0*nu(T)));
}

double physical_constants::rho0(double T) const {
	return m_rho0.get(T);
}

double physical_constants::alpha(double T) const {
	return m_alpha.get(T);
}

double physical_constants::c0(double T) const {
	return sqrt(K(T)/rho0(T));
}

johnson_cook_constants physical_constants::jc() const {
	return m_jc;
}

thermal_constants physical_constants::tc() const {
	return m_tc;
}

//---------------------------------------------------------------------------

double constants_monaghan::mghn_wdeltap() const {
	return m_mghn_wdeltap;
}

unsigned int constants_monaghan::mghn_corr_exp() const {
	return m_mghn_corr_exp;
}

double constants_monaghan::mghn_eps() const {
	return m_mghn_eps;
}

constants_monaghan::constants_monaghan(double wdeltap, unsigned int corr_exp, double eps) :
				m_mghn_wdeltap(wdeltap),
				m_mghn_corr_exp(corr_exp),
				m_mghn_eps(eps) {}

constants_monaghan::constants_monaghan() {}

//---------------------------------------------------------------------------

double constants_artificial_viscosity::artvisc_alpha() const {
	return m_artvisc_alpha;
}

double constants_artificial_viscosity::artvisc_beta() const {
	return m_artvisc_beta;
}

double constants_artificial_viscosity::artvisc_eta() const {
	return m_artvisc_eta;
}

constants_artificial_viscosity::constants_artificial_viscosity(double alpha, double beta, double eta) :
			m_artvisc_alpha(alpha),
			m_artvisc_beta(beta),
			m_artvisc_eta(eta) {}

constants_artificial_viscosity::constants_artificial_viscosity() {}

//---------------------------------------------------------------------------

correction_constants::correction_constants(constants_monaghan monaghan_constants, constants_artificial_viscosity constants_art_visc, double xsph_eps) :
				m_xsph_eps(xsph_eps),
				m_constants_monaghan(monaghan_constants),
				m_constants_art_visc(constants_art_visc)
{}

correction_constants::correction_constants() {}


double correction_constants::xsph_eps() const {
	return m_xsph_eps;
}

constants_monaghan correction_constants::get_monaghan_const() const {
	return m_constants_monaghan;
}

constants_artificial_viscosity correction_constants::get_art_visc_const() const {
	return m_constants_art_visc;
}

//---------------------------------------------------------------------------

simulation_data::simulation_data(physical_constants physical_constants, correction_constants correction_constants)
: m_physical_constants(physical_constants), m_correction_constants(correction_constants) {};

simulation_data::simulation_data() {}

physical_constants simulation_data::get_physical_constants() const {
	return m_physical_constants;
}

correction_constants simulation_data::get_correction_constants() const {
	return m_correction_constants;
}
