#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <string>
#include "../property.h"
#include "../simulation_data.h"
#include "../benchmarks/material_library.h"

static void require(bool cond, const std::string &msg) {
	if (cond)
		return;
	std::cerr << msg << std::endl;
	std::abort();
}

static bool approx_equal(double a, double b, double abs_tol, double rel_tol) {
	const double diff = std::abs(a - b);
	if (diff <= abs_tol)
		return true;
	const double scale = std::max(std::abs(a), std::abs(b));
	return diff <= rel_tol * scale;
}

void test_linear_interpolation() {
	std::cout << "Testing Linear Interpolation..." << std::endl;

	TableProperty prop;
	std::vector<double> temps = {100.0, 200.0, 300.0};
	std::vector<double> values = {10.0, 20.0, 30.0};

	prop.set_table(temps, values);

	// Test exact points
	assert(std::abs(prop.get(100.0) - 10.0) < 1e-6);
	assert(std::abs(prop.get(200.0) - 20.0) < 1e-6);
	assert(std::abs(prop.get(300.0) - 30.0) < 1e-6);

	// Test interpolation
	assert(std::abs(prop.get(150.0) - 15.0) < 1e-6);
	assert(std::abs(prop.get(250.0) - 25.0) < 1e-6);

	// Test extrapolation (clamping)
	assert(std::abs(prop.get(50.0) - 10.0) < 1e-6);
	assert(std::abs(prop.get(400.0) - 30.0) < 1e-6);

	std::cout << "Linear Interpolation Passed!" << std::endl;
}

void test_temperature_variation_in_table_mode() {
	std::cout << "Testing Temperature Variation in Table Mode..." << std::endl;

	{
		TableProperty prop;
		std::vector<double> temps = {250.0, 500.0, 750.0, 1000.0};
		std::vector<double> values = {100.0, 80.0, 60.0, 40.0};
		prop.set_table(temps, values);

		const double v0 = prop.get(250.0);
		const double v1 = prop.get(375.0);
		const double v2 = prop.get(625.0);
		const double v3 = prop.get(875.0);

		require(v0 != v1, "Table interpolation returned constant value between 250K and 375K");
		require(v1 != v2, "Table interpolation returned constant value between 375K and 625K");
		require(v2 != v3, "Table interpolation returned constant value between 625K and 875K");
		require(v0 > v1 && v1 > v2 && v2 > v3,
				"Table interpolation did not decrease with increasing temperature for decreasing table values");
	}

	{
		TableProperty prop;
		std::vector<double> temps = {300.0, 600.0, 900.0};
		std::vector<double> values = {10.0, 25.0, 55.0};
		prop.set_table(temps, values);

		const double v0 = prop.get(300.0);
		const double v1 = prop.get(450.0);
		const double v2 = prop.get(750.0);
		const double v3 = prop.get(900.0);

		require(v0 != v1, "Table interpolation returned constant value between 300K and 450K");
		require(v1 != v2, "Table interpolation returned constant value between 450K and 750K");
		require(v2 != v3, "Table interpolation returned constant value between 750K and 900K");
		require(v0 < v1 && v1 < v2 && v2 < v3,
				"Table interpolation did not increase with increasing temperature for increasing table values");
	}

	std::cout << "Temperature Variation in Table Mode Passed!" << std::endl;
}

void test_physical_constants_temperature_dependency() {
	std::cout << "Testing Physical Constants Temperature Dependency..." << std::endl;

	physical_constants pc(0.3, 200e9, 7800.0);

	std::vector<double> temps = {293.0, 1000.0};
	std::vector<double> E_values = {200e9, 100e9}; // Stiffness drops with temp

	pc.set_E_table(temps, E_values);

	// Check at T=293 (Default)
	assert(std::abs(pc.E(293.0) - 200e9) < 1e-6);

	// Check at T=1000
	assert(std::abs(pc.E(1000.0) - 100e9) < 1e-6);

	// Check derived constant G at T=293
	// G = E / (2(1+nu)) = 200e9 / (2*1.3) = 200e9 / 2.6 = 76.923e9
	double G_293 = pc.G(293.0);
	double expected_G_293 = 200e9 / (2.0 * (1.0 + 0.3));
	assert(std::abs(G_293 - expected_G_293) < 1e-1);

	// Check derived constant G at T=1000
	// G = 100e9 / 2.6 = 38.461e9
	double G_1000 = pc.G(1000.0);
	double expected_G_1000 = 100e9 / (2.0 * (1.0 + 0.3));
	assert(std::abs(G_1000 - expected_G_1000) < 1e-1);

	std::cout << "Physical Constants Dependency Passed!" << std::endl;
}

void test_linear_fit_and_clamping() {
	std::cout << "Testing Linear Fit + Clamping..." << std::endl;

	TableProperty prop;

	prop.set_linear(2.0, 1.0);
	assert(std::abs(prop.get(0.0) - 1.0) < 1e-6);
	assert(std::abs(prop.get(10.0) - 21.0) < 1e-6);

	prop.set_linear(1.0, 0.0, true, 0.0, true, 10.0);
	assert(std::abs(prop.get(-5.0) - 0.0) < 1e-6);
	assert(std::abs(prop.get(5.0) - 5.0) < 1e-6);
	assert(std::abs(prop.get(20.0) - 10.0) < 1e-6);

	std::cout << "Linear Fit + Clamping Passed!" << std::endl;
}

void test_linear_equations_configured() {
	std::cout << "Testing Linear Equation Configuration..." << std::endl;

	physical_constants pc(0.35, 113.8e9, 4430.0);

	pc.set_rho0_linear(-0.1401, 4464.74, true, 1.0, false, 0.0);
	pc.set_alpha_linear(1.08e-05, -3.41e-03, false, 0.0, false, 0.0);
	pc.set_k_linear(0.0178, 0.394, true, 1e-12, false, 0.0);
	pc.set_E_linear(-5.36e+07, 1.32e+11, true, 1e6, false, 0.0);
	pc.set_nu_linear(4.15e-05, 0.3053, true, 0.0, true, 0.499);
	pc.set_cp_linear(0.2273, 491.62, true, 1.0, false, 0.0);

	const double T1 = 300.0;
	const double T2 = 1000.0;

	assert(std::abs(pc.rho0(T1) - (-0.1401 * T1 + 4464.74)) < 1e-6);
	assert(std::abs(pc.alpha(T1) - (1.08e-05 * T1 - 3.41e-03)) < 1e-12);
	assert(std::abs(pc.tc().k(T1) - (0.0178 * T1 + 0.394)) < 1e-6);
	assert(std::abs(pc.E(T1) - (-5.36e+07 * T1 + 1.32e+11)) < 1e-3);
	assert(std::abs(pc.nu(T1) - (4.15e-05 * T1 + 0.3053)) < 1e-12);
	assert(std::abs(pc.tc().cp(T1) - (0.2273 * T1 + 491.62)) < 1e-6);

	assert(std::abs(pc.rho0(T2) - (-0.1401 * T2 + 4464.74)) < 1e-6);
	assert(std::abs(pc.alpha(T2) - (1.08e-05 * T2 - 3.41e-03)) < 1e-12);
	assert(std::abs(pc.tc().k(T2) - (0.0178 * T2 + 0.394)) < 1e-6);
	assert(std::abs(pc.E(T2) - (-5.36e+07 * T2 + 1.32e+11)) < 1e-3);
	assert(std::abs(pc.nu(T2) - (4.15e-05 * T2 + 0.3053)) < 1e-12);
	assert(std::abs(pc.tc().cp(T2) - (0.2273 * T2 + 491.62)) < 1e-6);

	std::cout << "Linear Equation Configuration Passed!" << std::endl;
}

void test_preset_properties_spot_check() {
	std::cout << "Testing Preset Property Wiring (Spot-Check)..." << std::endl;

	struct PresetEntry {
		const char *name;
		physical_constants (*fn)();
		bool expect_temp_dependent;
	};

	std::vector<PresetEntry> presets = {
		{"matlib_steel4430", &matlib_steel4430, false},
		{"matlib_ARMCO_iron", &matlib_ARMCO_iron, false},
		{"matlib_OFHC_copper", &matlib_OFHC_copper, false},
		{"matlib_AISI1045", &matlib_AISI1045, false},
		{"matlib_rubber", &matlib_rubber, false},
		{"matlib_rubber_real", &matlib_rubber_real, false},
		{"matlib_thermal_synthetic", &matlib_thermal_synthetic, false},
		{"matlib_tial6v4_lesuer", &matlib_tial6v4_lesuer, false},
		{"matlib_tial6v4_johnson_SI", &matlib_tial6v4_johnson_SI, false},
		{"matlib_tial6v4_johnson_cm_musec_g", &matlib_tial6v4_johnson_cm_musec_g, false},
		{"matlib_tial6v4_Sima_tanh2010_SI", &matlib_tial6v4_Sima_tanh2010_SI, true},
		{"matlib_tial6v4_Sima_tanh2010_cm_musec_g", &matlib_tial6v4_Sima_tanh2010_cm_musec_g, false},
		{"matlib_dummy", &matlib_dummy, false},
		{"matlib_a2024t351", &matlib_a2024t351, false},
	};

	auto require_finite = [&](double v, const std::string &label) { require(std::isfinite(v), "Non-finite value detected for " + label); };

	auto check_var_if_expected = [&](double v1, double v2, const std::string &label, bool expect) {
		require_finite(v1, label + " at T1");
		require_finite(v2, label + " at T2");
		if (!expect)
			return;
		const bool same = approx_equal(v1, v2, 0.0, 0.0);
		require(!same, "Preset property expected to vary with temperature but appears constant: " + label);
	};

	for (const auto &preset : presets) {
		physical_constants pc = preset.fn();
		const auto jc = pc.jc();

		const double Tref = jc.valid() ? jc.Tref() : 293.0;
		const double T1 = Tref;
		const double T2 = Tref + 200.0;

		const std::string prefix = std::string(preset.name) + " ";

		const double rho1 = pc.rho0(T1);
		const double rho2 = pc.rho0(T2);
		check_var_if_expected(rho1, rho2, prefix + "rho0(T)", preset.expect_temp_dependent);
		require(rho1 > 0.0 && rho2 > 0.0, prefix + "rho0(T) must be positive");

		const double a1 = pc.alpha(T1);
		const double a2 = pc.alpha(T2);
		check_var_if_expected(a1, a2, prefix + "alpha(T)", preset.expect_temp_dependent);

		const double k1 = pc.tc().k(T1);
		const double k2 = pc.tc().k(T2);
		check_var_if_expected(k1, k2, prefix + "k(T)", preset.expect_temp_dependent);
		if (preset.expect_temp_dependent) {
			require(k1 >= 0.0 && k2 >= 0.0, prefix + "k(T) must be non-negative");
		}

		const double cp1 = pc.tc().cp(T1);
		const double cp2 = pc.tc().cp(T2);
		check_var_if_expected(cp1, cp2, prefix + "cp(T)", preset.expect_temp_dependent);
		if (preset.expect_temp_dependent) {
			require(cp1 > 0.0 && cp2 > 0.0, prefix + "cp(T) must be positive");
		}

		const double E1 = pc.E(T1);
		const double E2 = pc.E(T2);
		check_var_if_expected(E1, E2, prefix + "E(T)", preset.expect_temp_dependent);
		if (preset.expect_temp_dependent) {
			require(E1 > 0.0 && E2 > 0.0, prefix + "E(T) must be positive");
		}

		const double nu1 = pc.nu(T1);
		const double nu2 = pc.nu(T2);
		check_var_if_expected(nu1, nu2, prefix + "nu(T)", preset.expect_temp_dependent);
		require(nu1 >= 0.0 && nu1 < 0.5 && nu2 >= 0.0 && nu2 < 0.5, prefix + "nu(T) must be in [0, 0.5)");
	}

	std::cout << "Preset Property Wiring (Spot-Check) Passed!" << std::endl;
}

int main() {
	test_linear_interpolation();
	test_temperature_variation_in_table_mode();
	test_linear_fit_and_clamping();
	test_physical_constants_temperature_dependency();
	test_linear_equations_configured();
	test_preset_properties_spot_check();
	return 0;
}
