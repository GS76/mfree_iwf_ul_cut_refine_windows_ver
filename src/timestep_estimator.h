/*
 * ================================================
 * 					COPYRIGHT:
 * Institute of Machine Tools & Manufacturing (IWF)
 * Department of Mechanical & Process Engineering
 * 					ETH ZURICH
 * ================================================
 */

#ifndef TIMESTEP_ESTIMATOR_H_
#define TIMESTEP_ESTIMATOR_H_

#include "simulation_data.h"

#include <string>

class fe_tool;

struct coupled_timestep_limits {
	double workpiece_mechanical_dt = 0.;
	double workpiece_thermal_dt = 0.;
	double tool_mechanical_dt = 0.;
	double tool_thermal_dt = 0.;
	double interface_thermal_dt = 0.;
	double empirical_dt = 0.;
	double maximum_dt = 0.;
	std::string limiting_reason;
};

struct coupled_timestep_config {
	double particle_spacing = 0.;
	double smoothing_length_ratio = 1.;
	double max_relative_speed = 0.;
	double empirical_dt_cap = 0.;
	double workpiece_mechanical_safety = 0.25;
	double workpiece_thermal_safety = 0.20;
	double tool_mechanical_safety = 0.90;
	double tool_thermal_safety = 1.00;
	double interface_thermal_safety = 0.50;
	double interface_contact_area = 0.;
	double contact_conductance_full = 100000.0;
};

coupled_timestep_limits estimate_coupled_timestep(const physical_constants &workpiece, const coupled_timestep_config &config,
                                                  const fe_tool *tool = nullptr);

void print_coupled_timestep_limits(const coupled_timestep_limits &limits);

#endif /* TIMESTEP_ESTIMATOR_H_ */
