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

#include <iostream>
#include <float.h>
#include <stdlib.h>
#include <fenv.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <omp.h>
#include <signal.h>
#include <stdexcept>
#include <exception>

#include "particle.h"
#include "contact.h"
#include "vtk_writer.h"
#include "material.h"

#include "benchmarks/test_density.h"
#include "benchmarks/test_benches.h"
#include "benchmarks/test_cuttings.h"

#include "tool.h"
#include "logger.h"
#include "body.h"
#include "config/build_from_config.h"
#include "config/simulation_config.h"

logger *global_logger;

#include <algorithm>
#include <set>
#include <iterator>

#ifdef __FAST_MATH__
#error "Do NOT compile using -ffast-math"
#endif

namespace fs = std::filesystem;

#ifdef _WIN32
void fpe_signal_handler(int sig) {
	_fpreset(); // Reset floating point state
	throw std::runtime_error("Floating Point Exception Detected (SIGFPE)");
}
#endif

int main(int argc, char *argv[]) {
#ifndef _WIN32
	feenableexcept(FE_INVALID | FE_OVERFLOW);
#elif defined(_WIN32)
	// Windows-compatible floating-point exception control
	_fpreset();
	// Enable (Unmask) Invalid, ZeroDivide, and Overflow exceptions
	// _controlfp(0, ...) clears the mask bits, enabling the exceptions
	_controlfp(0, _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW);

	// Register signal handler for SIGFPE
	signal(SIGFPE, fpe_signal_handler);
#endif

	// inputs
	int model = 1;
	bool smoke = false;
	bool cooldown = false;
	bool cooldown_remove_tool = false;
	double cooldown_hconv_W_m2K = 25.0;
	bool all_steps = false;
	std::string config_path;
	std::string dump_config_path;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-m" && i + 1 < argc) {
			model = std::atoi(argv[++i]);
			printf("running model %d\n", model);
		} else if (arg == "--config" && i + 1 < argc) {
			config_path = argv[++i];
		} else if (arg == "--dump-config" && i + 1 < argc) {
			dump_config_path = argv[++i];
		} else if (arg == "--smoke") {
			smoke = true;
		} else if (arg == "--cooldown") {
			cooldown = true;
		} else if (arg == "--cooldown-remove-tool") {
			cooldown = true;
			cooldown_remove_tool = true;
		} else if (arg == "--cooldown-hconv" && i + 1 < argc) {
			cooldown = true;
			cooldown_hconv_W_m2K = std::atof(argv[++i]);
		} else if (arg == "--all-steps") {
			all_steps = true;
		}
	}

	if (!dump_config_path.empty()) {
		const auto j = mfree::config::dump_default_simulation_config_json();
		std::ofstream out(dump_config_path, std::ios::out | std::ios::binary);
		if (!out) {
			throw std::runtime_error("Failed to open --dump-config path for writing: " + dump_config_path);
		}
		out << mfree::config::dump_json(j, 2);
		return 0;
	}

	bool use_config = !config_path.empty();
	mfree::config::simulation_config cfg;
	if (use_config) {
		cfg = mfree::config::load_simulation_config_file(config_path);
	}

	const std::string output_dir = use_config ? cfg.io.output_dir : "results";
	const bool clear_output_dir = use_config ? cfg.io.clear_output_dir : true;

	const fs::path folder = fs::current_path() / output_dir;
	std::error_code fs_ec;
	if (!fs::exists(folder, fs_ec)) {
		fs::create_directories(folder, fs_ec);
	} else if (!fs::is_directory(folder, fs_ec)) {
		fs::remove(folder, fs_ec);
		fs::create_directories(folder, fs_ec);
	}
	if (!fs_ec && clear_output_dir) {
		for (fs::directory_iterator it(folder, fs_ec), end; !fs_ec && it != end; it.increment(fs_ec)) {
			const fs::directory_entry &entry = *it;
			if (entry.is_regular_file(fs_ec)) {
				std::string ext = entry.path().extension().string();
				if (ext == ".txt" || ext == ".vtk") {
					fs::remove(entry.path(), fs_ec);
				}
			}
		}
	}

	int nx = 31;
	if (!use_config) {
		assert(model >= 1);
		assert(model <= 4);
	}

	/*
	 ==========================
	 *  set up chosen benchmark
	 *  	this runs model 1-4 in the paper
	 *  	other preliminary simulations are available in test_benches.h
	 *  	density reapproximation tests are aviable in test_density.h
	 ==========================
	 */
	body *b = 0;
	if (use_config) {
		b = mfree::config::build_body_from_config(cfg);
	} else {
		switch (model) {
		case 1:
			b = cutting_ref_single_resol(nx);
			break;
		case 2:
			nx = 61;
			b = cutting_ref_multi_resol_apriori(nx);
			break;
		case 3:
			nx = 61;
			b = cutting_ref_multi_resol_dynamic(nx);
			break;
		case 4:
			nx = 61;
			b = cutting_ref_single_resol(nx);
			break;
		}
	}

	/*
	  ==================================
	 settings of the printout
	 at least [num_print] frames are written out
	 ===================================
	 */
	simulation_time *time = &simulation_time::getInstance();
	int num_print = use_config ? cfg.io.num_print : 150;
	if (smoke) {
		if (!cooldown) {
			global_logger = 0;
		}
		time->set_t_final(time->get_dt() * 1000.0);
		num_print = cooldown ? 5 : 1;
	}

	const double cut_end_time = time->get_t_final();
	const double cut_dt_s = time->get_dt();
	const double ambient_T_K = 273.15 + 22.0;
	const double ambient_T_stop_K = 273.15 + 22.5;
	const double max_rate_K_per_s = 5.0 / 60.0;
	const double cooldown_max_time_s = smoke ? (time->get_dt() * 200.0) : (6.0 * 3600.0);
	const double cooldown_dt_s = 6.0;
	if (cooldown) {
		time->set_t_final(cut_end_time + cooldown_max_time_s);
	}

	unsigned long long num_step = 0;
	if (cooldown && !smoke) {
		const unsigned long long cut_steps = (cut_dt_s > 0.0) ? (unsigned long long)std::ceil(cut_end_time / cut_dt_s) : 0ULL;
		const unsigned long long cooldown_steps =
			(cooldown_dt_s > 0.0) ? (unsigned long long)std::ceil(cooldown_max_time_s / cooldown_dt_s) : 0ULL;
		num_step = cut_steps + cooldown_steps;
	} else {
		num_step = (time->get_dt() > 0.0) ? (unsigned long long)std::ceil(time->get_t_final() / time->get_dt()) : 0ULL;
	}

	unsigned int freq = (num_step > 0) ? (unsigned int)(num_step / (unsigned long long)num_print) : 1;
	unsigned int print_iter = 0;
	auto begin = std::chrono::high_resolution_clock::now();

	freq = std::max(1, (int)freq);
	if (all_steps || (use_config && cfg.io.all_steps))
		freq = 1;
	printf("starting simulation: particles=%u dt=%e t_final=%e steps=%llu print_every=%u\n", (*b).get_num_part(), time->get_dt(),
		   time->get_t_final(), num_step, freq);

	/*
	  ========================
	  (2nd-order) LeapFrog scheme is used
	  for the explicit time integration.
	  ========================
	 */
	leap_frog stepper((*b).get_num_part());

	/*
	 * This is the implementation of the main time-loop,
	 * also illustrated by the following flowchart in the paper:
	 * ---------------------------------------------------------
	 * Section 4:
	 * Fig. 5. Flowchart of the model logic for each time-step.
	 *
	 */
	try {
		bool cooldown_started = false;
		unsigned int fixed_count_initial = 0;
		for (unsigned int i = 0; i < b->get_num_part(); i++) {
			if (b->get_particles()[i].fixed)
				fixed_count_initial++;
		}

		std::ofstream cooldown_csv;
		std::ofstream cooldown_summary;
		if (cooldown) {
			cooldown_csv.open((folder / "cooldown_rate.csv").string(), std::ios::out);
			cooldown_summary.open((folder / "cooldown_summary.txt").string(), std::ios::out);
			cooldown_csv << "time_s,max_T_C,min_T_C,max_abs_dTdt_C_per_min,conv_ramp\n";
			cooldown_summary << "ambient_T_C,22\n";
			cooldown_summary << "ambient_stop_T_C,22.5\n";
			cooldown_summary << "max_rate_C_per_min,5\n";
			cooldown_summary << "cooldown_hconv_W_m2K," << cooldown_hconv_W_m2K << "\n";
			cooldown_summary << "cooldown_remove_tool," << (cooldown_remove_tool ? 1 : 0) << "\n";
			cooldown_summary << "fixed_particles_initial," << fixed_count_initial << "\n";
		}

		std::vector<double> prev_T;
		double worst_max_abs_rate_C_per_min = 0.0;
		double worst_positive_deltaT_C = 0.0;

		while (!time->finished()) {
			if (cooldown && !cooldown_started && (time->get_time() + time->get_dt()) >= cut_end_time) {
				thermal *trml = b->get_thermal();
				if (trml) {
					trml->set_convection(cooldown_hconv_W_m2K, ambient_T_K);
					trml->set_max_cooling_rate(max_rate_K_per_s);
					trml->set_convection_enabled(true);
				}
				if (global_logger) {
					global_logger->set_stage("cooldown");
				}

				tool *t = b->get_tool();
				if (t) {
					if (cooldown_remove_tool) {
						b->set_tool(nullptr);
						if (global_logger)
							global_logger->set_tool(nullptr);
					} else {
						glm::dvec2 v = t->get_vel();
						const double vmag = std::sqrt(v.x * v.x + v.y * v.y);
						t->set_vel(glm::dvec2(0.0, vmag));
					}
				}

				cooldown_started = true;
				time->set_dt(cooldown_dt_s);
				if (all_steps)
					freq = 1;
				prev_T.assign(b->get_num_part(), 0.0);
				if (global_logger) {
					global_logger->log(*b, print_iter);
					print_iter++;
				}
			}

			if (cooldown_started) {
				for (unsigned int i = 0; i < b->get_num_part(); i++) {
					prev_T[i] = b->get_particles()[i].T;
				}
			}

			// plot with given frequency
			if (time->get_step() % freq == 0) {
				if (global_logger) {
					global_logger->log(*b, print_iter);
				}

				auto intermediate = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> diff = intermediate - begin;
				double seconds_so_far = diff.count();

				double percent_done = (num_step > 0) ? (100.0 * time->get_step() / ((double)num_step)) : 0.0;
				double seconds_left = 0.0;
				if (percent_done > 0.0) {
					double time_left = seconds_so_far / percent_done * 100.0;
					seconds_left = time_left - seconds_so_far;
				}

				printf("%06d: #increments %06d, cur time %e, pctg done %f, seconds left: %f\n", print_iter, time->get_step(),
					   time->get_time(), percent_done, seconds_left);
				fflush(stdout);
				print_iter++;
			}

			/* Carry out the time-stepper:
			 * this is to update the system by evolving the variables
			 * over time using the LeapFrog time stepping
			 */
			if (!cooldown_started) {
				stepper.step(*b);
			} else {
				b->construct_verlet_lists();

#pragma omp parallel for
				for (unsigned int i = 0; i < b->get_num_part(); i++) {
					b->get_particles()[i].T_t = 0.0;
				}

				b->apply_thermal_conduction();

				const double dt = time->get_dt();
#pragma omp parallel for
				for (unsigned int i = 0; i < b->get_num_part(); i++) {
					b->get_particles()[i].T += dt * b->get_particles()[i].T_t;
				}

				b->move_tool();
				material_eos(*b);
			}

			if (cooldown_started) {
				const double dt = time->get_dt();
				double max_T = -DBL_MAX;
				double min_T = DBL_MAX;
				double max_abs_rate_C_per_min = 0.0;
				double max_positive_deltaT_C = 0.0;
				for (unsigned int i = 0; i < b->get_num_part(); i++) {
					const double T = b->get_particles()[i].T;
					if (T > max_T)
						max_T = T;
					if (T < min_T)
						min_T = T;
					const double dT = T - prev_T[i];
					const double abs_rate_C_per_min = (dt > 0.0) ? (std::abs(dT) / dt * 60.0) : 0.0;
					if (abs_rate_C_per_min > max_abs_rate_C_per_min)
						max_abs_rate_C_per_min = abs_rate_C_per_min;
					if (dT > max_positive_deltaT_C)
						max_positive_deltaT_C = dT;
				}

				if (max_abs_rate_C_per_min > worst_max_abs_rate_C_per_min)
					worst_max_abs_rate_C_per_min = max_abs_rate_C_per_min;
				if (max_positive_deltaT_C > worst_positive_deltaT_C)
					worst_positive_deltaT_C = max_positive_deltaT_C;

				double ramp = 1.0;
				thermal *trml = b->get_thermal();
				if (trml)
					ramp = trml->last_convection_ramp();

				if (cooldown_csv.is_open()) {
					cooldown_csv << time->get_time() << "," << (max_T - 273.15) << "," << (min_T - 273.15) << "," << max_abs_rate_C_per_min
								 << "," << ramp << "\n";
				}

				if (max_T <= ambient_T_stop_K && (max_T - min_T) <= 0.5) {
					if (global_logger) {
						vtk_writer_write(b->get_particles(), print_iter, "results", "residual-stress-ready", "residual-stress-ready");
						tool *t = b->get_tool();
						if (t) {
							vtk_writer_write(t, print_iter, "results", "residual-stress-ready", "residual-stress-ready");
						}
					}
					break;
				}
			}

			time->increment_step();
			time->increment_time();
		}

		if (cooldown && cooldown_summary.is_open()) {
			unsigned int fixed_count_final = 0;
			for (unsigned int i = 0; i < b->get_num_part(); i++) {
				if (b->get_particles()[i].fixed)
					fixed_count_final++;
			}
			double max_T = -DBL_MAX;
			for (unsigned int i = 0; i < b->get_num_part(); i++) {
				if (b->get_particles()[i].T > max_T)
					max_T = b->get_particles()[i].T;
			}
			cooldown_summary << "fixed_particles_final," << fixed_count_final << "\n";
			cooldown_summary << "max_final_T_C," << (max_T - 273.15) << "\n";
			cooldown_summary << "worst_max_abs_dTdt_C_per_min," << worst_max_abs_rate_C_per_min << "\n";
			cooldown_summary << "worst_positive_deltaT_C," << worst_positive_deltaT_C << "\n";
		}
	} catch (const std::exception &e) {
		std::cerr << "\n[CRITICAL ERROR] Simulation Terminated: " << e.what() << std::endl;
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << "\n[CRITICAL ERROR] Simulation Terminated: Unknown Exception" << std::endl;
		return EXIT_FAILURE;
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - begin;
	printf("Runtime: %f\n", elapsed.count());

	return EXIT_SUCCESS;
}
