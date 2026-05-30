#include "config/build_from_config.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

#include "benchmarks/material_library.h"
#include "body.h"
#include "johnson_cook_Sima_2010.h"
#include "kernel.h"
#include "logger.h"
#include "plasticity.h"
#include "simulation_data.h"
#include "simulation_time.h"
#include "thermal.h"
#include "tool.h"

extern logger *global_logger;

namespace mfree::config {

static physical_constants select_physical_constants(const simulation_config &cfg) {
	if (cfg.material.physical_constants == "tial6v4_sima_tanh2010_si")
		return matlib_tial6v4_Sima_tanh2010_SI();
	throw std::runtime_error("Unsupported material.physical_constants: " + cfg.material.physical_constants);
}

static thermal::thermal_solver select_thermal_solver(const simulation_config &cfg) {
	if (cfg.thermal.method == "thermal_pse")
		return thermal::thermal_solver::thermal_pse;
	if (cfg.thermal.method == "thermal_brookshaw")
		return thermal::thermal_solver::thermal_brookshaw;
	throw std::runtime_error("Unsupported thermal.method: " + cfg.thermal.method);
}

static void set_time_from_config(const simulation_config &cfg, double dx, double vc, const physical_constants &pc) {
	const double rho0 = pc.rho0();
	const double thermal_diffusivity = pc.tc().k() / (rho0 * pc.tc().cp());

	double t_final = cfg.time.t_final_override ? *cfg.time.t_final_override : (cfg.time.cut_length / vc);
	double dt = 0.0;
	if (cfg.time.dt_override) {
		dt = *cfg.time.dt_override;
	} else {
		const double mech_CFL = 0.5 * cfg.numerical.hdx * dx / (pc.c0() + vc);
		const double heat_CFL = 0.4 * dx * dx / (thermal_diffusivity);
		const double dt_mech = std::fmin(cfg.time.dt_empirical, cfg.time.mech_cfl_factor * mech_CFL);
		const double dt_heat = std::fmin(cfg.time.dt_empirical, cfg.time.heat_cfl_factor * heat_CFL);
		dt = std::fmin(dt_mech, dt_heat);
	}

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);
}

struct grid_dims {
	double dx = 0.0;
	double dy = 0.0;
	unsigned int nx = 0;
	unsigned int ny = 0;
};

static grid_dims compute_grid(const simulation_config &cfg) {
	grid_dims g;
	if (cfg.workpiece.keep_base_spacing) {
		const double dy_base = cfg.workpiece.base_height_y / (cfg.model.nbox - 1);
		g.dx = dy_base;
		g.dy = dy_base;
		g.ny = (unsigned int)(std::floor((cfg.workpiece.hi_y - cfg.workpiece.lo_y) / g.dy + 1.0) + 0u);
	} else {
		g.ny = (unsigned int)cfg.model.nbox;
		g.dy = (cfg.workpiece.hi_y - cfg.workpiece.lo_y) / (g.ny - 1);
		g.dx = g.dy;
	}
	g.nx = (unsigned int)(std::floor((cfg.workpiece.hi_x - cfg.workpiece.lo_x) / g.dx + 1.0) + 0u);
	return g;
}

static correction_constants build_corrections(const simulation_config &cfg, double dx) {
	kernel_result w = cubic_spline(0, 0, dx, 0, cfg.numerical.hdx * dx);
	double wdeltap = w.w;
	return correction_constants(constants_monaghan(wdeltap, cfg.numerical.stress_exponent, cfg.numerical.art_stress_eps),
								constants_artificial_viscosity(cfg.numerical.alpha, cfg.numerical.beta, cfg.numerical.eta),
								cfg.numerical.xsph_eps);
}

static void init_common_particle_fields(std::vector<particle> &particles, unsigned int n, double rho0, double T0) {
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = T0;
		particles[i].T_init = T0;
	}
}

static tool *build_tool_from_config(const simulation_config &cfg, double lo_x, double hi_y, double dx, double vc) {
	const double nudge = -dx;
	const glm::dvec2 tl(lo_x - cfg.tool.length - cfg.tool.tool_right_clearance + nudge + cfg.tool.tool_x_shift, cfg.tool.tl_y);
	tool *t = new tool(tl, cfg.tool.length, cfg.tool.height, cfg.tool.rake_deg, cfg.tool.clearance_deg, cfg.tool.fillet_radius,
					   cfg.tool.mu_friction);

	const double target_feed = cfg.tool.target_feed;
	const double current_feed = hi_y - t->low();
	const double dist_to_target_feed = std::fabs(current_feed - target_feed);
	const double correction_time = dist_to_target_feed / vc;
	const double sign = (current_feed > target_feed) ? 1.0 : -1.0;
	t->set_vel(glm::dvec2(0.0, vc));
	t->update_tool(correction_time * sign);
	t->set_vel(glm::dvec2(vc, 0.0));
	return t;
}

static plasticity *build_plasticity_from_config(const simulation_config &cfg, const physical_constants &pc) {
	if (!cfg.plasticity.enabled)
		return nullptr;
	if (cfg.plasticity.model != "johnson_cook_sima_2010") {
		throw std::runtime_error("Unsupported plasticity.model: " + cfg.plasticity.model);
	}
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc));
	plast->set_tolerance(cfg.plasticity.tolerance);
	plast->set_dissipation_considered(cfg.plasticity.dissipation_considered);
	return plast;
}

static thermal *build_thermal_from_config(const simulation_config &cfg, const physical_constants &pc) {
	if (!cfg.thermal.enabled)
		return nullptr;
	thermal *trml = new thermal(pc);
	trml->set_method(select_thermal_solver(cfg));
	return trml;
}

static void set_default_logger(tool *t) {
	global_logger = new logger("cutting");
	global_logger->set_tool(t);
	global_logger->set_log_vtk(true);
}

static body *build_single_resolution(const simulation_config &cfg) {
	const physical_constants pc = select_physical_constants(cfg);
	const double rho0 = pc.rho0();
	const double T0 = cfg.thermal.T0;

	const grid_dims g = compute_grid(cfg);
	set_time_from_config(cfg, g.dx, cfg.tool.cutting_speed, pc);

	particle *particles_raw = new particle[g.nx * g.ny];
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < g.nx; i++) {
		for (unsigned int j = 0; j < g.ny; j++) {
			double px = i * g.dx;
			double py = j * g.dy;
			particles_raw[part_iter] = particle(part_iter);
			particles_raw[part_iter].x = px + cfg.workpiece.lo_x;
			particles_raw[part_iter].y = py + cfg.workpiece.lo_y;
			particles_raw[part_iter].X = particles_raw[part_iter].x;
			particles_raw[part_iter].Y = particles_raw[part_iter].y;
			part_iter++;
		}
	}

	const unsigned int n = g.nx * g.ny;
	for (unsigned int i = 0; i < n; i++) {
		particles_raw[i].rho = rho0;
		particles_raw[i].h = cfg.numerical.hdx * g.dx;
		particles_raw[i].m = g.dx * g.dy * rho0;
		particles_raw[i].T = T0;
		particles_raw[i].T_init = T0;

		particles_raw[i].fixed = (particles_raw[i].y < cfg.workpiece.lo_y + 0.5 * g.dy);
		particles_raw[i].fixed = particles_raw[i].fixed || (particles_raw[i].x > cfg.workpiece.hi_x - 0.5 * g.dx);
	}

	const correction_constants cs = build_corrections(cfg, g.dx);
	simulation_data sim_data(pc, cs);

	body *b = new body(particles_raw, n, sim_data);

	plasticity *plast = build_plasticity_from_config(cfg, pc);
	thermal *trml = build_thermal_from_config(cfg, pc);
	tool *t = build_tool_from_config(cfg, cfg.workpiece.lo_x, cfg.workpiece.hi_y, g.dx, cfg.tool.cutting_speed);

	if (plast)
		b->set_plasticity(plast);
	if (trml)
		b->set_thermal(trml);
	b->set_tool(t);

	set_default_logger(t);
	return b;
}

static body *build_apriori_refinement(const simulation_config &cfg) {
	const physical_constants pc = select_physical_constants(cfg);
	const double rho0 = pc.rho0();
	const double T0 = cfg.thermal.T0;

	const double lx = cfg.workpiece.hi_x - cfg.workpiece.lo_x;
	const double ly = cfg.workpiece.hi_y - cfg.workpiece.lo_y;

	const grid_dims g = compute_grid(cfg);
	set_time_from_config(cfg, g.dx, cfg.tool.cutting_speed, pc);

	const double resol_ratio = cfg.multires.resol_ratio;
	const double py_split = cfg.multires.py_split_fraction * ly + cfg.workpiece.lo_y;
	const double dxh = g.dx;
	const double dxl = dxh * resol_ratio;
	const unsigned int nxh = g.nx;
	const unsigned int nyh = g.ny;
	const unsigned int nxl = (unsigned int)(std::floor(lx / dxl) + 1.0);
	const unsigned int nyl = (unsigned int)(std::floor(ly / dxl) + 1.0);
	const double dVl = dxl * dxl;
	const double dVh = dxh * dxh;
	const double h0l = cfg.numerical.hdx * dxl;
	const double h0h = cfg.numerical.hdx * dxh;

	particle *particles = new particle[nxh * nyh];
	std::srand(0);
	unsigned int part_iter = 0;

	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i * dxh;
			double pyh = j * dxh;
			if ((pyh + cfg.workpiece.lo_y) < (py_split - cfg.multires.py_margin_factor * dxh))
				continue;
			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + cfg.workpiece.lo_x;
			particles[part_iter].y = pyh + cfg.workpiece.lo_y;
			particles[part_iter].X = particles[part_iter].x;
			particles[part_iter].Y = particles[part_iter].y;
			particles[part_iter].refine_step = 1;
			part_iter++;
		}
	}

	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i * dxl;
			double pyl = j * dxl;
			if ((pyl + cfg.workpiece.lo_y) >= py_split)
				continue;
			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + cfg.workpiece.lo_x;
			particles[part_iter].y = pyl + cfg.workpiece.lo_y;
			particles[part_iter].X = particles[part_iter].x;
			particles[part_iter].Y = particles[part_iter].y;
			particles[part_iter].refine_step = 0;
			part_iter++;
		}
	}

	unsigned int n = part_iter;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = T0;
		particles[i].T_init = T0;
		particles[i].h = (particles[i].refine_step != 0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step != 0) ? dVh * rho0 : dVl * rho0;
		particles[i].split = false;
		particles[i].merge = false;

		particles[i].fixed = (particles[i].y < cfg.workpiece.lo_y + 0.5 * dxl);
		particles[i].fixed = particles[i].fixed || (particles[i].x > cfg.workpiece.hi_x - 0.5 * dxh);
	}

	const correction_constants cs = build_corrections(cfg, g.dx);
	simulation_data sim_data(pc, cs);
	body *b = new body(particles, n, sim_data);

	plasticity *plast = build_plasticity_from_config(cfg, pc);
	thermal *trml = build_thermal_from_config(cfg, pc);
	tool *t = build_tool_from_config(cfg, cfg.workpiece.lo_x, cfg.workpiece.hi_y, g.dx, cfg.tool.cutting_speed);

	if (plast)
		b->set_plasticity(plast);
	if (trml)
		b->set_thermal(trml);
	b->set_tool(t);

	set_default_logger(t);
	return b;
}

static adaptivity *build_adaptivity_from_config(const simulation_config &cfg, double lx, double l_eff) {
	if (!cfg.adaptivity.enabled)
		return nullptr;
	glm::dvec2 xy_min(cfg.adaptivity.xy_min_x, cfg.adaptivity.xy_min_y);
	glm::dvec2 xy_max(cfg.adaptivity.xy_max_x, cfg.adaptivity.xy_max_y);
	adaptivity *adapt =
		new adaptivity(cfg.adaptivity.alpha_dx, cfg.adaptivity.beta_h, cfg.adaptivity.v_cr, cfg.adaptivity.div_v_cr, cfg.adaptivity.SvM_cr,
					   cfg.adaptivity.eps_cr, cfg.adaptivity.T_cr, xy_min, xy_max, cfg.adaptivity.frame_width, cfg.adaptivity.frame_height,
					   (unsigned int)cfg.adaptivity.n_nbh, l_eff, cfg.adaptivity.allow_refine);
	adapt->set_refine_criterion(adaptivity::refine_criteria::moving_frame);
	adapt->set_refine_pattern(adaptivity::pattern::cubic_basic);
	return adapt;
}

static body *build_dynamic_refinement(const simulation_config &cfg) {
	const physical_constants pc = select_physical_constants(cfg);
	const double rho0 = pc.rho0();
	const double T0 = cfg.thermal.T0;

	const double lx = cfg.workpiece.hi_x - cfg.workpiece.lo_x;
	const double ly = cfg.workpiece.hi_y - cfg.workpiece.lo_y;

	const grid_dims g = compute_grid(cfg);
	set_time_from_config(cfg, g.dx, cfg.tool.cutting_speed, pc);

	const double resol_ratio = cfg.multires.resol_ratio;
	const double py_split = cfg.multires.py_split_fraction * ly + cfg.workpiece.lo_y;
	const double dxh = g.dx;
	const double dxl = dxh * resol_ratio;
	const unsigned int nxh = g.nx;
	const unsigned int nyh = g.ny;
	const unsigned int nxl = (unsigned int)(std::floor(lx / dxl) + 1.0);
	const unsigned int nyl = (unsigned int)(std::floor(ly / dxl) + 1.0);
	const double dVl = dxl * dxl;
	const double dVh = dxh * dxh;
	const double h0l = cfg.numerical.hdx * dxl;
	const double h0h = cfg.numerical.hdx * dxh;

	particle *particles = new particle[nxh * nyh];
	std::srand(0);
	unsigned int part_iter = 0;

	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i * dxh;
			double pyh = j * dxh;
			if ((pyh + cfg.workpiece.lo_y) < (py_split - cfg.multires.py_margin_factor_dynamic * dxh) ||
				pxh > cfg.multires.x_high_res_limit)
				continue;
			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + cfg.workpiece.lo_x;
			particles[part_iter].y = pyh + cfg.workpiece.lo_y;
			particles[part_iter].X = particles[part_iter].x;
			particles[part_iter].Y = particles[part_iter].y;
			particles[part_iter].refine_step = 1;
			part_iter++;
		}
	}

	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i * dxl;
			double pyl = j * dxl;
			if ((pyl + cfg.workpiece.lo_y) >= (py_split - cfg.multires.py_margin_factor_dynamic * dxh) &&
				pxl <= cfg.multires.x_high_res_limit)
				continue;
			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + cfg.workpiece.lo_x;
			particles[part_iter].y = pyl + cfg.workpiece.lo_y;
			particles[part_iter].X = particles[part_iter].x;
			particles[part_iter].Y = particles[part_iter].y;
			particles[part_iter].refine_step = 0;
			part_iter++;
		}
	}

	unsigned int n = part_iter;
	for (unsigned int i = n; i < nxh * nyh; i++) {
		particles[i] = particle(i);
	}

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = T0;
		particles[i].T_init = T0;
		particles[i].h = (particles[i].refine_step != 0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step != 0) ? dVh * rho0 : dVl * rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].fixed = (particles[i].y < cfg.workpiece.lo_y + 0.5 * dxl);
		particles[i].fixed = particles[i].fixed || (particles[i].x > cfg.workpiece.hi_x - 0.5 * dxh);
	}

	const correction_constants cs = build_corrections(cfg, g.dx);
	simulation_data sim_data(pc, cs);
	body *b = new body(particles, n, sim_data);

	plasticity *plast = build_plasticity_from_config(cfg, pc);
	thermal *trml = build_thermal_from_config(cfg, pc);
	tool *t = build_tool_from_config(cfg, cfg.workpiece.lo_x, cfg.workpiece.hi_y, g.dx, cfg.tool.cutting_speed);
	t->set_edge_coord(glm::dvec2(0.0, cfg.workpiece.hi_y - cfg.tool.target_feed));

	const double l_eff = cfg.time.cut_length + cfg.adaptivity.l_eff_extra_fraction * lx;
	adaptivity *adapt = build_adaptivity_from_config(cfg, lx, l_eff);

	if (plast)
		b->set_plasticity(plast);
	if (trml)
		b->set_thermal(trml);
	b->set_tool(t);
	if (adapt)
		b->set_adaptivity(adapt);

	set_default_logger(t);
	return b;
}

body *build_body_from_config(const simulation_config &cfg) {
	if (cfg.model.type == "single_resolution")
		return build_single_resolution(cfg);
	if (cfg.model.type == "apriori_refinement")
		return build_apriori_refinement(cfg);
	if (cfg.model.type == "dynamic_refinement")
		return build_dynamic_refinement(cfg);
	throw std::runtime_error("Unsupported model.type: " + cfg.model.type);
}

} // namespace mfree::config
