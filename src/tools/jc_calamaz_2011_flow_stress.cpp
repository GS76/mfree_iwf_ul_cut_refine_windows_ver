// jc_calamaz_2011_flow_stress.cpp
//
// Standalone tool: generates a CSV of flow stress vs. true strain from the
// modified Johnson-Cook model by Calamaz et al. (2011).
//
// Reference:
//   M. Calamaz, D. Coupard, M. Nouari, F. Girot,
//   "Numerical analysis of chip formation and shear localisation processes
//    in machining the Ti-6Al-4V titanium alloy",
//   Int J Adv Manuf Technol (2011) 52:887-895, Eq. (2).
//
// Model (Eq. 2):
//   sigma = [A + B * eps^(n - a - 0.12*(eps*eps_dot)^a)]
//         * [1 + C * ln(eps_dot / eps_dot_ref)]
//         * [1 - ((T - Tr) / (Tm - Tr))^m]
//
// When a = 0 this reduces to standard Johnson-Cook.
//
// Usage:
//   mfree_jc_calamaz_2011_flow_stress [options]
//
// Options:
//   --strain-rate <value>   Plastic strain rate in 1/s        (default: 1.0)
//   --temperature <value>   Temperature in K                  (default: 298.0)
//   --softening-a <value>   Calamaz softening parameter 'a'   (default: 0.22)
//   --output <filename>     Output CSV path                   (default: jc_calamaz_2011_flow_stress.csv)
//   --num-points <N>        Number of strain samples [0,15]   (default: 1500)

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

// ---------------------------------------------------------------------------
// Calamaz 2011 MJC parameters (Ti-6Al-4V, Li & He, SI units)
// ---------------------------------------------------------------------------
struct calamaz_2011_params {
	double A         = 968.0e6;   // [Pa]
	double B         = 380.0e6;   // [Pa]
	double n         = 0.421;
	double C         = 0.0197;
	double m         = 0.577;
	double Tr        = 298.0;     // [K] reference temperature
	double Tm        = 1878.0;    // [K] melting temperature
	double eps_dot_ref = 1.0;     // [1/s] reference strain rate
	double a         = 0.22;      // softening parameter (0 = standard JC)
};

// ---------------------------------------------------------------------------
// Flow stress computation (Calamaz 2011, Eq. 2)
// ---------------------------------------------------------------------------
static double sigma_calamaz_2011(double eps, double eps_dot, double T,
                                 const calamaz_2011_params &p) {
	// --- Strain hardening + softening term ---
	double Term_A;
	if (eps <= 0.0) {
		// At eps = 0 the B-term vanishes (limit of eps^(n-a) -> 0 for n > a)
		Term_A = p.A;
	} else {
		// Effective exponent: n - a - 0.12 * (eps * eps_dot)^a
		double eff_exp = p.n - p.a - 0.12 * std::pow(eps * eps_dot, p.a);
		Term_A = p.A + p.B * std::pow(eps, eff_exp);
	}

	// --- Strain rate term ---
	double eps_dot_ratio = eps_dot / p.eps_dot_ref;
	double Term_B = 1.0;
	if (eps_dot_ratio > 1.0) {
		Term_B = 1.0 + p.C * std::log(eps_dot_ratio);
	} else if (eps_dot_ratio > 0.0) {
		// For sub-reference rates, use power-law form (same convention as Sima tool)
		Term_B = std::pow(1.0 + eps_dot_ratio, p.C);
	}

	// --- Thermal softening term ---
	double T_clamped = (T < p.Tr) ? p.Tr : T;
	double theta = (T_clamped - p.Tr) / (p.Tm - p.Tr);
	double Term_C = 1.0 - std::pow(theta, p.m);

	return Term_A * Term_B * Term_C;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void print_usage(const char *prog) {
	std::cout << "Usage: " << prog << " [options]\n"
			  << "\nGenerates flow stress vs. true strain from the Calamaz et al. 2011\n"
			  << "modified Johnson-Cook model (Ti-6Al-4V, SI units).\n"
			  << "\nOptions:\n"
			  << "  --strain-rate <value>   Plastic strain rate in 1/s        (default: 1.0)\n"
			  << "  --temperature <value>   Temperature in K                  (default: 298.0)\n"
			  << "  --softening-a <value>   Calamaz softening parameter 'a'   (default: 0.22)\n"
			  << "  --output <filename>     Output CSV path                   (default: jc_calamaz_2011_flow_stress.csv)\n"
			  << "  --num-points <N>        Number of strain samples [0,15]   (default: 1500)\n"
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
	double softening_a = 0.22;
	std::string output_path = "jc_calamaz_2011_flow_stress.csv";
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
		} else if (std::strcmp(argv[i], "--softening-a") == 0 && i + 1 < argc) {
			if (!try_parse_double(argv[++i], softening_a) || softening_a < 0.0) {
				std::cerr << "Error: invalid --softening-a value\n";
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

	// Build parameters
	calamaz_2011_params params;
	params.a = softening_a;

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
		double sigma = sigma_calamaz_2011(eps, strain_rate, temperature, params);

		csv << eps << "," << sigma << "\n";

		if (sigma < stress_min) stress_min = sigma;
		if (sigma > stress_max) stress_max = sigma;
	}

	csv.close();

	// Console summary
	std::cout << "JC-Calamaz 2011 Flow Stress Generator\n"
			  << "--------------------------------------\n"
			  << "Material       : Ti-6Al-4V (Li & He / Calamaz 2011, SI)\n"
			  << "Strain rate    : " << strain_rate << " 1/s\n"
			  << "Temperature    : " << temperature << " K\n"
			  << "Softening a    : " << softening_a << "\n"
			  << "Strain range   : [" << eps_min << ", " << eps_max << "]\n"
			  << "Num points     : " << num_points << "\n"
			  << "Min flow stress: " << stress_min << " Pa\n"
			  << "Max flow stress: " << stress_max << " Pa\n"
			  << "Output         : " << output_path << "\n";

	return 0;
}
