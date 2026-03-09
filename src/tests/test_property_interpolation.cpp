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

int main() {
    test_linear_interpolation();
    test_physical_constants_temperature_dependency();
    return 0;
}
