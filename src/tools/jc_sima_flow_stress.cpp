// jc_sima_flow_stress.cpp
//
// Standalone tool: generates a CSV of flow stress vs. true strain from the
// Johnson-Cook Sima 2010 material model for a given strain rate and temperature.
//
// Usage:
//   mfree_jc_sima_flow_stress [options]
//
// Options:
//   --strain-rate <value>   Plastic strain rate in 1/s      (default: 1.0)
//   --temperature <value>   Temperature in K                (default: 298.0)
//   --output <filename>     Output CSV path                 (default: jc_sima_flow_stress.csv)
//   --num-points <N>        Number of strain samples [0,15] (default: 1500)

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "../simulation_data.h"
#include "../simulation_time.h"
#include "../johnson_cook_Sima_2010.h"
#include "../benchmarks/material_library.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void print_usage(const char *prog) {
	std::cout << "Usage: " << prog << " [options]\n"
			  << "\nGenerates flow stress vs. true strain from the JC-Sima 2010 model\n"
			  << "(Ti-6Al-4V, SI units) for a given strain rate and temperature.\n"
			  << "\nOptions:\n"
			  << "  --strain-rate <value>   Plastic strain rate in 1/s      (default: 1.0)\n"
			  << "  --temperature <value>   Temperature in K                (default: 298.0)\n"
			  << "  --output <filename>     Output CSV path                 (default: jc_sima_flow_stress.csv)\n"
			  << "  --num-points <N>        Number of strain samples [0,15] (default: 1500)\n"
			  << "  --help                  Show this message\n";
}

static bool try_parse_double(const char *s, double &out) {
	char *end = nullptr;
	out = std::strtod(s, &end);
	return end != s && std::isfinite(out);
}

static bool try_parse_int(const char *s, int &out) {
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s || v <= 0)
		return false;
	out = static_cast<int>(v);
	return true;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char **argv) {
	// Defaults
	double strain_rate = 1.0;
	double temperature = 298.0;
	std::string output_path = "jc_sima_flow_stress.csv";
	int num_points = 1500;
	const double eps_min = 0.0;
	const double eps_max = 15.0;

	// Parse command-line arguments
	for (int i = 1; i < argc; i++) {
		if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
			print_usage(argv[0]);
			return 0;
		}
		if (std::strcmp(argv[i], "--strain-rate") == 0 && i + 1 < argc) {
			if (!try_parse_double(argv[++i], strain_rate) || strain_rate < 0.0) {
				std::cerr << "Error: invalid --strain-rate value\n";
				return 1;
			}
		} else if (std::strcmp(argv[i], "--temperature") == 0 && i + 1 < argc) {
			if (!try_parse_double(argv[++i], temperature) || temperature <= 0.0) {
				std::cerr << "Error: invalid --temperature value\n";
				return 1;
			}
		} else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
			output_path = argv[++i];
		} else if (std::strcmp(argv[i], "--num-points") == 0 && i + 1 < argc) {
			if (!try_parse_int(argv[++i], num_points)) {
				std::cerr << "Error: invalid --num-points value\n";
				return 1;
			}
		} else {
			std::cerr << "Unknown option: " << argv[i] << "\n";
			print_usage(argv[0]);
			return 1;
		}
	}

	// Initialise the simulation_time singleton with a dummy dt.
	// sigma_yield() does not use it, but the singleton must exist for linkage.
	simulation_time &sim_time = simulation_time::getInstance();
	sim_time.set_dt(1.0e-8);

	// Build the material model from the library (Ti-6Al-4V, Sima 2010, SI)
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	johnson_cook_Sima_2010 jc(pc);

	// Open output CSV
	std::ofstream csv(output_path);
	if (!csv.is_open()) {
		std::cerr << "Error: cannot open output file: " << output_path << "\n";
		return 1;
	}

	csv << std::setprecision(12);
	csv << "true_strain,flow_stress_Pa\n";

	// Sweep true strain from eps_min to eps_max
	double stress_min = std::numeric_limits<double>::max();
	double stress_max = std::numeric_limits<double>::lowest();

	const double d_eps = (num_points > 1) ? (eps_max - eps_min) / (num_points - 1) : 0.0;

	for (int i = 0; i < num_points; i++) {
		double eps = eps_min + i * d_eps;
		double sigma = jc.sigma_yield(eps, strain_rate, temperature);

		csv << eps << "," << sigma << "\n";

		if (sigma < stress_min) stress_min = sigma;
		if (sigma > stress_max) stress_max = sigma;
	}

	csv.close();

	// Console summary
	std::cout << "JC-Sima 2010 Flow Stress Generator\n"
			  << "-----------------------------------\n"
			  << "Material       : Ti-6Al-4V (Sima/Oezel 2010, SI)\n"
			  << "Strain rate    : " << strain_rate << " 1/s\n"
			  << "Temperature    : " << temperature << " K\n"
			  << "Strain range   : [" << eps_min << ", " << eps_max << "]\n"
			  << "Num points     : " << num_points << "\n"
			  << "Min flow stress: " << stress_min << " Pa\n"
			  << "Max flow stress: " << stress_max << " Pa\n"
			  << "Output         : " << output_path << "\n";

	return 0;
}
