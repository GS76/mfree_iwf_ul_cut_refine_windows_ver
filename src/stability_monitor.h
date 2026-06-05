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
 */

#ifndef STABILITY_MONITOR_H_
#define STABILITY_MONITOR_H_

#include "body.h"
#include <vector>
#include <string>

// Result structure for stability checks
struct stability_check_result {
	bool is_valid = true;
	bool has_nan = false;
	bool has_inf = false;
	bool velocity_excessive = false;
	bool temperature_extreme = false;
	bool density_negative = false;
	bool stress_invalid = false;
	unsigned int num_invalid_particles = 0;
	unsigned int first_invalid_idx = 0;
	double max_velocity_magnitude = 0.0;
	double max_temperature = 0.0;
	double min_temperature = 0.0;
	std::string failure_reason;
};

// Configuration for stability monitoring (populated from environment variables)
struct stability_config {
	bool enabled = false;
	double max_velocity_factor = 0.5;           // max velocity as fraction of sound speed
	double energy_closure_warning_threshold = 10.0;   // % threshold for warning
	double energy_closure_critical_threshold = 50.0; // % threshold for timestep reduction
	double min_timestep = 1e-12;              // minimum viable dt before termination
	int max_timestep_reductions = 10;          // max consecutive reductions before abort
	int validation_frequency = 1;             // validate every N steps (1 = every step)
	double temperature_min_K = 200.0;           // minimum physically reasonable T
	double temperature_max_K = 5000.0;          // maximum physically reasonable T
	
	// Load configuration from environment variables
	void load_from_env();
};

// Validate all particles in the body
// Returns result with is_valid=false if any particle has NaN, Inf, or exceeds bounds
stability_check_result validate_particle_state(const body &b, const stability_config &config);

// Check if a single particle is valid
bool is_particle_valid(const particle &p, const stability_config &config, std::string &reason);

// Energy closure monitoring
struct energy_closure_result {
	double closure_residual_pct = 0.0;
	double cum_suppression_ratio = 0.0;
	bool is_critical = false;
	bool is_warning = false;
};

// Check energy closure from the logger's recorded data
energy_closure_result check_energy_closure(const body &b);

// Adaptive timestep control
class adaptive_timestep_controller {
public:
	void initialize(double initial_dt, const stability_config &config);
	
	// Call when instability detected - returns true if timestep was reduced
	bool on_instability_detected();
	
	// Call when energy closure critical - returns true if timestep was reduced
	bool on_energy_closure_critical(double closure_pct);
	
	// Check if we've exceeded max reductions
	bool should_terminate() const;
	
	// Get current effective timestep
	double get_current_dt() const { return m_current_dt; }
	
	// Get number of reductions performed
	int get_reduction_count() const { return m_reduction_count; }
	
	// Get termination reason if should_terminate() is true
	std::string get_termination_reason() const { return m_termination_reason; }
	
	// Reset to initial state
	void reset();
	
private:
	double m_initial_dt = 0.0;
	double m_current_dt = 0.0;
	double m_min_timestep = 1e-12;
	int m_max_reductions = 10;
	int m_reduction_count = 0;
	std::string m_termination_reason;
	static constexpr double REDUCTION_FACTOR = 0.5;
};

// Global stability monitor instance (singleton pattern)
class stability_monitor {
public:
	static stability_monitor &get_instance();
	
	// Initialize with current simulation configuration
	void initialize(double initial_dt);
	
	// Perform validation check on the body
	// Returns true if simulation should continue, false if should terminate
	bool validate_step(const body &b, std::string &status_message);
	
	// Get the current adaptive timestep (may be reduced from initial)
	double get_adaptive_dt() const;
	
	// Check if timestep has been adapted
	bool is_dt_adapted() const { return m_controller.get_reduction_count() > 0; }
	
	// Get configuration
	const stability_config &get_config() const { return m_config; }
	
	// Get last validation result
	const stability_check_result &get_last_validation() const { return m_last_validation; }
	
	// Reset for new simulation
	void reset();
	
	// Enable/disable monitoring at runtime
	void set_enabled(bool enabled) { m_config.enabled = enabled; }
	bool is_enabled() const { return m_config.enabled; }
	
private:
	stability_monitor() = default;
	~stability_monitor() = default;
	stability_monitor(const stability_monitor &) = delete;
	stability_monitor &operator=(const stability_monitor &) = delete;
	
	stability_config m_config;
	adaptive_timestep_controller m_controller;
	stability_check_result m_last_validation;
	unsigned int m_step_count = 0;
	bool m_initialized = false;
};

#endif /* STABILITY_MONITOR_H_ */