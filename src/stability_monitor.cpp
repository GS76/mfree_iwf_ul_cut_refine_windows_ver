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

#include "stability_monitor.h"
#include "simulation_time.h"
#include "logger.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <omp.h>
#include <limits>

// Helper to check if value is finite (not NaN or Inf)
static inline bool is_finite(double val) {
	return std::isfinite(val);
}

// Load configuration from environment variables
void stability_config::load_from_env() {
	const char *val = nullptr;
	
	val = std::getenv("MFREE_ENABLE_STABILITY_MONITOR");
	if (val) enabled = (std::atoi(val) != 0);
	
	val = std::getenv("MFREE_MAX_VELOCITY_FACTOR");
	if (val) max_velocity_factor = std::atof(val);
	
	val = std::getenv("MFREE_ENERGY_CLOSURE_THRESHOLD");
	if (val) energy_closure_warning_threshold = std::atof(val);
	
	val = std::getenv("MFREE_ENERGY_CLOSURE_CRITICAL");
	if (val) energy_closure_critical_threshold = std::atof(val);
	
	val = std::getenv("MFREE_MIN_TIMESTEP");
	if (val) min_timestep = std::atof(val);
	
	val = std::getenv("MFREE_MAX_TIMESTEP_REDUCTIONS");
	if (val) max_timestep_reductions = std::atoi(val);
	
	val = std::getenv("MFREE_STABILITY_VALIDATION_FREQ");
	if (val) validation_frequency = std::atoi(val);
	
	val = std::getenv("MFREE_TEMP_MIN_K");
	if (val) temperature_min_K = std::atof(val);
	
	val = std::getenv("MFREE_TEMP_MAX_K");
	if (val) temperature_max_K = std::atof(val);
}

// Check if a single particle is valid
bool is_particle_valid(const particle &p, const stability_config &config, std::string &reason) {
	reason.clear();
	
	// Check position (x, y)
	if (!is_finite(p.x) || !is_finite(p.y)) {
		reason = "position contains NaN or Inf";
		return false;
	}
	
	// Check velocity
	if (!is_finite(p.vx) || !is_finite(p.vy)) {
		reason = "velocity contains NaN or Inf";
		return false;
	}
	
	// Check temperature
	if (!is_finite(p.T)) {
		reason = "temperature is NaN or Inf";
		return false;
	}
	if (p.T < config.temperature_min_K || p.T > config.temperature_max_K) {
		reason = "temperature outside physical bounds (" + 
				 std::to_string(config.temperature_min_K) + "-" + 
				 std::to_string(config.temperature_max_K) + " K)";
		return false;
	}
	
	// Check density
	if (!is_finite(p.rho)) {
		reason = "density is NaN or Inf";
		return false;
	}
	if (p.rho <= 0.0) {
		reason = "density is negative or zero";
		return false;
	}
	
	// Check stress components
	if (!is_finite(p.Sxx) || !is_finite(p.Sxy) || !is_finite(p.Syy) || !is_finite(p.Szz)) {
		reason = "stress components contain NaN or Inf";
		return false;
	}
	
	// Check smoothing length
	if (!is_finite(p.h) || p.h <= 0.0) {
		reason = "smoothing length is invalid";
		return false;
	}
	
	return true;
}

// Validate all particles in the body
stability_check_result validate_particle_state(const body &b, const stability_config &config) {
	stability_check_result result;
	result.is_valid = true;
	
	const auto &particles = b.get_particles();
	const unsigned int num_part = b.get_num_part();
	
	if (num_part == 0) {
		return result;
	}
	
	// Get sound speed from physical constants for velocity check
	double c0 = 0.0;
	const auto &phys_const = b.get_sim_data().get_physical_constants();
	c0 = phys_const.c0();
	double max_allowed_velocity = config.max_velocity_factor * c0;
	
	// Thread-local results for reduction
	unsigned int num_invalid = 0;
	unsigned int first_invalid = num_part;
	double max_vel_mag = 0.0;
	double max_T = -std::numeric_limits<double>::infinity();
	double min_T = std::numeric_limits<double>::infinity();
	
	bool has_nan = false;
	bool has_inf = false;
	bool velocity_excessive = false;
	std::string first_reason;
	
	#pragma omp parallel reduction(+:num_invalid) reduction(max:max_vel_mag) \
					 reduction(max:max_T) reduction(min:min_T) reduction(min:first_invalid)
	{
		std::string local_reason;
		
		#pragma omp for
		for (unsigned int i = 0; i < num_part; i++) {
			const particle &p = particles[i];
			
			// Check basic validity
			bool valid = is_particle_valid(p, config, local_reason);
			if (!valid) {
				num_invalid++;
				if (i < first_invalid) {
					first_invalid = i;
					#pragma omp critical
					{
						if (first_reason.empty()) {
							first_reason = local_reason;
						}
					}
				}
				continue;
			}
			
			// Velocity magnitude check
			double vel_mag = std::sqrt(p.vx * p.vx + p.vy * p.vy);
			if (vel_mag > max_vel_mag) {
				max_vel_mag = vel_mag;
			}
			if (max_allowed_velocity > 0 && vel_mag > max_allowed_velocity) {
				velocity_excessive = true;
			}
			
			// Temperature tracking
			if (p.T > max_T) max_T = p.T;
			if (p.T < min_T) min_T = p.T;
		}
	}
	
	result.num_invalid_particles = num_invalid;
	result.first_invalid_idx = (first_invalid < num_part) ? first_invalid : 0;
	result.max_velocity_magnitude = max_vel_mag;
	result.max_temperature = max_T;
	result.min_temperature = min_T;
	result.has_nan = has_nan;
	result.has_inf = has_inf;
	result.velocity_excessive = velocity_excessive;
	
	if (num_invalid > 0) {
		result.is_valid = false;
		result.failure_reason = "Particle " + std::to_string(result.first_invalid_idx) + ": " + first_reason;
	} else if (velocity_excessive) {
		result.is_valid = false;
		result.failure_reason = "Velocity exceeds maximum allowed (" + 
								 std::to_string(max_allowed_velocity) + " m/s)";
	}
	
	return result;
}

// Check energy closure - uses logger's computed data
// This is a simplified check - the full closure is computed by the logger
double compute_energy_closure_residual(const body &b);

energy_closure_result check_energy_closure(const body &b) {
	energy_closure_result result;
	
	// Get from simulation data if available, or compute inline
	// For now, we'll rely on the logger's tracking
	// This function can be enhanced to access logger state directly
	
	return result;
}

// Adaptive timestep controller implementation
void adaptive_timestep_controller::initialize(double initial_dt, const stability_config &config) {
	m_initial_dt = initial_dt;
	m_current_dt = initial_dt;
	m_min_timestep = config.min_timestep;
	m_max_reductions = config.max_timestep_reductions;
	m_reduction_count = 0;
	m_termination_reason.clear();
}

bool adaptive_timestep_controller::on_instability_detected() {
	if (m_reduction_count >= m_max_reductions) {
		m_termination_reason = "Maximum timestep reductions (" + 
							   std::to_string(m_max_reductions) + 
							   ") exceeded due to persistent instability";
		return false;
	}
	
	if (m_current_dt <= m_min_timestep) {
		m_termination_reason = "Timestep reduced to minimum (" + 
							   std::to_string(m_min_timestep) + 
							   " s), cannot proceed safely";
		return false;
	}
	
	m_current_dt *= REDUCTION_FACTOR;
	m_reduction_count++;
	
	std::printf("[STABILITY] Timestep reduced to %e s (reduction %d/%d)\n",
				m_current_dt, m_reduction_count, m_max_reductions);
	
	return true;
}

bool adaptive_timestep_controller::on_energy_closure_critical(double closure_pct) {
	// Energy closure critical triggers the same response as instability
	std::printf("[STABILITY] Energy closure critical: %.2f%%\n", closure_pct);
	return on_instability_detected();
}

bool adaptive_timestep_controller::should_terminate() const {
	return !m_termination_reason.empty();
}

void adaptive_timestep_controller::reset() {
	m_current_dt = m_initial_dt;
	m_reduction_count = 0;
	m_termination_reason.clear();
}

// Singleton instance
stability_monitor &stability_monitor::get_instance() {
	static stability_monitor instance;
	return instance;
}

void stability_monitor::initialize(double initial_dt) {
	m_config.load_from_env();
	m_controller.initialize(initial_dt, m_config);
	m_step_count = 0;
	m_initialized = true;
	
	if (m_config.enabled) {
		std::printf("[STABILITY] Monitoring enabled:\n");
		std::printf("  max_velocity_factor: %.2f\n", m_config.max_velocity_factor);
		std::printf("  energy_closure_warning: %.1f%%\n", m_config.energy_closure_warning_threshold);
		std::printf("  energy_closure_critical: %.1f%%\n", m_config.energy_closure_critical_threshold);
		std::printf("  min_timestep: %e s\n", m_config.min_timestep);
		std::printf("  max_reductions: %d\n", m_config.max_timestep_reductions);
	}
}

bool stability_monitor::validate_step(const body &b, std::string &status_message) {
	status_message.clear();
	
	if (!m_initialized || !m_config.enabled) {
		return true; // Continue if not initialized or disabled
	}
	
	m_step_count++;
	
	// Only validate every N steps if configured
	if ((m_step_count % m_config.validation_frequency) != 0) {
		return true;
	}
	
	// Perform particle validation
	m_last_validation = validate_particle_state(b, m_config);
	
	if (!m_last_validation.is_valid) {
		// Instability detected - try to adapt
		if (m_controller.on_instability_detected()) {
			// Timestep was reduced, continue with warning
			status_message = "[WARNING] " + m_last_validation.failure_reason + 
							 "; timestep reduced to " + 
							 std::to_string(m_controller.get_current_dt()) + " s";
			std::printf("%s\n", status_message.c_str());
			return true; // Continue with reduced timestep
		} else {
			// Cannot reduce further - must terminate
			status_message = m_controller.get_termination_reason() + 
							 "; last validation: " + m_last_validation.failure_reason;
			return false; // Signal termination
		}
	}
	
	// Check if we should terminate due to max reductions being reached
	if (m_controller.should_terminate()) {
		status_message = m_controller.get_termination_reason();
		return false;
	}
	
	return true;
}

double stability_monitor::get_adaptive_dt() const {
	if (!m_initialized) {
		// Return current simulation dt if not initialized
		simulation_time *time = &simulation_time::getInstance();
		return time->get_dt();
	}
	return m_controller.get_current_dt();
}

void stability_monitor::reset() {
	m_controller.reset();
	m_step_count = 0;
	m_initialized = false;
}
