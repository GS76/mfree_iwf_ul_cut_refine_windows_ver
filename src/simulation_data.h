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

#ifndef SIMULATION_DATA_H_
#define SIMULATION_DATA_H_

#include <math.h>
#include "property.h"

/*
 * for a given body:
 * ------------------
 * this file contains all elastic, plastic, thermal and correction constants
 */

class johnson_cook_constants {
// ... existing private members ...
private:
	double m_A = 0.;
	double m_B = 0.;
	double m_C = 0.;
	double m_m = 0.;
	double m_n = 0.;

	double m_a = 0.;
	double m_b = 0.;
	double m_c = 0.;
	double m_d = 0.;
	double m_s = 0.;

	double m_Tmelt = 0.;
	double m_Tref  = 0.;
	double m_eps_ref = 0.;

public:
	johnson_cook_constants(double A, double B, double C, double m, double n, double Tmelt, double Tref, double eps_ref = 1.);
	johnson_cook_constants(double A, double B, double C, double m, double n, double a, double b, double c, double d, double s, double Tmelt, double Tref, double eps_ref = 1.);	// JC-tanh- Erweiterung, Calamaz 2008
	johnson_cook_constants();

	double A() const;
	double B() const;
	double C() const;
	double m() const;
	double n() const;

	double a() const;
	double b() const;
	double c() const;
	double d() const;
	double s() const;

	double eps_pl_0() const;

	double Tmelt() const;
	double Tref() const;
	double eps_ref() const;

	bool valid() const;
};

class thermal_constants {
	TableProperty m_cp;
	double m_Taylor_Quinney = 0.;
	TableProperty m_k;

public:
	thermal_constants(double cp, double Taylor_Quinney, double k = 0.);
	thermal_constants();

	double cp(double T = 293.0) const;				/*!< Heat capacity */
	double Taylor_Quinney() const;	/*!< Percentage of plastic work converted into thermal energy */
	double k(double T = 293.0) const;				/*!< Thermal conduction coefficient */
	
	void set_cp_table(const std::vector<double>& temps, const std::vector<double>& values) { m_cp.set_table(temps, values); }
	void set_k_table(const std::vector<double>& temps, const std::vector<double>& values) { m_k.set_table(temps, values); }
	void set_cp_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_cp.set_linear(m, b, use_min, min_val, use_max, max_val); }
	void set_k_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_k.set_linear(m, b, use_min, min_val, use_max, max_val); }
};

class physical_constants {
private:
	TableProperty m_nu;
	TableProperty m_E;
	TableProperty m_rho0;
	TableProperty m_alpha;

	johnson_cook_constants m_jc;
	thermal_constants m_tc;
public:
	physical_constants(double nu, double E, double rho0);
	physical_constants(double nu, double E, double rho0, johnson_cook_constants jc);
	physical_constants(double nu, double E, double rho0, johnson_cook_constants jc, thermal_constants tc);
	physical_constants();
	
	double nu(double T = 293.0) const;
	double E(double T = 293.0) const;
	double G(double T = 293.0) const;
	double K(double T = 293.0) const;
	double rho0(double T = 293.0) const;
	double alpha(double T = 293.0) const;
	double c0(double T = 293.0) const;
	double mu_lame(double T = 293.0) const;
	double lambda_lame(double T = 293.0) const;
	
	johnson_cook_constants jc() const;
	thermal_constants tc() const;

	void set_E_table(const std::vector<double>& temps, const std::vector<double>& values) { m_E.set_table(temps, values); }
	void set_nu_table(const std::vector<double>& temps, const std::vector<double>& values) { m_nu.set_table(temps, values); }
	void set_rho0_table(const std::vector<double>& temps, const std::vector<double>& values) { m_rho0.set_table(temps, values); }
	void set_alpha_table(const std::vector<double>& temps, const std::vector<double>& values) { m_alpha.set_table(temps, values); }
	void set_cp_table(const std::vector<double>& temps, const std::vector<double>& values) { m_tc.set_cp_table(temps, values); }
	void set_k_table(const std::vector<double>& temps, const std::vector<double>& values) { m_tc.set_k_table(temps, values); }

	void set_E_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_E.set_linear(m, b, use_min, min_val, use_max, max_val); }
	void set_nu_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_nu.set_linear(m, b, use_min, min_val, use_max, max_val); }
	void set_rho0_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_rho0.set_linear(m, b, use_min, min_val, use_max, max_val); }
	void set_alpha_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_alpha.set_linear(m, b, use_min, min_val, use_max, max_val); }
	void set_cp_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_tc.set_cp_linear(m, b, use_min, min_val, use_max, max_val); }
	void set_k_linear(double m, double b, bool use_min, double min_val, bool use_max, double max_val) { m_tc.set_k_linear(m, b, use_min, min_val, use_max, max_val); }
};

class constants_monaghan {
	double m_mghn_wdeltap = 0.;
	unsigned int m_mghn_corr_exp = 0;
	double m_mghn_eps = 0.;

public:
	double mghn_wdeltap() const;
	unsigned int mghn_corr_exp() const;
	double mghn_eps() const;

	constants_monaghan(double wdeltap, unsigned int corr_exp, double eps);
	constants_monaghan();
};

class constants_artificial_viscosity {
	double m_artvisc_alpha = 0.;
	double m_artvisc_beta = 0.;
	double m_artvisc_eta = 0.;

public:
	double artvisc_alpha() const;
	double artvisc_beta() const;
	double artvisc_eta() const;

	constants_artificial_viscosity(double alpha, double beta, double eta);
	constants_artificial_viscosity();
};

class correction_constants {

private:
	double m_xsph_eps = 0.;
	constants_monaghan m_constants_monaghan;
	constants_artificial_viscosity m_constants_art_visc;

public:
	correction_constants(constants_monaghan monaghan_constants, constants_artificial_viscosity constants_art_visc, double xsph_eps);
	correction_constants();
	double xsph_eps() const;
	constants_monaghan get_monaghan_const() const;
	constants_artificial_viscosity get_art_visc_const() const;
};

class simulation_data {
private:
	physical_constants   m_physical_constants;
	correction_constants m_correction_constants;

public:
	simulation_data();
	simulation_data(physical_constants physical_constants, correction_constants correction_constants);
	physical_constants   get_physical_constants() const;
	correction_constants get_correction_constants() const;
};

#endif /* SIMULATION_DATA_H_ */
