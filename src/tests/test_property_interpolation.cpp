#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include "../property.h"
#include "../simulation_data.h"

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

int main() {
    test_linear_interpolation();
    test_linear_fit_and_clamping();
    test_physical_constants_temperature_dependency();
    test_linear_equations_configured();
    return 0;
}
