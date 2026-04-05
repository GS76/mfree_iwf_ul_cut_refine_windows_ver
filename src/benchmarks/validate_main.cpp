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

#include "../fe_tool.h"
#include "../contact.h"
#include "../simulation_time.h"

#include "../benchmarks/material_library.h"

#include <cmath>
#include <cstdio>
#include <vector>

static fe_tool make_rect_tool_mesh(double L, double H, unsigned int nx, unsigned int ny, int tag_left, int tag_right, int tag_other) {
	std::vector<glm::dvec2> nodes;
	nodes.reserve(nx * ny);
	for (unsigned int j = 0; j < ny; j++) {
		double y = H * (static_cast<double>(j) / static_cast<double>(ny - 1));
		for (unsigned int i = 0; i < nx; i++) {
			double x = L * (static_cast<double>(i) / static_cast<double>(nx - 1));
			nodes.push_back(glm::dvec2(x, y));
		}
	}

	auto idx = [&](unsigned int i, unsigned int j) { return j * nx + i; };

	std::vector<std::array<unsigned int, 3>> tris;
	tris.reserve(2 * (nx - 1) * (ny - 1));
	for (unsigned int j = 0; j < ny - 1; j++) {
		for (unsigned int i = 0; i < nx - 1; i++) {
			unsigned int n00 = idx(i, j);
			unsigned int n10 = idx(i + 1, j);
			unsigned int n01 = idx(i, j + 1);
			unsigned int n11 = idx(i + 1, j + 1);
			tris.push_back({n00, n10, n11});
			tris.push_back({n00, n11, n01});
		}
	}

	std::vector<fe_tool::boundary_edge> bnd;
	for (unsigned int j = 0; j < ny - 1; j++) {
		fe_tool::boundary_edge e;
		e.n0 = idx(0, j);
		e.n1 = idx(0, j + 1);
		e.physical_tag = tag_left;
		bnd.push_back(e);
	}
	for (unsigned int j = 0; j < ny - 1; j++) {
		fe_tool::boundary_edge e;
		e.n0 = idx(nx - 1, j);
		e.n1 = idx(nx - 1, j + 1);
		e.physical_tag = tag_right;
		bnd.push_back(e);
	}
	for (unsigned int i = 0; i < nx - 1; i++) {
		fe_tool::boundary_edge e0;
		e0.n0 = idx(i, 0);
		e0.n1 = idx(i + 1, 0);
		e0.physical_tag = tag_other;
		bnd.push_back(e0);

		fe_tool::boundary_edge e1;
		e1.n0 = idx(i, ny - 1);
		e1.n1 = idx(i + 1, ny - 1);
		e1.physical_tag = tag_other;
		bnd.push_back(e1);
	}

	fe_tool ft;
	ft.set_mesh(nodes, tris, bnd);
	return ft;
}

static double analytic_dirichlet_neumann(double x, double t, double L, double alpha, double Ts) {
	double sum = 0.;
	for (int n = 0; n < 200; n++) {
		double lam = (2.0 * n + 1.0) * M_PI / (2.0 * L);
		double term = (4.0 / ((2.0 * n + 1.0) * M_PI)) * std::sin(lam * x) * std::exp(-alpha * lam * lam * t);
		sum += term;
	}
	return Ts * (1.0 - sum);
}

static bool test_tool_1d_conduction() {
	const double L = 0.01;
	const double H = 0.001;
	fe_tool ft = make_rect_tool_mesh(L, H, 101, 5, 1, 2, 3);

	fe_tool::thermal_material mat;
	mat.rho = 7800.0;
	mat.cp = 500.0;
	mat.k = 45.0;
	ft.set_material(mat);
	ft.set_pose(glm::dvec2(0.), glm::dvec2(0.));
	ft.set_initial_temperature(0.0);

	fe_tool::convection_bc left;
	left.h = 0.0;
	left.T_inf = 100.0;
	ft.set_dirichlet_on_physical(1, left.T_inf);

	const double alpha = mat.k / (mat.rho * mat.cp);
	const double t_final = 0.002;
	const double dt = 1.0e-7;
	unsigned int nstep = static_cast<unsigned int>(t_final / dt);
	for (unsigned int s = 0; s < nstep; s++) ft.advance_explicit(dt);

	unsigned int best = 0;
	double best_d2 = 1e300;
	glm::dvec2 target(0.005, 0.0005);
	const auto &nodes = ft.nodes_tool_frame();
	for (unsigned int i = 0; i < nodes.size(); i++) {
		glm::dvec2 d = nodes[i] - target;
		double d2 = glm::dot(d, d);
		if (d2 < best_d2) {
			best_d2 = d2;
			best = i;
		}
	}

	double T_num = ft.temperature_at_node(best);
	double T_ref = analytic_dirichlet_neumann(nodes[best].x, t_final, L, alpha, left.T_inf);
	double rel = std::abs(T_num - T_ref) / std::max(1e-12, std::abs(T_ref));
	std::printf("tool_1d rel=%e T_num=%g T_ref=%g\n", rel, T_num, T_ref);
	return rel <= 0.05;
}

static bool test_frictional_heating_partition() {
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	correction_constants cs(constants_monaghan(0.0, 4, 0.3), constants_artificial_viscosity(1.0, 1.0, 0.1), 0.5);
	simulation_data sim_data(pc, cs);

	particle p(0);
	p.x = 0.99;
	p.y = 0.5;
	p.vx = 0.0;
	p.vy = 10.0;
	p.rho = pc.rho0();
	p.m = 1.0e-6;
	p.T = 300.0;

	body b(&p, 1, sim_data);
	particle &pp = b.get_particles()[0];

	tool t(glm::dvec2(0., 1.), glm::dvec2(1., 1.), glm::dvec2(1., 0.), glm::dvec2(0., 0.), 0.5);
	t.set_vel(glm::dvec2(0.));
	b.set_tool(&t);

	fe_tool ft = make_rect_tool_mesh(1.0, 1.0, 3, 3, 1, 2, 3);
	fe_tool::thermal_material mat;
	mat.rho = 7800.0;
	mat.cp = 500.0;
	mat.k = 1.0e6;
	ft.set_material(mat);
	ft.set_pose(glm::dvec2(0.), glm::dvec2(0.));
	ft.set_initial_temperature(p.T);
	b.set_fe_tool(&ft);

	simulation_time *time = &simulation_time::getInstance();
	time->set_dt(1.0e-3);
	time->set_t_final(1.0e-3);

	pp.T_t = 0.;
	b.apply_contact();
	std::printf("inside=%g\n", t.inside(glm::dvec2(pp.x, pp.y)));

	glm::dvec2 F_t(pp.ftx, pp.fty);
	glm::dvec2 F_n(pp.fcx, pp.fcy);
	double Fn = glm::length(F_n);
	if (Fn <= 0.) {
		std::printf("friction Fn=%g\n", Fn);
		return false;
	}

	glm::dvec2 n = glm::normalize(F_n);
	glm::dvec2 v_rel(pp.vx, pp.vy);
	glm::dvec2 vt = v_rel - glm::dot(v_rel, n) * n;
	double slip = glm::length(vt);
	double P_fric = glm::length(F_t) * slip;

	double dE_p = pp.m * pc.tc().cp() * (time->get_dt() * pp.T_t);
	double ratio = dE_p / (0.8 * P_fric * time->get_dt());
	std::printf("friction ratio=%g P_fric=%g dE_p=%g\n", ratio, P_fric, dE_p);
	return std::abs(ratio - 1.0) <= 0.1;
}

static bool test_convection_lumped() {
	const double L = 0.01;
	const double H = 0.01;
	fe_tool ft = make_rect_tool_mesh(L, H, 11, 11, 1, 2, 3);

	fe_tool::thermal_material mat;
	mat.rho = 7800.0;
	mat.cp = 500.0;
	mat.k = 0.0;
	ft.set_material(mat);
	ft.set_pose(glm::dvec2(0.), glm::dvec2(0.));

	double T0 = 400.0;
	ft.set_initial_temperature(T0);

	fe_tool::convection_bc air;
	air.h = 20.0;
	air.T_inf = 298.15;
	ft.set_convection_air_all_exposed(air);

	double V = L * H;
	double A = 2.0 * (L + H);
	double tau = (mat.rho * mat.cp * V) / (air.h * A);

	double t_final = 0.05;
	double dt = 1.0e-4;
	unsigned int nstep = static_cast<unsigned int>(t_final / dt);
	for (unsigned int s = 0; s < nstep; s++) ft.advance_explicit(dt);

	double T_ref = air.T_inf + (T0 - air.T_inf) * std::exp(-t_final / tau);
	double T_avg = 0.;
	for (unsigned int i = 0; i < ft.nodes_tool_frame().size(); i++) T_avg += ft.temperature_at_node(i);
	T_avg /= static_cast<double>(ft.nodes_tool_frame().size());

	double rel = std::abs(T_avg - T_ref) / std::max(1e-12, std::abs(T_ref));
	std::printf("convection rel=%e T_avg=%g T_ref=%g\n", rel, T_avg, T_ref);
	return rel <= 0.05;
}

int main() {
	bool ok = true;
	bool ok1 = test_tool_1d_conduction();
	bool ok2 = test_frictional_heating_partition();
	bool ok3 = test_convection_lumped();
	std::printf("tool_1d_conduction %s\n", ok1 ? "ok" : "fail");
	std::printf("friction_partition %s\n", ok2 ? "ok" : "fail");
	std::printf("convection_lumped %s\n", ok3 ? "ok" : "fail");
	ok = ok1 && ok2 && ok3;

	if (!ok) {
		std::printf("validation_failed\n");
		return 1;
	}

	std::printf("validation_ok\n");
	return 0;
}
