/*
 * ================================================
 * 					COPYRIGHT:
 * Institute of Machine Tools & Manufacturing (IWF)
 * Department of Mechanical & Process Engineering
 * 					ETH ZURICH
 * ================================================
 */

#include "timestep_estimator.h"

#include "fe_tool.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

namespace {
static void consider_limit(double value, const char *reason, double &current, std::string &current_reason) {
	if (!std::isfinite(value) || value <= 0.) return;
	if (!std::isfinite(current) || current <= 0. || value < current) {
		current = value;
		current_reason = reason;
	}
}

static double safe_positive(double value) {
	return (std::isfinite(value) && value > 0.) ? value : 0.;
}
} // namespace

coupled_timestep_limits estimate_coupled_timestep(const physical_constants &workpiece, const coupled_timestep_config &config,
                                                  const fe_tool *tool) {
	coupled_timestep_limits limits;

	const double dx = safe_positive(config.particle_spacing);
	const double hdx = safe_positive(config.smoothing_length_ratio);
	const double vmax = std::max(0., std::isfinite(config.max_relative_speed) ? config.max_relative_speed : 0.);

	if (dx > 0. && hdx > 0.) {
		double c_wp = workpiece.c0();
		if (std::isfinite(c_wp) && c_wp > 0.) {
			limits.workpiece_mechanical_dt = config.workpiece_mechanical_safety * hdx * dx / (c_wp + vmax);
		}
	}

	const double rho_wp = workpiece.rho0();
	const double cp_wp = workpiece.tc().cp();
	const double k_wp = workpiece.tc().k();
	if (dx > 0. && rho_wp > 0. && cp_wp > 0. && k_wp > 0.) {
		const double alpha_wp = k_wp / (rho_wp * cp_wp);
		if (std::isfinite(alpha_wp) && alpha_wp > 0.) limits.workpiece_thermal_dt = config.workpiece_thermal_safety * dx * dx / alpha_wp;
	}

	if (tool) {
		double dt_tool_mech = tool->mechanics_dt_crit();
		if (std::isfinite(dt_tool_mech) && dt_tool_mech > 0.) limits.tool_mechanical_dt = config.tool_mechanical_safety * dt_tool_mech;

		double dt_tool_thermal = tool->thermal_dt_crit();
		if (std::isfinite(dt_tool_thermal) && dt_tool_thermal > 0.) limits.tool_thermal_dt = config.tool_thermal_safety * dt_tool_thermal;

		const double h_contact = safe_positive(config.contact_conductance_full);
		const double A_contact = safe_positive(config.interface_contact_area);
		const double C_wp = dx > 0. && rho_wp > 0. && cp_wp > 0. ? rho_wp * dx * dx * cp_wp : 0.;
		const double C_tool = tool->min_thermal_nodal_capacity();
		if (h_contact > 0. && A_contact > 0. && C_wp > 0. && std::isfinite(C_tool) && C_tool > 0.) {
			const double conductance = h_contact * A_contact;
			const double inv_capacity_sum = (1.0 / C_wp) + (1.0 / C_tool);
			if (conductance > 0. && std::isfinite(inv_capacity_sum) && inv_capacity_sum > 0.) {
				limits.interface_thermal_dt = config.interface_thermal_safety * 2.0 / (conductance * inv_capacity_sum);
			}
		}
	}

	if (std::isfinite(config.empirical_dt_cap) && config.empirical_dt_cap > 0.) limits.empirical_dt = config.empirical_dt_cap;

	limits.maximum_dt = std::numeric_limits<double>::infinity();
	consider_limit(limits.workpiece_mechanical_dt, "workpiece_mechanical", limits.maximum_dt, limits.limiting_reason);
	consider_limit(limits.workpiece_thermal_dt, "workpiece_thermal", limits.maximum_dt, limits.limiting_reason);
	consider_limit(limits.tool_mechanical_dt, "tool_mechanical", limits.maximum_dt, limits.limiting_reason);
	consider_limit(limits.tool_thermal_dt, "tool_thermal", limits.maximum_dt, limits.limiting_reason);
	consider_limit(limits.interface_thermal_dt, "interface_thermal", limits.maximum_dt, limits.limiting_reason);
	consider_limit(limits.empirical_dt, "empirical", limits.maximum_dt, limits.limiting_reason);

	if (!std::isfinite(limits.maximum_dt)) {
		limits.maximum_dt = 0.;
		limits.limiting_reason = "none";
	}
	return limits;
}

void print_coupled_timestep_limits(const coupled_timestep_limits &limits) {
	std::printf("timestep estimate: dt=%e limiter=%s wp_mech=%e wp_therm=%e tool_mech=%e tool_therm=%e interface_therm=%e empirical=%e\n",
	            limits.maximum_dt,
	            limits.limiting_reason.c_str(),
	            limits.workpiece_mechanical_dt,
	            limits.workpiece_thermal_dt,
	            limits.tool_mechanical_dt,
	            limits.tool_thermal_dt,
	            limits.interface_thermal_dt,
	            limits.empirical_dt);
}
