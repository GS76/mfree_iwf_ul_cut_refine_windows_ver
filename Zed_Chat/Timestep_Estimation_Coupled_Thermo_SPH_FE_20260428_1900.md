## User (checkpoint)

[@mfree_iwf_ul_cut_refine_windows_ver_FEA_Tool.code-workspace](file:///D:/mfree_iwf_ul_cut_refine_windows_ver/mfree_iwf_ul_cut_refine_windows_ver_FEA_Tool.code-workspace) [@Guidance for mfree_iwf_ul_cut_refine_windows_ver](zed:///agent/thread/95da402b-cb2a-4848-a773-240aedf59e45?name=Guidance+for+mfree_iwf_ul_cut_refine_windows_ver) Determine and create a method whereby the maximum time step size can be estimated considering the coupled thermal-structural simulation as well as the different material's properties for the FE tool as well as the SPH workpiece.

## Assistant

<thinking>
**Implementing max timestep**

I'm looking to implement the method for max timestep while considering coupled thermal-structural factors and materials. This sounds technical, and I need to make sure I inspect everything thoroughly. It's important to find the right files that relate to this process. I'll need to focus on those elements to ensure a smooth implementation. It’s a bit complex, but I’m committed to getting it right!
</thinking>

**Tool Call: List the `mfree_iwf_ul_cut_refine_windows_ver` directory's contents**
Status: Completed

# Folders:
mfree_iwf_ul_cut_refine_windows_ver\.cline
mfree_iwf_ul_cut_refine_windows_ver\.github
mfree_iwf_ul_cut_refine_windows_ver\.vscode
mfree_iwf_ul_cut_refine_windows_ver\Bugs
mfree_iwf_ul_cut_refine_windows_ver\Cline_Chat
mfree_iwf_ul_cut_refine_windows_ver\Debug
mfree_iwf_ul_cut_refine_windows_ver\Meshing
mfree_iwf_ul_cut_refine_windows_ver\Original_Paper
mfree_iwf_ul_cut_refine_windows_ver\References
mfree_iwf_ul_cut_refine_windows_ver\Release
mfree_iwf_ul_cut_refine_windows_ver\Tangent_Feed
mfree_iwf_ul_cut_refine_windows_ver\build
mfree_iwf_ul_cut_refine_windows_ver\docs
mfree_iwf_ul_cut_refine_windows_ver\img
mfree_iwf_ul_cut_refine_windows_ver\results
mfree_iwf_ul_cut_refine_windows_ver\results_run1
mfree_iwf_ul_cut_refine_windows_ver\scripts
mfree_iwf_ul_cut_refine_windows_ver\snapshots
mfree_iwf_ul_cut_refine_windows_ver\src

# Files:
mfree_iwf_ul_cut_refine_windows_ver\.clang-format
mfree_iwf_ul_cut_refine_windows_ver\.editorconfig
mfree_iwf_ul_cut_refine_windows_ver\.gitattributes
mfree_iwf_ul_cut_refine_windows_ver\.gitignore
mfree_iwf_ul_cut_refine_windows_ver\AGENTS.md
mfree_iwf_ul_cut_refine_windows_ver\CMakeLists.txt
mfree_iwf_ul_cut_refine_windows_ver\CONTRIBUTING.md
mfree_iwf_ul_cut_refine_windows_ver\CodeCitations.md
mfree_iwf_ul_cut_refine_windows_ver\LICENSE
mfree_iwf_ul_cut_refine_windows_ver\README.md
mfree_iwf_ul_cut_refine_windows_ver\mfree_iwf_ul_cut_refine_windows_ver_FEA_Tool.code-workspace


## Assistant

<thinking>
**Implementing and inspecting**

I realize it's crucial to implement the changes. I should definitely inspect the source code to understand what's going on. Maybe I can use a tool like grep to search for specific terms, particularly related to "timestep." That way, I can pinpoint where adjustments are needed. This could streamline the process and lead to better outcomes. It feels like a good plan to move forward with!
</thinking>

**Tool Call: Search files for regex `time.?step|dt|timestep|CFL|courant`**
Status: Completed

Showing matches 1-20 (there were more matches found; use offset: 20 to see next page):

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\adaptivity.cpp

### int adaptivity::scan_mark_moving_frame( ) › double width_adapt › L421-429
```
	double ytool = t*vc_y + y0;

	double width_adapt = m_width;
	double nudge = 1e-5;

	glm::dvec2 ylim(ytool - m_height + nudge, +DBL_MAX);
	glm::dvec2 xlim(-DBL_MAX, xtool + width_adapt + nudge);
	//======================================================

```

### adaptivity::adaptivity( ) › L1028-1038
```
adaptivity::adaptivity(double alpha_dx, double beta_h, double v, double div_v, double SvM, double epsPl,
			   double T, glm::dvec2 xy_min, glm::dvec2 xy_max,
			   double frm_width, double frm_height, unsigned int num_nbh, double l_eff, bool eccentric) {

	m_alpha = alpha_dx;
	m_beta = beta_h;
	m_v_threshold = v;
	m_div_v_threshold = div_v;
	m_SvM_threshold = SvM;
	m_eps_threshold = epsPl;
	m_T_threshold = T;
```

8 lines remaining in ancestor node. Read the file to see all.

### adaptivity::adaptivity( ) › L1039-1043
```
	m_xy_min = xy_min;
	m_xy_max = xy_max;
	m_width  = frm_width;
	m_height = frm_height;
	m_num_nbh_threshold = num_nbh;
```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\adaptivity.h

### class adaptivity › adaptivity( ) › L98-100
```
	adaptivity(double alpha_dx, double beta_h, double v, double div_v, double SvM, double epsPl,
			   double T, glm::dvec2 xy_min, glm::dvec2 xy_max, double frm_width, double frm_height,
			   unsigned int num_nbh, double l_eff, bool eccentric);
```

### class adaptivity › double m_width › L114-119
```
	glm::dvec2 m_xy_min = {0., 0.};      // x_min & y_min of the refinement zone
	glm::dvec2 m_xy_max = {0., 0.};      // x_max & y_max of the refinement zone
	double m_width  = 0.;                // width of the moving refinement frame
	double m_height = 0.;                // width of the moving refinement frame
	unsigned int m_num_nbh_threshold = 0;// min number of neighboring particles
	double m_l_eff = 0.;                 // do not refine after this length
```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_benches.cpp

### body *test_bench_setup_rings( ) › double dt › L84-90
```

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

	printf("using timestep %e\n", dt);

	particle *particles = new particle[nbox*nbox];
```

### body *test_bench_setup_rings( ) › L136-141
```

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(6e3*dt);
	time->set_dt(dt);

	global_logger = new logger("rings");
```

### body *test_bench_setup_ring_contact( ) › double dt › L163-169
```

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

	printf("using timestep %e\n", dt);

	particle *particles = new particle[nbox*nbox];
```

### body *test_bench_setup_ring_contact( ) › L211-216
```

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(6e3*dt);
	time->set_dt(dt);

	glm::dvec2 bl(0.,    -2*ro);
```

### body *test_bench_setup_disk_impact( ) › double dt › L241-247
```

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

	printf("using timestep %e\n", dt);

	particle *particles = new particle[nbox*nbox];
```

### body *test_bench_setup_disk_impact( ) › L291-296
```

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(6e3*dt);
	time->set_dt(dt);

	glm::dvec2 bl(0.,    -0.5*ro);
```

### body *test_bench_setup_thermal( ) › double dt › L318-324
```
	double hdx = 1.7;

	double dt = 1e-6;

	printf("using timestep %e\n", dt);

	particle *particles = new particle[nbox*nbox];
```

### body *test_bench_setup_thermal( ) › L372-377
```

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(1e3*dt);
	time->set_dt(dt);

	thermal *trml = new thermal(sim_data.get_physical_constants());
```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp

### static void apply_mech_fix_tags_from_env( ) › double width › L248-255
```

		std::vector<unsigned int> fixed;
		double width = x_max - x_min;
		double tol = 0.01 * width;
		try_read_env_double("MFREE_FE_TOOL_FIX_X_TOL", tol);
		if (!std::isfinite(tol) || tol <= 0.) tol = 0.01 * width;

		for (int attempt = 0; attempt < 4; attempt++) {
```

### body *cutting_ref_mr( ) › double dt › L592-596
```

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut

```

### body *cutting_ref_mr( ) › L597-603
```
	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx*ny);

	particle *particles = new particle[nx*ny];
```

### body *cutting_ref_single_resol( ) › double dt_empirical › L752-765
```
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nx*ny];
```

### body *cutting_ref_single_resol( ) › L911-915
```
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, nx*ny);
	printf("<<< single-resolution simulation >>>\n");

```

### body *cutting_ref_multi_resol_apriori( ) › double dt_empirical › L997-1010
```
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];
```

### body *cutting_ref_multi_resol_apriori( ) › L1205-1209
```
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

```


**Tool Call: 2 matches**
Status: Completed

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\thermal.cpp

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\thermal.h

**Tool Call: 4 matches**
Status: Completed

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\material_library.cpp

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\material_library.h

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\material.cpp

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\material.h

## Assistant

<thinking>
**Inspecting files for properties**

I need to take action and inspect the files properly. I should avoid using code blocks unless specifying a path. It might be necessary to implement a method along with proper documentation and tests. Also, I want to look into finding the front-end material properties. There’s a lot to tackle here, but I’ll make sure to handle each part step by step to ensure everything is covered correctly!
</thinking>

**Tool Call: Search files for regex `thermal_diffusivity|conductivity|heat_capacity|young|density|rho|c0\(|set_dt|max.*dt|stable`**
Status: Completed

Showing matches 1-20 (there were more matches found; use offset: 20 to see next page):

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\adaptivity.cpp

### void copy_dad_to_son( ) › L99-105
```
void copy_dad_to_son(const particle &dad, particle &son) {
	son.rho     = dad.rho;
	son.T       = dad.T;
	son.T_init  = dad.T_init;
	son.vx      = dad.vx;
	son.vy      = dad.vy;
}
```

### void adaptivity::dens_before_approx_N2( ) › double rho0_i › L113-129
```
	for (unsigned int i = 0; i < b.get_num_part(); i++) {

		double xi = particles[i].x;
		double yi = particles[i].y;
		double rho0_i = 0.;

		for (unsigned int j = 0; j < b.get_num_part(); j++) {
			if(particles[j].refine_step!=0 && !particles[j].split) continue;

			double xj = particles[j].x;
			double yj = particles[j].y;
			double hj = (particles[j].split) ? (1./m_beta)*particles[j].h : particles[j].h;
			double mj = (particles[j].split) ? m_num_child*particles[j].m : particles[j].m;

			kernel_result w = cubic_spline(xi, yi, xj, yj, hj);

			rho0_i += mj*w.w;
```

6 lines remaining in ancestor node. Read the file to see all.

### void adaptivity::dens_before_approx_N2( ) › L132-136
```
		glm::dvec2 pos(xi,yi);

		particles[i].rho_init = (!inside_bounding_box(xlim,ylim,pos)) ? rho0_i : 1.0;
	}
}
```

### void adaptivity::dens_after_approx_N2( ) › double rho_i › L141-160
```
	for (unsigned int i = 0; i < b.get_num_part(); i++) {

		double xi = particles[i].x;
		double yi = particles[i].y;
		double rho_i = 0.;

		for (unsigned int j = 0; j < b.get_num_part(); j++) {

			double xj = particles[j].x;
			double yj = particles[j].y;
			double hj = particles[j].h;
			double mj = particles[j].m;

			kernel_result w  = cubic_spline(xi, yi, xj, yj, hj);

			rho_i += mj*w.w;
		}
		particles[i].rho = rho_i;
	}
}
```

### int adaptivity::scan_mark_moving_frame( ) › glm::dvec2 xlim › L425-429
```

	glm::dvec2 ylim(ytool - m_height + nudge, +DBL_MAX);
	glm::dvec2 xlim(-DBL_MAX, xtool + width_adapt + nudge);
	//======================================================

```

### void adaptivity::perform_split_triangular( ) › double dx › L492-502
```
		if(particles[i].split && particles[i].refine_step<MAX_REFINE_STEP) {

			// 0. call your DAD
			double dx = sqrt(particles[i].m/particles[i].rho);
			double x_DAD = particles[i].x;
			double y_DAD = particles[i].y;
			double h_DAD = particles[i].h;
			double m_DAD = particles[i].m;

			// 1. increase the refinement step of "DAD"
			particles[i].last_refine_at = step;
```

26 lines remaining in ancestor node. Read the file to see all.

### void adaptivity::perform_split_cubic_basic( ) › double dx › L558-568
```
		if(particles[i].split && particles[i].refine_step<MAX_REFINE_STEP) {
			std::array<double, max_SON2D> x_SON{};
			std::array<double, max_SON2D> y_SON{};
			std::array<double, max_SON2D> h_SON{};
			std::array<double, max_SON2D> m_SON{};

			// 0. call your DAD
			double dx = sqrt(particles[i].m/particles[i].rho);
			double x_DAD = particles[i].X; // or x if ~regular arrangement
			double y_DAD = particles[i].Y; // or y if ~regular arrangement
			double h_DAD = particles[i].h;
```

92 lines remaining in ancestor node. Read the file to see all.

### void adaptivity::perform_split_cubic( ) › double dx › L692-702
```
		if(particles[i].split && particles[i].refine_step < MAX_REFINE_STEP) {
			std::array<double, max_SON2D> x_SON{};
			std::array<double, max_SON2D> y_SON{};
			std::array<double, max_SON2D> h_SON{};
			std::array<double, max_SON2D> m_SON{};

			// 0. call your DAD
			double dx = sqrt(particles[i].m/particles[i].rho);
			double x_DAD = particles[i].x;
			double y_DAD = particles[i].y;
			double h_DAD = particles[i].h;
```

74 lines remaining in ancestor node. Read the file to see all.

### void adaptivity::perform_split_hexagonal( ) › L782-792
```
void adaptivity::perform_split_hexagonal(body &b)  const {
	simulation_time *time = &simulation_time::getInstance();
	unsigned int step = time->get_step();

	std::vector<particle> &particles = b.get_particles();

	// how many "SON" do you have in 2D? ---> 6
	const unsigned int num_SON2D = m_num_child-1;
	assert(num_SON2D <= max_SON2D);

	// given by J. Feldman & J. Bonet after minimizing the global density error [2006] with (alpha=60%,beta=60%)
```

122 lines remaining in ancestor node. Read the file to see all.

### void adaptivity::perform_split_hexagonal( ) › double dx › L811-821
```
		if(particles[i].split && particles[i].refine_step<MAX_REFINE_STEP) {

			std::vector<double> x_SON(num_SON2D);
			std::vector<double> y_SON(num_SON2D);
			std::vector<double> h_SON(num_SON2D);
			std::vector<double> m_SON(num_SON2D);

			// 0. call your DAD
			double dx = sqrt(particles[i].m/particles[i].rho);
			double x_DAD = particles[i].x;
			double y_DAD = particles[i].y;
```

89 lines remaining in ancestor node. Read the file to see all.

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\adaptivity.h

### class adaptivity › adaptivity( ) › L98-100
```
	adaptivity(double alpha_dx, double beta_h, double v, double div_v, double SvM, double epsPl,
			   double T, glm::dvec2 xy_min, glm::dvec2 xy_max, double frm_width, double frm_height,
			   unsigned int num_nbh, double l_eff, bool eccentric);
```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\material_library.cpp

### physical_constants matlib_steel4430() › double rho0 › L53-63
```
physical_constants matlib_steel4430() {	// correct name: Steel 4340
	double E    = 200e9;
	double nu   = 0.29;
	double rho0 = 7830.0;

	double JC_A		= 792.0e6;
	double JC_B		= 510.0e6;
	double JC_C		= 0.014;
	double JC_m		= 1.03;
	double JC_n		= 0.26;
	double Tref		= 273.0;
```

11 lines remaining in ancestor node. Read the file to see all.

### physical_constants matlib_steel4430() › L71-75
```
	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}

```

### physical_constants matlib_ARMCO_iron() › double rho0 › L76-86
```
physical_constants matlib_ARMCO_iron() {
	double E = 207e9;
	double nu = 0.29;
	double rho0 = 7890.0;

	double JC_A		= 175.0e6;
	double JC_B		= 380.0e6;
	double JC_C		= 0.06;
	double JC_m		= 0.55;
	double JC_n		= 0.32;
	double Tref		= 273.0;
```

10 lines remaining in ancestor node. Read the file to see all.

### physical_constants matlib_ARMCO_iron() › L93-97
```
	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq);
	return physical_constants(nu, E, rho0, jc, tc);
}

```

### physical_constants matlib_OFHC_copper() › double rho0 › L98-108
```
physical_constants matlib_OFHC_copper() {
	double E = 124e9;
	double nu = 0.34;
	double rho0 = 8960.0;

	double JC_A		= 90.0e6;
	double JC_B		= 292.0e6;
	double JC_C		= 0.025;
	double JC_m		= 1.09;
	double JC_n		= 0.31;
	double Tref		= 273.0;
```

10 lines remaining in ancestor node. Read the file to see all.

### physical_constants matlib_OFHC_copper() › L115-119
```
	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq);
	return physical_constants(nu, E, rho0, jc, tc);
}

```

### physical_constants matlib_AISI1045() › double rho0 › L120-130
```
physical_constants matlib_AISI1045() {
	double E = 205e9;
	double nu = 0.29;
	double rho0 = 7850.0;

	//	Johnson Cook parameters
	double JC_A		= 615.8e6;
	double JC_B		= 667.7e6;
	double JC_C		= 0.0134;
	double JC_m		= 1.078;
	double JC_n		= 0.255;
```

11 lines remaining in ancestor node. Read the file to see all.

### physical_constants matlib_AISI1045() › L138-142
```
	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq);
	return physical_constants(nu, E, rho0, jc, tc);
}

```

### physical_constants matlib_rubber() › double rho0 › L143-149
```
physical_constants matlib_rubber() {
	double E = 1e7;
	double nu = 0.4;
	double rho0 = 1;

	return physical_constants(nu, E, rho0, johnson_cook_constants());
}
```


**Tool Call: 2 matches**
Status: Completed

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\fe_tool.cpp

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\fe_tool.h

**Tool Call: 4 matches**
Status: Completed

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\simulation_data.cpp

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\simulation_data.h

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\simulation_time.cpp

file://D:\mfree_iwf_ul_cut_refine_windows_ver\src\simulation_time.h

## Assistant

<thinking>
**Looking into material inspection**

I need to inspect the material using the fe_tool and run some tests on the cuttings. Additionally, it seems like I might want to implement new timestep estimator files. I need to find the structure to make sure everything is set up correctly. I should remember to read any relevant documentation and guidance to help me through it. This seems like a critical step to get things moving smoothly!
</thinking>

**Tool Call: Read file `src\material.h` (lines 1-220)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/material.h
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

#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <math.h>
#include <stdio.h>
#include <type_traits>
#include <glm/glm.hpp>

#include "kernel.h"
#include "particle.h"
#include "body.h"

/*
 This header includes 2 functions:
 ---------------------------------
 	 1- For "hydrostatic" part of stress: equation of state  --> Eq. (13) in Section 2.2.3 of the paper
 	 2- For "deviatoric" part of stress: jaumann stress rate --> Eq. (14) in Section 2.2.3 of the paper
*/

void material_eos(body &b);
void material_stress_rate_jaumann(body &b);

#endif /* MATERIAL_H_ */

```


**Tool Call: Read file `src\fe_tool.h` (lines 1-260)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.h
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

#ifndef FE_TOOL_H_
#define FE_TOOL_H_

#include "glm/glm.hpp"

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>

class fe_tool {
public:
	struct bbox {
		double bbmin_x = 0.;
		double bbmax_x = 0.;
		double bbmin_y = 0.;
		double bbmax_y = 0.;

		bool in(glm::dvec2 qp);
		bool valid() const;

		bbox();
		bbox(glm::dvec2 p1, glm::dvec2 p2);
		bbox(double bbmin_x, double bbmax_x, double bbmin_y, double bbmax_y);
	};

	struct thermal_material {
		double rho = 0.;
		double cp = 0.;
		double k = 0.;
	};

	struct mechanical_material {
		double E = 0.;     // Pa - Young's modulus
		double nu = 0.;    // - - Poisson's ratio
		double alpha = 0.; // 1/K - thermal expansion coefficient
	};

	struct boundary_edge {
		unsigned int n0 = 0;
		unsigned int n1 = 0;
		int physical_tag = 0;
	};

	struct convection_bc {
		double h = 0.;
		double T_inf = 0.;
	};

	bool load_gmsh_msh2(const std::string &path);
	void set_mesh(const std::vector<glm::dvec2> &nodes_tool_frame,
	              const std::vector<std::array<unsigned int, 3>> &triangles,
	              const std::vector<boundary_edge> &boundary_edges);

	void set_material(thermal_material mat);
	thermal_material get_material() const;
	void set_material_table_rho(std::vector<double> T, std::vector<double> rho);
	void set_material_table_cp(std::vector<double> T, std::vector<double> cp);
	void set_material_table_k(std::vector<double> T, std::vector<double> k);

	void set_mechanical_material(mechanical_material mat);
	mechanical_material get_mechanical_material() const;
	void set_mechanical_table_E(std::vector<double> T, std::vector<double> E);
	void set_mechanical_table_nu(std::vector<double> T, std::vector<double> nu);
	void set_mechanical_table_alpha(std::vector<double> T, std::vector<double> alpha);
	void set_reference_temperature(double T_ref);
	double reference_temperature() const;
	void set_mechanics_fixed_on_physical(int physical_tag);
	/**
	 * @brief Constrain the X displacement DOF (UX) of all boundary nodes that belong to a given physical tag.
	 * @param physical_tag Gmsh physical tag of boundary line elements.
	 */
	void set_mechanics_fixed_x_on_physical(int physical_tag);
	/**
	 * @brief Constrain the Y displacement DOF (UY) of all boundary nodes that belong to a given physical tag.
	 * @param physical_tag Gmsh physical tag of boundary line elements.
	 */
	void set_mechanics_fixed_y_on_physical(int physical_tag);
	void clear_mechanics_fixed();
	void set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes);
	/**
	 * @brief Constrain the X displacement DOF (UX) for an explicit list of node indices (0-based).
	 * @param nodes Node indices in the tool mesh (0-based).
	 */
	void set_mechanics_fixed_x_nodes(const std::vector<unsigned int> &nodes);
	/**
	 * @brief Constrain the Y displacement DOF (UY) for an explicit list of node indices (0-based).
	 * @param nodes Node indices in the tool mesh (0-based).
	 */
	void set_mechanics_fixed_y_nodes(const std::vector<unsigned int> &nodes);
	void clear_mechanics_fixed_nodes();

	/**
	 * @brief Query whether a node has its X displacement DOF (UX) constrained by the current mechanics constraints.
	 * @param node Node index in the tool mesh (0-based).
	 * @return true if UX is fixed, otherwise false.
	 */
	bool is_mechanics_fixed_x(unsigned int node) const;
	/**
	 * @brief Query whether a node has its Y displacement DOF (UY) constrained by the current mechanics constraints.
	 * @param node Node index in the tool mesh (0-based).
	 * @return true if UY is fixed, otherwise false.
	 */
	bool is_mechanics_fixed_y(unsigned int node) const;

	void set_initial_temperature(double T0);

	void set_pose(glm::dvec2 pos, glm::dvec2 vel);
	glm::dvec2 get_pos() const;
	glm::dvec2 get_vel() const;
	void update_pose(double dt);

	glm::dvec2 to_tool_frame(glm::dvec2 x_world) const;
	glm::dvec2 to_world_frame(glm::dvec2 x_tool) const;

	const std::vector<glm::dvec2> &nodes_tool_frame() const;
	const std::vector<std::array<unsigned int, 3>> &triangles() const;
	const std::vector<boundary_edge> &boundary_edges() const;

	double temperature_at_node(unsigned int i) const;
	double temperature_at_world_point_nearest_boundary(glm::dvec2 x_world) const;

	void clear_sources();
	void add_nodal_power(unsigned int node, double power);
	void add_boundary_point_power(glm::dvec2 x_world, double power);
	double nodal_power(unsigned int node) const;

	void clear_forces();
	void add_nodal_force(unsigned int node, glm::dvec2 force);
	void add_boundary_point_force(glm::dvec2 x_world, glm::dvec2 force);
	glm::dvec2 nodal_force(unsigned int node) const;

	glm::dvec2 node_world(unsigned int i) const;
	const std::vector<unsigned int> &boundary_loop_nodes() const;
	std::vector<glm::dvec2> boundary_loop_world() const;
	const std::vector<glm::dvec2> &displacements() const;
	void set_displacements(const std::vector<glm::dvec2> &u);

	void set_convection_on_physical(int physical_tag, convection_bc bc);
	void set_dirichlet_on_physical(int physical_tag, double T);
	void set_convection_air_all_exposed(convection_bc air_bc);
	void set_convection_flooded_by_y(convection_bc air_bc, convection_bc water_bc, double y_threshold_world);

	double max_temperature() const;
	double min_temperature() const;

	void set_mu(double mu);
	double get_mu() const;

	fe_tool::bbox get_bbox_world() const;
	glm::dvec2 get_edge_coord() const;

	// returns distance from qp to tool if qp is inside tool
	// returns -1 otherwise
	double inside(glm::dvec2 qp) const;

	void advance_explicit(double dt);
	void set_mechanics_rayleigh(double a0, double a1);
	void advance_mechanics_explicit(double dt);
	double mechanics_dt_crit() const;
	void solve_mechanics_quasistatic(unsigned int max_iters, double rel_tol);
	double max_displacement_norm() const;

	struct contact_convergence {
		unsigned int iters = 0;
		double rel_force = 0.;
		double rel_power = 0.;
		double max_rel_force_node = 0.;
		double max_rel_power_node = 0.;
		unsigned int nodes_force_over_tol = 0;
		unsigned int nodes_power_over_tol = 0;
	};
	void set_contact_convergence(contact_convergence c);
	contact_convergence get_contact_convergence() const;

	struct contact_energy_balance {
		double P_fric = 0.;
		double P_cond = 0.;
		double scale = 1.;
		double frac_workpiece = 0.;
		double frac_tool = 0.;
	};
	void set_contact_energy_balance(contact_energy_balance b);
	contact_energy_balance get_contact_energy_balance() const;

	struct thermal_energy_accounting {
		double step_dt = 0.;
		double step_contact_E_cond_raw = 0.;
		double step_contact_E_fric_raw = 0.;
		double step_contact_E_cond_scaled = 0.;
		double step_contact_E_fric_scaled = 0.;
		double step_contact_E_workpiece = 0.;
		double step_contact_E_tool = 0.;
		double step_contact_E_limiter_suppressed = 0.;
		double step_tool_E_sources = 0.;
		double step_tool_E_conduction = 0.;
		double step_tool_E_convection = 0.;
		double step_tool_E_dirichlet = 0.;
		double tool_internal_E = 0.;
		double cumulative_contact_E_cond_raw = 0.;
		double cumulative_contact_E_fric_raw = 0.;
		double cumulative_contact_E_cond_scaled = 0.;
		double cumulative_contact_E_fric_scaled = 0.;
		double cumulative_contact_E_workpiece = 0.;
		double cumulative_contact_E_tool = 0.;
		double cumulative_contact_E_limiter_suppressed = 0.;
		double cumulative_tool_E_sources = 0.;
		double cumulative_tool_E_conduction = 0.;

```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 720-790)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;

	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nx*ny];

	srand(0);
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

```


**Tool Call: Read file `src\simulation_time.h` (lines 1-160)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/simulation_time.h
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

#ifndef SIMULATION_TIME_H_
#define SIMULATION_TIME_H_

#include <stdio.h>

/*
 In order to have one single interface through which all temporal
 attributes can be accessed globally.

 It saves:
 time, time step, time step number, t final, etc.
*/

class simulation_time {

public:
	static simulation_time& getInstance();
	simulation_time(simulation_time const &) = delete;
	void operator=(simulation_time const &) = delete;
	double get_time() const;
	double get_dt() const;
	bool finished() const;
	unsigned int get_step() const;
	void increment_time();
	void increment_step();
	double get_t_final() const;

	void set_t_final(double t_final);
	void set_dt(double dt);

	double get_CFL() const;
	void modify_dt(double dt_adapted);

private:
	simulation_time();
	double m_time = 0.;
	double m_dt = 0.;
	double m_t_final = 0.;
	unsigned int m_step = 0;
};

#endif /* SIMULATION_TIME_H_ */

```


## Assistant

We need continue fe_tool header.

**Tool Call: Read file `src\fe_tool.h` (lines 220-520)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.h
		double max_rel_force_node = 0.;
		double max_rel_power_node = 0.;
		unsigned int nodes_force_over_tol = 0;
		unsigned int nodes_power_over_tol = 0;
	};
	void set_contact_convergence(contact_convergence c);
	contact_convergence get_contact_convergence() const;

	struct contact_energy_balance {
		double P_fric = 0.;
		double P_cond = 0.;
		double scale = 1.;
		double frac_workpiece = 0.;
		double frac_tool = 0.;
	};
	void set_contact_energy_balance(contact_energy_balance b);
	contact_energy_balance get_contact_energy_balance() const;

	struct thermal_energy_accounting {
		double step_dt = 0.;
		double step_contact_E_cond_raw = 0.;
		double step_contact_E_fric_raw = 0.;
		double step_contact_E_cond_scaled = 0.;
		double step_contact_E_fric_scaled = 0.;
		double step_contact_E_workpiece = 0.;
		double step_contact_E_tool = 0.;
		double step_contact_E_limiter_suppressed = 0.;
		double step_tool_E_sources = 0.;
		double step_tool_E_conduction = 0.;
		double step_tool_E_convection = 0.;
		double step_tool_E_dirichlet = 0.;
		double tool_internal_E = 0.;
		double cumulative_contact_E_cond_raw = 0.;
		double cumulative_contact_E_fric_raw = 0.;
		double cumulative_contact_E_cond_scaled = 0.;
		double cumulative_contact_E_fric_scaled = 0.;
		double cumulative_contact_E_workpiece = 0.;
		double cumulative_contact_E_tool = 0.;
		double cumulative_contact_E_limiter_suppressed = 0.;
		double cumulative_tool_E_sources = 0.;
		double cumulative_tool_E_conduction = 0.;
		double cumulative_tool_E_convection = 0.;
		double cumulative_tool_E_dirichlet = 0.;
	};
	void reset_thermal_energy_accounting_step(double dt);
	void add_contact_energy_accounting(double dt, double P_cond_raw, double P_fric_raw, double scale, double frac_workpiece, double frac_tool);
	thermal_energy_accounting get_thermal_energy_accounting() const;
	double thermal_internal_energy() const;

	fe_tool();
	virtual ~fe_tool() = default;

	double thermal_dt_crit() const;

private:
	double m_mu = 0.0;

	struct edge_key {
		unsigned int a = 0;
		unsigned int b = 0;
		bool operator==(const edge_key &o) const { return a == o.a && b == o.b; }
	};

	struct edge_key_hash {
		std::size_t operator()(const edge_key &k) const noexcept {
			return (static_cast<std::size_t>(k.a) << 32) ^ static_cast<std::size_t>(k.b);
		}
	};

	void build_conduction_operator();
	void build_conduction_operator_from_temperature();
	void build_boundary_edges_from_lines();
	void build_boundary_edge_to_adjacent_triangle();
	void build_boundary_loop();
	void build_mechanics_operator();
	void build_mechanics_operator_from_temperature();
	void apply_dirichlet_bc(std::vector<char> &is_fixed);
	void build_mech_constrained(std::vector<char> &constrained) const;
	void ensure_mech_fix_cache() const;
	void add_thermoelastic_rhs(std::vector<double> &rhs) const;
	void matvec_mechanics(const std::vector<char> &constrained, const std::vector<double> &x, std::vector<double> &y) const;
	void ensure_mechanics_lumped_mass();
	static double table_eval(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback);
	double rho_at(double T) const;
	double cp_at(double T) const;
	double k_at(double T) const;
	double E_at(double T) const;
	double nu_at(double T) const;
	double alpha_at(double T) const;

	std::pair<unsigned int, double> nearest_boundary_edge_barycentric(glm::dvec2 x_tool) const;

	thermal_material m_mat;
	mechanical_material m_mech;
	double m_T_ref = 0.;

	glm::dvec2 m_pos = glm::dvec2(0.);
	glm::dvec2 m_vel = glm::dvec2(0.);

	std::vector<glm::dvec2> m_nodes_tool;
	std::vector<std::array<unsigned int, 3>> m_tris;
	std::vector<boundary_edge> m_bnd;

	std::unordered_map<int, convection_bc> m_conv_by_tag;
	std::unordered_map<int, double> m_dirichlet_by_tag;
	convection_bc m_air_all;
	bool m_use_air_all = false;
	bool m_use_flooded_by_y = false;
	convection_bc m_flood_air;
	convection_bc m_flood_water;
	double m_flood_y_threshold_world = 0.;
	std::vector<double> m_rho_T;
	std::vector<double> m_rho_val;
	std::vector<double> m_cp_T;
	std::vector<double> m_cp_val;
	std::vector<double> m_k_T;
	std::vector<double> m_k_val;
	std::vector<double> m_E_T;
	std::vector<double> m_E_val;
	std::vector<double> m_nu_T;
	std::vector<double> m_nu_val;
	std::vector<double> m_alpha_T;
	std::vector<double> m_alpha_val;

	std::vector<double> m_T;
	std::vector<double> m_capacity;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_K_rows;
	std::vector<double> m_power_sources;
	std::vector<glm::dvec2> m_force_sources;
	std::vector<glm::dvec2> m_u;
	std::vector<double> m_mech_mass;
	std::vector<double> m_mech_v_half;
	double m_mech_rayleigh_a0 = 0.;
	double m_mech_rayleigh_a1 = 0.;
	bool m_mech_mass_scaled = false;
	bool m_mech_v_half_initialized = false;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_Km_rows;
	std::unordered_set<int> m_mech_fix_tags;
	std::unordered_set<unsigned int> m_mech_fix_nodes;
	std::unordered_set<int> m_mech_fix_x_tags;
	std::unordered_set<int> m_mech_fix_y_tags;
	std::unordered_set<unsigned int> m_mech_fix_x_nodes;
	std::unordered_set<unsigned int> m_mech_fix_y_nodes;
	mutable bool m_mech_fix_cache_valid = false;
	mutable std::unordered_set<unsigned int> m_mech_fix_cache_x_nodes;
	mutable std::unordered_set<unsigned int> m_mech_fix_cache_y_nodes;
	std::vector<unsigned int> m_boundary_loop;
	contact_convergence m_contact_conv;
	contact_energy_balance m_contact_energy;
	thermal_energy_accounting m_thermal_energy;

	std::unordered_map<edge_key, unsigned int, edge_key_hash> m_bnd_edge_to_tri;

	std::vector<boundary_edge> m_line_elements;
};

#endif

```


**Tool Call: Search files for regex `thermal_dt_crit|mechanics_dt_crit|advance_explicit|advance_mechanics_explicit|set_material\(|c0\(|thermal_constants|class physical_constants` (case-sensitive)**
Status: Completed

Showing matches 1-20 (there were more matches found; use offset: 20 to see next page):

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\material_library.cpp

### physical_constants matlib_steel4430() › thermal_constants tc( ) › L70-74
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_ARMCO_iron() › thermal_constants tc( ) › L92-96
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_OFHC_copper() › thermal_constants tc( ) › L114-118
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_AISI1045() › thermal_constants tc( ) › L137-141
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_thermal_synthetic() › L159-169
```
physical_constants matlib_thermal_synthetic() {
	double E = 0;
	double nu = 0.;
	double rho0 = 1.;

	double cp = 1.;
	double tq = 0.;
	double k  = 1.;

	return physical_constants(nu, E, rho0, johnson_cook_constants(), thermal_constants(cp, tq, k));
}
```

### physical_constants matlib_tial6v4_lesuer() › thermal_constants tc( ) › L189-193
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_tial6v4_johnson_SI() › thermal_constants tc( ) › L212-216
```
	double k  = 6.8;

	thermal_constants tc(cp, tq, k);

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
```

### physical_constants matlib_tial6v4_johnson_cm_musec_g() › thermal_constants tc( ) › L239-243
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_tial6v4_Sima_tanh2010_SI() › thermal_constants tc( ) › L272-276
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, JC_tanh_a, JC_tanh_b, JC_tanh_c, JC_tanh_d, JC_tanh_Sima2010_s, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_tial6v4_Sima_tanh2010_cm_musec_g() › thermal_constants tc( ) › L303-307
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, JC_tanh_a, JC_tanh_b, JC_tanh_c, JC_tanh_d, JC_tanh_Sima2010_s, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

### physical_constants matlib_a2024t351() › thermal_constants tc( ) › L335-339
```

	johnson_cook_constants jc(JC_A, JC_B, JC_C, JC_m, JC_n, Tmelt, Tref, eps_dot_ref);
	thermal_constants tc(cp, tq, k);
	return physical_constants(nu, E, rho0, jc, tc);
}
```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_benches.cpp

### body *test_bench_setup_rings( ) › double c0 › L83-87
```
	double vel_rings =  180.;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

```

### body *test_bench_setup_ring_contact( ) › double c0 › L162-166
```
	double vel_rings =  180.;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

```

### body *test_bench_setup_disk_impact( ) › double c0 › L240-244
```
	double vel_rings = 180.;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp

### static fe_tool *attach_fe_tool_from_env( ) › L448-452
```
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
```

### body *cutting_ref_mr( ) › double c0 › L591-595
```
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut
```

### body *cutting_ref_single_resol( ) › double mech_CFL › L753-757
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
```

### body *cutting_ref_multi_resol_apriori( ) › double mech_CFL › L998-1002
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
```

### body *cutting_ref_multi_resol_dynamic( ) › double mech_CFL › L1292-1296
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
```

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\validate_main.cpp

### static bool test_tool_1d_conduction() › L180-190
```
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
```

20 lines remaining in ancestor node. Read the file to see all.


**Tool Call: Read file `src\simulation_data.h` (lines 1-260)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/simulation_data.h
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

#ifndef SIMULATION_DATA_H_
#define SIMULATION_DATA_H_

#include <math.h>

/*
 * for a given body:
 * ------------------
 * this file contains all elastic, plastic, thermal and correction constants
 */

class johnson_cook_constants {
private:
	double m_A = 0.;
	double m_B = 0.;
	double m_C = 0.;
	double m_m = 0.;
	double m_n = 0.;

	double m_a = 0.;
	double m_b = 0.;
	double m_c = 0.;
	double m_d = 0.;
	double m_s = 0.;

	double m_Tmelt = 0.;
	double m_Tref  = 0.;
	double m_eps_ref = 0.;

public:
	johnson_cook_constants(double A, double B, double C, double m, double n, double Tmelt, double Tref, double eps_ref = 1.);
	johnson_cook_constants(double A, double B, double C, double m, double n, double a, double b, double c, double d, double s, double Tmelt, double Tref, double eps_ref = 1.);	// JC-tanh- Erweiterung, Calamaz 2008
	johnson_cook_constants();

	double A() const;
	double B() const;
	double C() const;
	double m() const;
	double n() const;

	double a() const;
	double b() const;
	double c() const;
	double d() const;
	double s() const;

	double eps_pl_0() const;

	double Tmelt() const;
	double Tref() const;
	double eps_ref() const;

	bool valid() const;
};

class thermal_constants {
	double m_cp = 0.;
	double m_Taylor_Quinney = 0.;
	double m_k = 0.;

public:
	thermal_constants(double cp, double Taylor_Quinney, double k = 0.);
	thermal_constants();

	double cp() const;				/*!< Heat capacity */
	double Taylor_Quinney() const;	/*!< Percentage of plastic work converted into thermal energy */
	double k() const;				/*!< Thermal conduction coefficient */
};

class physical_constants {
private:
	double m_nu = 0.;
	double m_E = 0.;
	double m_rho0 = 0.;

	johnson_cook_constants m_jc;
	thermal_constants m_tc;
public:
	physical_constants(double nu, double E, double rho0);
	physical_constants(double nu, double E, double rho0, johnson_cook_constants jc);
	physical_constants(double nu, double E, double rho0, johnson_cook_constants jc, thermal_constants tc);
	physical_constants();
	double nu() const;
	double E() const;
	double G() const;
	double K() const;
	double rho0() const;
	double c0() const;
	double mu_lame() const;
	double lambda_lame() const;
	johnson_cook_constants jc() const;
	thermal_constants tc() const;
};

class constants_monaghan {
	double m_mghn_wdeltap = 0.;
	unsigned int m_mghn_corr_exp = 0;
	double m_mghn_eps = 0.;

public:
	double mghn_wdeltap() const;
	unsigned int mghn_corr_exp() const;
	double mghn_eps() const;

	constants_monaghan(double wdeltap, unsigned int corr_exp, double eps);
	constants_monaghan();
};

class constants_artificial_viscosity {
	double m_artvisc_alpha = 0.;
	double m_artvisc_beta = 0.;
	double m_artvisc_eta = 0.;

public:
	double artvisc_alpha() const;
	double artvisc_beta() const;
	double artvisc_eta() const;

	constants_artificial_viscosity(double alpha, double beta, double eta);
	constants_artificial_viscosity();
};

class correction_constants {

private:
	double m_xsph_eps = 0.;
	constants_monaghan m_constants_monaghan;
	constants_artificial_viscosity m_constants_art_visc;

public:
	correction_constants(constants_monaghan monaghan_constants, constants_artificial_viscosity constants_art_visc, double xsph_eps);
	correction_constants();
	double xsph_eps() const;
	constants_monaghan get_monaghan_const() const;
	constants_artificial_viscosity get_art_visc_const() const;
};

class simulation_data {
private:
	physical_constants   m_physical_constants;
	correction_constants m_correction_constants;

public:
	simulation_data();
	simulation_data(physical_constants physical_constants, correction_constants correction_constants);
	physical_constants   get_physical_constants() const;
	correction_constants get_correction_constants() const;
};

#endif /* SIMULATION_DATA_H_ */

```


**Tool Call: Read file `src\thermal.h` (lines 1-220)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/thermal.h
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

#ifndef THERMAL_H_
#define THERMAL_H_

#include <math.h>
#include <assert.h>
#include <glm/glm.hpp>

#include "grid.h"
#include "kernel.h"
#include "particle.h"
#include "simulation_data.h"

/*
 This implements heat conduction using either of the desired methods:
 	 1- the particle strength exchange (PSE) method.
 	 2- the Brookshaw-SPH method.

 	 > both schemes discretize the heat equation in a Finite-Difference like approach.
 	 > both schemes are energy conservative. (anti-symmetric form)
 	 > both schemes are numerically efficient and capable of handling adiabatic boundary condition without dummy particles.

 	 For further details, please refer to the following publications:

 	 1- "A general deterministic treatment of derivatives in particle methods."
 	    	By: J. Eldredge et al.
 	    	Journal of Computational Physics 180.2 (2002): 686-709.

 	 2- "A method of calculating radiative heat diffusion in particle simulations”
 	 	 	 By: L. Brookshaw
 	 	     Proceedings of the Astronomical Society of Australia, vol. 6, pp. 207–210, 1985"
*/


class body;

class thermal {
public:
	enum thermal_solver {
		thermal_pse,
		thermal_brookshaw,
	};

	void set_method(thermal_solver solver);
	void conduction(body &body) const;
	thermal(physical_constants pc);

private:
	double m_alpha = 0.;
	thermal_solver m_thermal_solver = thermal_pse;

	void heat_conduction_pse(body &b) const;
	void heat_conduction_brookshaw(body &b) const;
};

#endif /* THERMAL_H_ */

```


## Assistant

<thinking>
**Planning method implementation**

I need to implement a method for estimating the maximum time step size, taking into account the coupled thermal-structural simulation and different material properties for both the FE tool and the SPH workpiece. It makes sense to add a class called `timestep_estimator`. I also need to understand the CMake source list and how to set up the body attachment for the FE tool. I'll read the FE tool implementation to figure out the critical time step.
</thinking>

**Tool Call: Read file `src\fe_tool.cpp` (lines 1-260)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp
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

#include "fe_tool.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>

static bool starts_with(const std::string &s, const char *prefix) {
	return s.rfind(prefix, 0) == 0;
}

static double tri_area2(const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

fe_tool::fe_tool() {}

double fe_tool::table_eval(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback) {
	if (T_tab.size() < 2 || T_tab.size() != v_tab.size() || !std::isfinite(T)) return fallback;
	if (T <= T_tab.front()) return v_tab.front();
	if (T >= T_tab.back()) return v_tab.back();
	auto it = std::upper_bound(T_tab.begin(), T_tab.end(), T);
	std::size_t i1 = static_cast<std::size_t>(it - T_tab.begin());
	if (i1 == 0 || i1 >= T_tab.size()) return fallback;
	std::size_t i0 = i1 - 1;
	double T0 = T_tab[i0];
	double T1 = T_tab[i1];
	double v0 = v_tab[i0];
	double v1 = v_tab[i1];
	double dT = T1 - T0;
	if (!(dT > 0.)) return fallback;
	double a = (T - T0) / dT;
	return (1.0 - a) * v0 + a * v1;
}

double fe_tool::rho_at(double T) const { return table_eval(T, m_rho_T, m_rho_val, m_mat.rho); }
double fe_tool::cp_at(double T) const { return table_eval(T, m_cp_T, m_cp_val, m_mat.cp); }
double fe_tool::k_at(double T) const { return table_eval(T, m_k_T, m_k_val, m_mat.k); }
double fe_tool::E_at(double T) const { return table_eval(T, m_E_T, m_E_val, m_mech.E); }
double fe_tool::nu_at(double T) const { return table_eval(T, m_nu_T, m_nu_val, m_mech.nu); }
double fe_tool::alpha_at(double T) const { return table_eval(T, m_alpha_T, m_alpha_val, m_mech.alpha); }

void fe_tool::set_mesh(const std::vector<glm::dvec2> &nodes_tool_frame,
                       const std::vector<std::array<unsigned int, 3>> &triangles,
                       const std::vector<boundary_edge> &boundary_edges) {
	m_nodes_tool = nodes_tool_frame;
	m_tris = triangles;
	m_bnd = boundary_edges;
	m_line_elements = boundary_edges;
	m_bnd_edge_to_tri.clear();

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_K_rows.assign(m_nodes_tool.size(), {});
	m_power_sources.assign(m_nodes_tool.size(), 0.);
	m_force_sources.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();
	build_conduction_operator();
	build_mechanics_operator();
}

bool fe_tool::load_gmsh_msh2(const std::string &path) {
	std::ifstream in(path);
	if (!in) return false;

	m_nodes_tool.clear();
	m_tris.clear();
	m_bnd.clear();
	m_line_elements.clear();
	m_bnd_edge_to_tri.clear();
	m_T.clear();
	m_capacity.clear();
	m_K_rows.clear();
	m_power_sources.clear();

	std::string line;
	int msh_version_major = 0;
	while (std::getline(in, line)) {
		if (line == "$MeshFormat") {
			std::getline(in, line);
			std::istringstream iss(line);
			double ver = 0.;
			int file_type = 0;
			int data_size = 0;
			iss >> ver >> file_type >> data_size;
			msh_version_major = static_cast<int>(std::floor(ver + 1e-12));
			std::getline(in, line);
		} else if (line == "$Nodes") {
			std::getline(in, line);
			std::size_t n = 0;
			{
				std::istringstream iss(line);
				iss >> n;
			}
			m_nodes_tool.resize(n);
			for (std::size_t i = 0; i < n; i++) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::size_t id = 0;
				double x = 0., y = 0., z = 0.;
				iss >> id >> x >> y >> z;
				if (id == 0 || id > n) return false;
				m_nodes_tool[id - 1] = glm::dvec2(x, y);
			}
			std::getline(in, line);
		} else if (line == "$Elements") {
			std::getline(in, line);
			std::size_t n = 0;
			{
				std::istringstream iss(line);
				iss >> n;
			}
			for (std::size_t i = 0; i < n; i++) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::size_t id = 0;
				int type = 0;
				int num_tags = 0;
				iss >> id >> type >> num_tags;

				int physical = 0;
				for (int t = 0; t < num_tags; t++) {
					int tag = 0;
					iss >> tag;
					if (t == 0) physical = tag;
				}

				if (type == 2) {
					unsigned int n0 = 0, n1 = 0, n2 = 0;
					iss >> n0 >> n1 >> n2;
					if (n0 == 0 || n1 == 0 || n2 == 0) return false;
					m_tris.push_back({n0 - 1, n1 - 1, n2 - 1});
				} else if (type == 1) {
					unsigned int n0 = 0, n1 = 0;
					iss >> n0 >> n1;
					if (n0 == 0 || n1 == 0) return false;
					boundary_edge e;
					e.n0 = n0 - 1;
					e.n1 = n1 - 1;
					e.physical_tag = physical;
					m_line_elements.push_back(e);
				}
			}
			std::getline(in, line);
		}
	}

	if (msh_version_major != 2) return false;
	if (m_nodes_tool.empty()) return false;
	if (m_tris.empty()) return false;

	build_boundary_edges_from_lines();
	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_power_sources.assign(m_nodes_tool.size(), 0.);
	m_force_sources.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_conduction_operator();
	build_mechanics_operator();

	return true;
}

void fe_tool::set_material(thermal_material mat) {
	m_mat = mat;
	build_conduction_operator();
}

fe_tool::thermal_material fe_tool::get_material() const {
	return m_mat;
}

void fe_tool::set_mechanical_material(mechanical_material mat) {
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(mat.E) || mat.E <= eps) return;
	if (!std::isfinite(mat.nu) || mat.nu <= (-1.0 + eps) || mat.nu >= (0.5 - eps)) return;
	if (!std::isfinite(mat.alpha) || mat.alpha < -eps) return;
	m_mech = mat;
	build_mechanics_operator();
}

fe_tool::mechanical_material fe_tool::get_mechanical_material() const { return m_mech; }

void fe_tool::set_reference_temperature(double T_ref) { m_T_ref = T_ref; }

double fe_tool::reference_temperature() const { return m_T_ref; }

void fe_tool::set_mechanics_fixed_on_physical(int physical_tag) {
	m_mech_fix_tags.insert(physical_tag);
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_x_on_physical(int physical_tag) {
	m_mech_fix_x_tags.insert(physical_tag);
	m_mech_fix_cache_valid = false;

```


**Tool Call: Read file `src\fe_tool.cpp` (lines 260-620)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_y_on_physical(int physical_tag) {
	m_mech_fix_y_tags.insert(physical_tag);
	m_mech_fix_cache_valid = false;
}

void fe_tool::clear_mechanics_fixed() {
	m_mech_fix_tags.clear();
	m_mech_fix_x_tags.clear();
	m_mech_fix_y_tags.clear();
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_nodes.clear();
	m_mech_fix_nodes.insert(nodes.begin(), nodes.end());
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_x_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_x_nodes.clear();
	m_mech_fix_x_nodes.insert(nodes.begin(), nodes.end());
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_y_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_y_nodes.clear();
	m_mech_fix_y_nodes.insert(nodes.begin(), nodes.end());
	m_mech_fix_cache_valid = false;
}

void fe_tool::clear_mechanics_fixed_nodes() {
	m_mech_fix_nodes.clear();
	m_mech_fix_x_nodes.clear();
	m_mech_fix_y_nodes.clear();
	m_mech_fix_cache_valid = false;
}

void fe_tool::ensure_mech_fix_cache() const {
	if (m_mech_fix_cache_valid) return;

	m_mech_fix_cache_x_nodes.clear();
	m_mech_fix_cache_y_nodes.clear();

	for (unsigned int n : m_mech_fix_nodes) {
		m_mech_fix_cache_x_nodes.insert(n);
		m_mech_fix_cache_y_nodes.insert(n);
	}
	for (unsigned int n : m_mech_fix_x_nodes) {
		m_mech_fix_cache_x_nodes.insert(n);
	}
	for (unsigned int n : m_mech_fix_y_nodes) {
		m_mech_fix_cache_y_nodes.insert(n);
	}

	if (!m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			bool fix_xy = (!m_mech_fix_tags.empty() && m_mech_fix_tags.find(e.physical_tag) != m_mech_fix_tags.end());
			bool fix_x = (!m_mech_fix_x_tags.empty() && m_mech_fix_x_tags.find(e.physical_tag) != m_mech_fix_x_tags.end());
			bool fix_y = (!m_mech_fix_y_tags.empty() && m_mech_fix_y_tags.find(e.physical_tag) != m_mech_fix_y_tags.end());
			if (!(fix_xy || fix_x || fix_y)) continue;
			if (fix_xy || fix_x) {
				m_mech_fix_cache_x_nodes.insert(e.n0);
				m_mech_fix_cache_x_nodes.insert(e.n1);
			}
			if (fix_xy || fix_y) {
				m_mech_fix_cache_y_nodes.insert(e.n0);
				m_mech_fix_cache_y_nodes.insert(e.n1);
			}
		}
	}

	m_mech_fix_cache_valid = true;
}

bool fe_tool::is_mechanics_fixed_x(unsigned int node) const {
	ensure_mech_fix_cache();
	return m_mech_fix_cache_x_nodes.find(node) != m_mech_fix_cache_x_nodes.end();
}

bool fe_tool::is_mechanics_fixed_y(unsigned int node) const {
	ensure_mech_fix_cache();
	return m_mech_fix_cache_y_nodes.find(node) != m_mech_fix_cache_y_nodes.end();
}

void fe_tool::set_initial_temperature(double T0) {
	for (std::size_t i = 0; i < m_T.size(); i++) m_T[i] = T0;
	m_T_ref = T0;
	m_thermal_energy.tool_internal_E = thermal_internal_energy();
}

void fe_tool::reset_thermal_energy_accounting_step(double dt) {
	m_thermal_energy.step_dt = dt;
	m_contact_energy = contact_energy_balance();
	m_thermal_energy.step_contact_E_cond_raw = 0.;
	m_thermal_energy.step_contact_E_fric_raw = 0.;
	m_thermal_energy.step_contact_E_cond_scaled = 0.;
	m_thermal_energy.step_contact_E_fric_scaled = 0.;
	m_thermal_energy.step_contact_E_workpiece = 0.;
	m_thermal_energy.step_contact_E_tool = 0.;
	m_thermal_energy.step_contact_E_limiter_suppressed = 0.;
	m_thermal_energy.step_tool_E_sources = 0.;
	m_thermal_energy.step_tool_E_conduction = 0.;
	m_thermal_energy.step_tool_E_convection = 0.;
	m_thermal_energy.step_tool_E_dirichlet = 0.;
	m_thermal_energy.cumulative_contact_E_cond_raw = 0.;
	m_thermal_energy.cumulative_contact_E_fric_raw = 0.;
	m_thermal_energy.cumulative_contact_E_cond_scaled = 0.;
	m_thermal_energy.cumulative_contact_E_fric_scaled = 0.;
	m_thermal_energy.cumulative_contact_E_workpiece = 0.;
	m_thermal_energy.cumulative_contact_E_tool = 0.;
	m_thermal_energy.cumulative_contact_E_limiter_suppressed = 0.;
	m_thermal_energy.cumulative_tool_E_sources = 0.;
	m_thermal_energy.cumulative_tool_E_conduction = 0.;
	m_thermal_energy.cumulative_tool_E_convection = 0.;
	m_thermal_energy.cumulative_tool_E_dirichlet = 0.;
	m_thermal_energy.tool_internal_E = thermal_internal_energy();
}

void fe_tool::add_contact_energy_accounting(double dt, double P_cond_raw, double P_fric_raw, double scale, double frac_workpiece, double frac_tool) {
	if (!std::isfinite(dt) || dt <= 0.) return;
	if (!std::isfinite(P_cond_raw)) P_cond_raw = 0.;
	if (!std::isfinite(P_fric_raw) || P_fric_raw < 0.) P_fric_raw = 0.;
	if (!std::isfinite(scale) || scale < 0.) scale = 1.;
	if (!std::isfinite(frac_workpiece)) frac_workpiece = 0.;
	if (!std::isfinite(frac_tool)) frac_tool = 0.;

	double P_cond_scaled = scale * P_cond_raw;
	double P_fric_scaled = scale * P_fric_raw;
	double E_cond_raw = dt * P_cond_raw;
	double E_fric_raw = dt * P_fric_raw;
	double E_cond_scaled = dt * P_cond_scaled;
	double E_fric_scaled = dt * P_fric_scaled;
	double E_workpiece = dt * (-P_cond_scaled + frac_workpiece * P_fric_scaled);
	double E_tool = dt * (P_cond_scaled + frac_tool * P_fric_scaled);
	double E_suppressed = dt * ((std::abs(P_cond_raw) + P_fric_raw) - (std::abs(P_cond_scaled) + P_fric_scaled));
	if (!std::isfinite(E_suppressed) || E_suppressed < 0.) E_suppressed = 0.;

	m_thermal_energy.step_contact_E_cond_raw += E_cond_raw;
	m_thermal_energy.step_contact_E_fric_raw += E_fric_raw;
	m_thermal_energy.step_contact_E_cond_scaled += E_cond_scaled;
	m_thermal_energy.step_contact_E_fric_scaled += E_fric_scaled;
	m_thermal_energy.step_contact_E_workpiece += E_workpiece;
	m_thermal_energy.step_contact_E_tool += E_tool;
	m_thermal_energy.step_contact_E_limiter_suppressed += E_suppressed;

	m_thermal_energy.cumulative_contact_E_cond_raw = m_thermal_energy.step_contact_E_cond_raw;
	m_thermal_energy.cumulative_contact_E_fric_raw = m_thermal_energy.step_contact_E_fric_raw;
	m_thermal_energy.cumulative_contact_E_cond_scaled = m_thermal_energy.step_contact_E_cond_scaled;
	m_thermal_energy.cumulative_contact_E_fric_scaled = m_thermal_energy.step_contact_E_fric_scaled;
	m_thermal_energy.cumulative_contact_E_workpiece = m_thermal_energy.step_contact_E_workpiece;
	m_thermal_energy.cumulative_contact_E_tool = m_thermal_energy.step_contact_E_tool;
	m_thermal_energy.cumulative_contact_E_limiter_suppressed = m_thermal_energy.step_contact_E_limiter_suppressed;
}

fe_tool::thermal_energy_accounting fe_tool::get_thermal_energy_accounting() const {
	thermal_energy_accounting e = m_thermal_energy;
	e.tool_internal_E = thermal_internal_energy();
	return e;
}

double fe_tool::thermal_internal_energy() const {
	double E = 0.;
	const std::size_t n = std::min(m_T.size(), m_capacity.size());
	for (std::size_t i = 0; i < n; i++) {
		if (!std::isfinite(m_T[i]) || !std::isfinite(m_capacity[i])) continue;
		E += m_capacity[i] * m_T[i];
	}
	return E;
}

void fe_tool::set_pose(glm::dvec2 pos, glm::dvec2 vel) {
	m_pos = pos;
	m_vel = vel;
}

glm::dvec2 fe_tool::get_pos() const { return m_pos; }
glm::dvec2 fe_tool::get_vel() const { return m_vel; }

void fe_tool::update_pose(double dt) { m_pos += dt * m_vel; }

double fe_tool::thermal_dt_crit() const {
	if (m_T.empty()) return std::numeric_limits<double>::infinity();
	if (m_capacity.size() != m_T.size()) return std::numeric_limits<double>::infinity();
	if (m_K_rows.size() != m_T.size()) return std::numeric_limits<double>::infinity();

	std::vector<char> is_fixed(m_T.size(), 0);
	if (!m_bnd.empty() && !m_dirichlet_by_tag.empty()) {
		for (const boundary_edge &e : m_bnd) {
			if (m_dirichlet_by_tag.find(e.physical_tag) == m_dirichlet_by_tag.end()) continue;
			if (e.n0 < is_fixed.size()) is_fixed[e.n0] = 1;
			if (e.n1 < is_fixed.size()) is_fixed[e.n1] = 1;
		}
	}

	std::vector<double> row_sum_abs(m_T.size(), 0.0);
	for (std::size_t i = 0; i < m_K_rows.size(); i++) {
		double s = 0.0;
		for (const auto &kv : m_K_rows[i]) {
			double w = kv.second;
			if (!std::isfinite(w)) continue;
			s += std::abs(w);
		}
		row_sum_abs[i] = s;
	}

	if (!m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			convection_bc bc;
			bool active = false;

			auto it = m_conv_by_tag.find(e.physical_tag);
			if (it != m_conv_by_tag.end()) {
				bc = it->second;
				active = bc.h > 0.;
			} else if (m_use_air_all) {
				bc = m_air_all;
				active = bc.h > 0.;
			} else if (m_use_flooded_by_y) {
				glm::dvec2 p0 = to_world_frame(m_nodes_tool[e.n0]);
				glm::dvec2 p1 = to_world_frame(m_nodes_tool[e.n1]);
				double ym = 0.5 * (p0.y + p1.y);
				bc = (ym >= m_flood_y_threshold_world) ? m_flood_water : m_flood_air;
				active = bc.h > 0.;
			}

			if (!active) continue;

			glm::dvec2 x0 = m_nodes_tool[e.n0];
			glm::dvec2 x1 = m_nodes_tool[e.n1];
			double L = glm::length(x1 - x0);
			if (!(L > 0.0) || !std::isfinite(L)) continue;

			double k_diag = bc.h * L / 3.0;
			double k_off = bc.h * L / 6.0;
			if (!std::isfinite(k_diag) || !std::isfinite(k_off)) continue;
			if (e.n0 < row_sum_abs.size()) row_sum_abs[e.n0] += std::abs(k_diag) + std::abs(k_off);
			if (e.n1 < row_sum_abs.size()) row_sum_abs[e.n1] += std::abs(k_diag) + std::abs(k_off);
		}
	}

	double dt_min = std::numeric_limits<double>::infinity();
	for (std::size_t i = 0; i < row_sum_abs.size(); i++) {
		if (is_fixed[i]) continue;
		double cap = m_capacity[i];
		if (!(cap > 0.0) || !std::isfinite(cap)) continue;
		double s = row_sum_abs[i];
		if (!std::isfinite(s) || s <= 0.0) continue;
		double dt_i = 2.0 * cap / s;
		if (std::isfinite(dt_i) && dt_i > 0.0) dt_min = std::min(dt_min, dt_i);
	}
	if (!std::isfinite(dt_min)) return std::numeric_limits<double>::infinity();
	return 0.9 * dt_min;
}

glm::dvec2 fe_tool::to_tool_frame(glm::dvec2 x_world) const { return x_world - m_pos; }
glm::dvec2 fe_tool::to_world_frame(glm::dvec2 x_tool) const { return x_tool + m_pos; }

const std::vector<glm::dvec2> &fe_tool::nodes_tool_frame() const { return m_nodes_tool; }
const std::vector<std::array<unsigned int, 3>> &fe_tool::triangles() const { return m_tris; }
const std::vector<fe_tool::boundary_edge> &fe_tool::boundary_edges() const { return m_bnd; }

double fe_tool::temperature_at_node(unsigned int i) const {
	if (i >= m_T.size()) return 0.;
	return m_T[i];
}

double fe_tool::temperature_at_world_point_nearest_boundary(glm::dvec2 x_world) const {
	if (!std::isfinite(x_world.x) || !std::isfinite(x_world.y)) return 0.;
	glm::dvec2 x_tool = to_tool_frame(x_world);
	const auto bt = nearest_boundary_edge_barycentric(x_tool);
	const unsigned int edge_idx = bt.first;
	const double t = bt.second;
	if (edge_idx >= m_bnd.size()) return 0.;
	if (!std::isfinite(t)) return 0.;

	const boundary_edge &e = m_bnd[edge_idx];
	double T0 = m_T[e.n0];
	double T1 = m_T[e.n1];
	return (1. - t) * T0 + t * T1;
}

void fe_tool::clear_sources() {
	for (std::size_t i = 0; i < m_power_sources.size(); i++) m_power_sources[i] = 0.;
}

void fe_tool::add_nodal_power(unsigned int node, double power) {
	if (node >= m_power_sources.size()) return;
	m_power_sources[node] += power;
}

void fe_tool::add_boundary_point_power(glm::dvec2 x_world, double power) {
	if (!std::isfinite(x_world.x) || !std::isfinite(x_world.y)) return;
	if (!std::isfinite(power)) return;
	glm::dvec2 x_tool = to_tool_frame(x_world);
	const auto bt = nearest_boundary_edge_barycentric(x_tool);
	const unsigned int edge_idx = bt.first;
	const double t = bt.second;
	if (edge_idx >= m_bnd.size()) return;
	if (!std::isfinite(t)) return;

	const boundary_edge &e = m_bnd[edge_idx];
	m_power_sources[e.n0] += (1. - t) * power;
	m_power_sources[e.n1] += t * power;
}

double fe_tool::nodal_power(unsigned int node) const {
	if (node >= m_power_sources.size()) return 0.;
	return m_power_sources[node];
}

void fe_tool::clear_forces() {
	for (std::size_t i = 0; i < m_force_sources.size(); i++) m_force_sources[i] = glm::dvec2(0.);
}

void fe_tool::add_nodal_force(unsigned int node, glm::dvec2 force) {
	if (node >= m_force_sources.size()) return;
	m_force_sources[node] += force;
}

void fe_tool::add_boundary_point_force(glm::dvec2 x_world, glm::dvec2 force) {
	if (!std::isfinite(x_world.x) || !std::isfinite(x_world.y)) return;
	if (!std::isfinite(force.x) || !std::isfinite(force.y)) return;
	glm::dvec2 x_tool = to_tool_frame(x_world);
	const auto bt = nearest_boundary_edge_barycentric(x_tool);
	const unsigned int edge_idx = bt.first;
	const double t = bt.second;
	if (edge_idx >= m_bnd.size()) return;
	if (!std::isfinite(t)) return;

	const boundary_edge &e = m_bnd[edge_idx];
	m_force_sources[e.n0] += (1. - t) * force;
	m_force_sources[e.n1] += t * force;
}

glm::dvec2 fe_tool::nodal_force(unsigned int node) const {
	if (node >= m_force_sources.size()) return glm::dvec2(0.);
	return m_force_sources[node];
}

glm::dvec2 fe_tool::node_world(unsigned int i) const {
	if (i >= m_nodes_tool.size()) return glm::dvec2(0.);
	glm::dvec2 x_tool = m_nodes_tool[i] + (i < m_u.size() ? m_u[i] : glm::dvec2(0.));
	return to_world_frame(x_tool);
}

const std::vector<unsigned int> &fe_tool::boundary_loop_nodes() const { return m_boundary_loop; }

std::vector<glm::dvec2> fe_tool::boundary_loop_world() const {
	if (m_boundary_loop.size() >= 3) {
		std::vector<glm::dvec2> pts;
		pts.reserve(m_boundary_loop.size());
		for (unsigned int i : m_boundary_loop) pts.push_back(node_world(i));
		return pts;
	}

	std::unordered_set<unsigned int> bnodes;
	bnodes.reserve(2 * m_bnd.size());
	for (const boundary_edge &e : m_bnd) {

```


**Tool Call: Read file `CMakeLists.txt` (lines 1-220)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(mfree_iwf_ul_cut_refine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mfree_iwf
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/refine_cut_main.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/test_benches.cpp
  src/benchmarks/test_cuttings.cpp
  src/benchmarks/test_density.cpp
)

target_include_directories(mfree_iwf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf PRIVATE OpenMP::OpenMP_CXX)
endif()

enable_testing()

add_executable(mfree_iwf_validate
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/validate_main.cpp
)

target_include_directories(mfree_iwf_validate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf_validate PRIVATE OpenMP::OpenMP_CXX)
endif()

add_test(NAME mfree_iwf_validate COMMAND mfree_iwf_validate)

add_test(NAME mfree_iwf_preprocess_model1_rigid COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_rigid PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_rigid"
)

add_test(NAME mfree_iwf_preprocess_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh"
)

add_test(NAME mfree_iwf_geom_validate_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_geom_validate_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/geom_model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh;MFREE_GEOM_VALIDATE=1"
)

add_executable(mfree_geom_validation_math_test
  src/benchmarks/test_geom_validation_math.cpp
)

target_include_directories(mfree_geom_validation_math_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

add_test(NAME mfree_geom_validation_math_test COMMAND mfree_geom_validation_math_test)

```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 400-490)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;

		double y_bottom = poly_min_y(poly);
		double dy = (wp_corner.y - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft.get_pos();
		pos.y += dy;
		ft.set_pose(pos, ft.get_vel());

		poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;
		glm::dvec2 cp = closest_point_on_polyline(wp_corner, poly);
		double dx = wp_corner.x - cp.x;
		pos = ft.get_pos();
		pos.x += dx;
		ft.set_pose(pos, ft.get_vel());
	}
}

static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
                                     glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def)) msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
			exit(1);
		}
	}

	fe_tool *ft = new fe_tool();
	if (!ft->load_gmsh_msh2(msh)) {
		delete ft;
		std::fprintf(stderr, "Failed to load MFREE_FE_TOOL_MSH\n");
		exit(1);
	}

	fe_tool::thermal_material mat;
	mat.rho = 14500.0;
	mat.cp = 200.0;
	mat.k = 80.0;
	try_read_env_double("MFREE_FE_TOOL_RHO", mat.rho);
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_RHO_TABLE", T, v)) ft->set_material_table_rho(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_CP_TABLE", T, v)) ft->set_material_table_cp(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_K_TABLE", T, v)) ft->set_material_table_k(std::move(T), std::move(v));
	}

	fe_tool::mechanical_material mech;
	mech.E = 600e9;
	mech.nu = 0.22;
	mech.alpha = 4.5e-6;
	try_read_env_double("MFREE_FE_TOOL_E", mech.E);
	try_read_env_double("MFREE_FE_TOOL_NU", mech.nu);
	try_read_env_double("MFREE_FE_TOOL_ALPHA", mech.alpha);
	ft->set_mechanical_material(mech);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_E_TABLE", T, v)) ft->set_mechanical_table_E(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_NU_TABLE", T, v)) ft->set_mechanical_table_nu(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_ALPHA_TABLE", T, v)) ft->set_mechanical_table_alpha(std::move(T), std::move(v));
	}
	ft->set_reference_temperature(T0);
	bool bc_validate = false;
	{
		int v = 0;
		if (try_read_env_int("MFREE_FE_BC_VALIDATE", v) && v != 0) bc_validate = true;
	}
	if (!bc_validate) {
		apply_mech_fix_tags_from_env(*ft);
	}

	ft->set_initial_temperature(T0);
	glm::dvec2 pos(0.);
	const char *align_env = getenv("MFREE_FE_TOOL_ALIGN_CENTER");
	bool align = true;
	if (align_env) align = (atoi(align_env) != 0);

```


## Assistant

Need rest fe_tool dt mechanics.

**Tool Call: Search files for regex `double fe_tool::mechanics_dt_crit|advance_mechanics_explicit|build_mechanics_operator|build_conduction_operator|set_material_table` (case-sensitive)**
Status: Completed

Found 15 matches:

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\fe_tool.cpp

### void fe_tool::set_mesh( ) › L116-121
```
	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();
	build_conduction_operator();
	build_mechanics_operator();
}

```

### bool fe_tool::load_gmsh_msh2( ) › L221-226
```
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_conduction_operator();
	build_mechanics_operator();

	return true;
```

### void fe_tool::set_material( ) › L229-232
```
void fe_tool::set_material(thermal_material mat) {
	m_mat = mat;
	build_conduction_operator();
}
```

### void fe_tool::set_mechanical_material( ) › L238-245
```
void fe_tool::set_mechanical_material(mechanical_material mat) {
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(mat.E) || mat.E <= eps) return;
	if (!std::isfinite(mat.nu) || mat.nu <= (-1.0 + eps) || mat.nu >= (0.5 - eps)) return;
	if (!std::isfinite(mat.alpha) || mat.alpha < -eps) return;
	m_mech = mat;
	build_mechanics_operator();
}
```

### void fe_tool::set_material_table_rho( ) › L681-684
```
void fe_tool::set_material_table_rho(std::vector<double> T, std::vector<double> rho) {
	m_rho_T = std::move(T);
	m_rho_val = std::move(rho);
}
```

### void fe_tool::set_material_table_cp( ) › L686-689
```
void fe_tool::set_material_table_cp(std::vector<double> T, std::vector<double> cp) {
	m_cp_T = std::move(T);
	m_cp_val = std::move(cp);
}
```

### void fe_tool::set_material_table_k( ) › L691-694
```
void fe_tool::set_material_table_k(std::vector<double> T, std::vector<double> k) {
	m_k_T = std::move(T);
	m_k_val = std::move(k);
}
```

### void fe_tool::advance_explicit( ) › L764-774
```
void fe_tool::advance_explicit(double dt) {
	if (m_T.empty()) return;
	if (m_capacity.size() != m_T.size()) return;
	if (m_K_rows.size() != m_T.size()) return;
	if (m_mat.rho <= 0. || m_mat.cp <= 0. || m_mat.k < 0.) return;
	if (!std::isfinite(dt) || dt <= 0.) return;

	if (!m_k_T.empty() || !m_cp_T.empty() || !m_rho_T.empty()) build_conduction_operator_from_temperature();

	std::vector<char> is_fixed;
	apply_dirichlet_bc(is_fixed);
```

90 lines remaining in ancestor node. Read the file to see all.

### void fe_tool::build_conduction_operator() › L1005-1015
```
void fe_tool::build_conduction_operator() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_mat.rho <= 0. || m_mat.cp <= 0. || m_mat.k < 0.) return;

	m_capacity.assign(m_nodes_tool.size(), 0.);

	std::vector<std::unordered_map<unsigned int, double>> rows(m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
```

51 lines remaining in ancestor node. Read the file to see all.

### void fe_tool::build_conduction_operator_from_temperature() › L1068-1078
```
void fe_tool::build_conduction_operator_from_temperature() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_T.size() != m_nodes_tool.size()) return;

	m_capacity.assign(m_nodes_tool.size(), 0.);
	std::vector<std::unordered_map<unsigned int, double>> rows(m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
```

58 lines remaining in ancestor node. Read the file to see all.

### void fe_tool::build_mechanics_operator() › L1138-1148
```
void fe_tool::build_mechanics_operator() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(m_mech.E) || m_mech.E <= eps) return;
	if (!std::isfinite(m_mech.nu) || m_mech.nu <= (-1.0 + eps) || m_mech.nu >= (0.5 - eps)) return;
	if (!std::isfinite(m_mech.alpha) || m_mech.alpha < -eps) return;

	std::vector<std::unordered_map<unsigned int, double>> rows(2 * m_nodes_tool.size());

	double E = m_mech.E;
	double nu = m_mech.nu;
```

73 lines remaining in ancestor node. Read the file to see all.

### void fe_tool::build_mechanics_operator_from_temperature() › L1223-1233
```
void fe_tool::build_mechanics_operator_from_temperature() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_T.size() != m_nodes_tool.size()) return;
	const double eps = std::numeric_limits<double>::epsilon();

	std::vector<std::unordered_map<unsigned int, double>> rows(2 * m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
```

81 lines remaining in ancestor node. Read the file to see all.

### double fe_tool::mechanics_dt_crit() › L1458-1468
```
double fe_tool::mechanics_dt_crit() const {
	if (m_nodes_tool.empty() || m_tris.empty()) return std::numeric_limits<double>::infinity();
	const double eps = std::numeric_limits<double>::epsilon();

	double dtmin = std::numeric_limits<double>::infinity();
	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;
		const glm::dvec2 &x0 = m_nodes_tool[i0];
```

32 lines remaining in ancestor node. Read the file to see all.

### void fe_tool::advance_mechanics_explicit( ) › L1502-1512
```
void fe_tool::advance_mechanics_explicit(double dt) {
	if (dt <= 0.) return;
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_Km_rows.size() != 2 * m_nodes_tool.size()) return;
	if (m_force_sources.size() != m_nodes_tool.size()) return;

	if (!m_E_T.empty() || !m_nu_T.empty()) build_mechanics_operator_from_temperature();

	ensure_mechanics_lumped_mass();
	if (m_mech_mass.size() != 2 * m_nodes_tool.size()) return;

```

106 lines remaining in ancestor node. Read the file to see all.

### void fe_tool::solve_mechanics_quasistatic( ) › L1620-1630
```
void fe_tool::solve_mechanics_quasistatic(unsigned int max_iters, double rel_tol) {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_Km_rows.size() != 2 * m_nodes_tool.size()) return;
	if (m_force_sources.size() != m_nodes_tool.size()) return;

	if (!m_E_T.empty() || !m_nu_T.empty()) build_mechanics_operator_from_temperature();

	const double eps = std::numeric_limits<double>::epsilon();

	std::vector<char> constrained;
	build_mech_constrained(constrained);
```

93 lines remaining in ancestor node. Read the file to see all.


**Tool Call: Read file `src\fe_tool.cpp` (lines 1450-1505)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp
		m_mech_mass[2 * i + 0] = nodal_mass[i];
		m_mech_mass[2 * i + 1] = nodal_mass[i];
	}
	m_mech_v_half.assign(ndof, 0.);
	m_mech_mass_scaled = false;
	m_mech_v_half_initialized = false;
}

double fe_tool::mechanics_dt_crit() const {
	if (m_nodes_tool.empty() || m_tris.empty()) return std::numeric_limits<double>::infinity();
	const double eps = std::numeric_limits<double>::epsilon();

	double dtmin = std::numeric_limits<double>::infinity();
	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;
		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];
		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;
		double Tavg = m_T.size() == m_nodes_tool.size() ? (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0 : m_T_ref;
		double rho = rho_at(Tavg);
		double E = E_at(Tavg);
		double nu = nu_at(Tavg);
		if (!std::isfinite(rho) || rho <= 0.) rho = m_mat.rho;
		if (!std::isfinite(E) || E <= eps) E = m_mech.E;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) nu = m_mech.nu;
		if (!std::isfinite(rho) || rho <= 0.) continue;
		if (!std::isfinite(E) || E <= eps) continue;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) continue;

		double mu = E / (2.0 * (1.0 + nu));
		double lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
		double c = std::sqrt((lambda + 2.0 * mu) / rho);
		if (!(c > 0.) || !std::isfinite(c)) continue;
		double e01 = glm::length(x1 - x0);
		double e12 = glm::length(x2 - x1);
		double e20 = glm::length(x0 - x2);
		double emax = std::max(e01, std::max(e12, e20));
		if (!(emax > 0.)) continue;
		double h = 2.0 * A / emax;
		if (!(h > 0.)) continue;
		double dt = h / c;
		if (dt > 0. && std::isfinite(dt)) dtmin = std::min(dtmin, dt);
	}
	return dtmin;
}

void fe_tool::advance_mechanics_explicit(double dt) {
	if (dt <= 0.) return;
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_Km_rows.size() != 2 * m_nodes_tool.size()) return;

```


## Assistant

<thinking>
**Considering implementation steps**

I’m looking at the need to implement and possibly integrate some computations in the test_cuttings data. I’ll also need to run tests to ensure everything is working smoothly. It seems like I should check if the body has a pointer to the fe tool to make sure I’m covering all necessary aspects. Planning out these details is essential for a successful integration! Let’s get this right.
</thinking>

**Tool Call: Read file `src\body.h` (lines 1-260)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/body.h
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

#ifndef BODY_H_
#define BODY_H_

#include <type_traits>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "particle.h"
#include "simulation_data.h"
#include "plasticity.h"
#include "precomp_shape_functions.h"
#include "thermal.h"
#include "contact.h"

#include "adaptivity.h"

class fe_tool;

class body {

private:
	/*
	 "BODY" is comprised of the following encapsulated items:
	*/

	grid m_grid;             			// spatial hashing
	plasticity *m_plast = 0; 			// plasticity algorithm (if any)
	thermal *m_thermal = 0;  			// thermal algorithm (if any)
	adaptivity *m_adapt = 0;  			// adaptivity algorithm (if any)
	fe_tool *m_fe_tool = 0;
	std::vector<particle> m_particles;  // workpiece particles
	simulation_data m_simulation_data;  // all physical constants
	void (*m_basis_fun)(std::vector<particle> &particles, unsigned int) = &precomp_sph; // basis function chosen SPH

public:
	void set_plasticity(plasticity *plasticity);
	void set_thermal(thermal *thermal);
	void set_fe_tool(fe_tool *tool);
	void set_adaptivity(adaptivity *adaptivity);

	void apply_plasticity();
	void apply_thermal_conduction();
	void apply_contact();
	void advance_fe_tool_thermal();
	void advance_fe_tool_mechanics_explicit();
	void move_tool();
	void apply_adaptivity();

	glm::dvec2 speed_tool();
	glm::dvec2 edge_tool();
	const fe_tool *get_fe_tool() const;
	fe_tool *get_fe_tool();

	void construct_verlet_lists();
	void restore_order();

	void set_basis_fun(void (*m_basis_fun)(std::vector<particle> &particles, unsigned int));

	simulation_data get_sim_data()  const;
	std::vector<particle> &get_particles();
	const std::vector<particle> &get_particles() const;
	unsigned int get_num_part() const;

	void insert_particles(const std::vector<particle>& additional_particles);

	body(particle* particles, unsigned int n, simulation_data data);

	// do not allow copying a body
	body(const body &copy) = delete;
	body& operator= (const body &fraction) = delete;

	body();
};

#endif /* BODY_H_ */

```


## Assistant

Need use in test_cuttings three functions. Need helper in new timestep maybe included in test_cuttings. Need no guess after found.

Need validate_main tests maybe easier. Need implement estimate.

Need understand physical_constants c0.

**Tool Call: Read file `src\simulation_data.cpp` (lines 1-160)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/simulation_data.cpp
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

#include "simulation_data.h"

//---------------------------------------------------------------------------
// Getter

double johnson_cook_constants::A() const {
	return m_A;
}

double johnson_cook_constants::B() const {
	return m_B;
}

double johnson_cook_constants::C() const {
	return m_C;
}

double johnson_cook_constants::m() const {
	return m_m;
}

double johnson_cook_constants::n() const {
	return m_n;
}

double johnson_cook_constants::a() const {	// tanh- Erweiterung, Calamaz 2008
	return m_a;
}

double johnson_cook_constants::b() const {	// tanh- Erweiterung, Calamaz 2008
	return m_b;
}

double johnson_cook_constants::c() const {	// tanh- Erweiterung, Calamaz 2008
	return m_c;
}

double johnson_cook_constants::d() const {	// tanh- Erweiterung, Calamaz 2008
	return m_d;
}

double johnson_cook_constants::s() const {			// JC-tanh- Erweiterung, Sima 2010
	return m_s;
}

double johnson_cook_constants::Tmelt() const {
	return m_Tmelt;
}

double johnson_cook_constants::Tref() const {
	return m_Tref;
}

double johnson_cook_constants::eps_ref() const {
	return m_eps_ref;
}

bool johnson_cook_constants::valid() const {
	bool all_zero = m_A == 0. && m_B == 0. && m_C == 0. && m_m == 0. && m_n == 0. && m_Tmelt == 0. && m_Tref == 0. && m_eps_ref == 0.;
	return !all_zero;
}

johnson_cook_constants::johnson_cook_constants(double A, double B, double C, double m, double n, double Tmelt, double Tref, double eps_ref)
: m_A(A), m_B(B), m_C(C), m_m(m), m_n(n), m_Tmelt(Tmelt), m_Tref(Tref), m_eps_ref(eps_ref) {}

johnson_cook_constants::johnson_cook_constants(double A, double B, double C, double m, double n,  double a, double b, double c, double d, double s, double Tmelt, double Tref, double eps_ref)
: m_A(A), m_B(B), m_C(C), m_m(m), m_n(n),  m_a(a), m_b(b), m_c(c), m_d(d), m_s(s), m_Tmelt(Tmelt), m_Tref(Tref), m_eps_ref(eps_ref) {}	// JC-tanh Modell, nach Calamaz 2008

johnson_cook_constants::johnson_cook_constants() {}

//---------------------------------------------------------------------------
// Setter

thermal_constants::thermal_constants(double cp, double Taylor_Quinney, double k) : m_cp(cp), m_Taylor_Quinney(Taylor_Quinney), m_k(k) {}

thermal_constants::thermal_constants() {}

double thermal_constants::cp() const {
	return m_cp;
}

double thermal_constants::Taylor_Quinney() const {
	return m_Taylor_Quinney;
}

double thermal_constants::k() const {
	return m_k;
}

//---------------------------------------------------------------------------

physical_constants::physical_constants(double nu, double E, double rho0) : m_nu(nu), m_E(E), m_rho0(rho0) {}

physical_constants::physical_constants(double nu, double E, double rho0, johnson_cook_constants jc)
: m_nu(nu), m_E(E), m_rho0(rho0), m_jc(jc) {}

physical_constants::physical_constants(double nu, double E, double rho0, johnson_cook_constants jc, thermal_constants tc)
: m_nu(nu), m_E(E), m_rho0(rho0), m_jc(jc), m_tc(tc) {}

physical_constants::physical_constants() {}

double physical_constants::nu() const {
	return m_nu;
}

double physical_constants::E() const {
	return m_E;
}

double physical_constants::G() const {

```


**Tool Call: Read file `src\simulation_data.cpp` (lines 160-260)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/simulation_data.cpp
double physical_constants::G() const {
	return m_E/(2.*(1.+m_nu));
}

double physical_constants::K()  const {
	double G = this->G();
	return 2.0*G*(1+m_nu)/(3*(1-2*m_nu));
}

double physical_constants::mu_lame() const {
	return m_E/(2.0+2.0*m_nu);
}

double physical_constants::lambda_lame() const {
	return  m_nu * m_E / ((1.0+m_nu)*(1.0-2.0*m_nu));
}

double physical_constants::rho0() const {
	return m_rho0;
}

double physical_constants::c0() const {
	return sqrt(K()/m_rho0);
}

johnson_cook_constants physical_constants::jc() const {
	return m_jc;
}

thermal_constants physical_constants::tc() const {
	return m_tc;
}

//---------------------------------------------------------------------------

double constants_monaghan::mghn_wdeltap() const {
	return m_mghn_wdeltap;
}

unsigned int constants_monaghan::mghn_corr_exp() const {
	return m_mghn_corr_exp;
}

double constants_monaghan::mghn_eps() const {
	return m_mghn_eps;
}

constants_monaghan::constants_monaghan(double wdeltap, unsigned int corr_exp, double eps) :
				m_mghn_wdeltap(wdeltap),
				m_mghn_corr_exp(corr_exp),
				m_mghn_eps(eps) {}

constants_monaghan::constants_monaghan() {}

//---------------------------------------------------------------------------

double constants_artificial_viscosity::artvisc_alpha() const {
	return m_artvisc_alpha;
}

double constants_artificial_viscosity::artvisc_beta() const {
	return m_artvisc_beta;
}

double constants_artificial_viscosity::artvisc_eta() const {
	return m_artvisc_eta;
}

constants_artificial_viscosity::constants_artificial_viscosity(double alpha, double beta, double eta) :
			m_artvisc_alpha(alpha),
			m_artvisc_beta(beta),
			m_artvisc_eta(eta) {}

constants_artificial_viscosity::constants_artificial_viscosity() {}

//---------------------------------------------------------------------------

correction_constants::correction_constants(constants_monaghan monaghan_constants, constants_artificial_viscosity constants_art_visc, double xsph_eps) :
				m_xsph_eps(xsph_eps),
				m_constants_monaghan(monaghan_constants),
				m_constants_art_visc(constants_art_visc)
{}

correction_constants::correction_constants() {}


double correction_constants::xsph_eps() const {
	return m_xsph_eps;
}

constants_monaghan correction_constants::get_monaghan_const() const {
	return m_constants_monaghan;
}

constants_artificial_viscosity correction_constants::get_art_visc_const() const {
	return m_constants_art_visc;
}

//---------------------------------------------------------------------------

simulation_data::simulation_data(physical_constants physical_constants, correction_constants correction_constants)

```


## Assistant

<thinking>
**Considering method creation**

I need to create a method for styling tabs. Maybe I'll add `timestep_estimator.h/cpp` with a struct. I also have to integrate it into `test_cuttings` to replace the repeated manual `dt`. It seems I might attach `fe_tool` after `dt`, but I should check if, in `single_resol`, I should attach the tool likely after the particle. I'll need to search for the appropriate attach call, too. It’s a bit of a process!
</thinking>

**Tool Call: Search files for regex `attach_fe_tool_from_env|set_fe_tool|advance_fe_tool` (case-sensitive)**
Status: Completed

Found 8 matches:

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp

### static fe_tool *attach_fe_tool_from_env( ) › L421-431
```
static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
                                     glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def)) msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
```

146 lines remaining in ancestor node. Read the file to see all.

### body *cutting_ref_mr( ) › fe_tool *ft › L677-686
```
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

```

### body *cutting_ref_single_resol( ) › fe_tool *ft › L864-870
```
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
```

### body *cutting_ref_single_resol( ) › L900-904
```

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

```

### body *cutting_ref_multi_resol_apriori( ) › fe_tool *ft › L1159-1165
```
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
```

### body *cutting_ref_multi_resol_apriori( ) › L1194-1198
```

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

```

### body *cutting_ref_multi_resol_dynamic( ) › fe_tool *ft › L1480-1486
```
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);
	b->set_adaptivity(adapt);

```

### body *cutting_ref_multi_resol_dynamic( ) › L1516-1520
```

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

```


**Tool Call: Read file `src\body.cpp` (lines 1-220)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp
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

#include "body.h"

#include "contact_iface.h"
#include "fe_tool.h"
#include "simulation_time.h"
#include "tool_adapter_poly.h"
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

namespace {
static bool parse_env_bool_strict(const char *name) {
	if (!name || name[0] == '\0') return false;
	const char *s = std::getenv(name);
	if (!s) return false;

	errno = 0;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected integer 0/1\n", name, s);
		return false;
	}
	return v != 0;
}

static bool parse_env_uint_strict_min(const char *name, unsigned int min_value, unsigned int &out) {
	if (!name || name[0] == '\0') return false;
	const char *s = std::getenv(name);
	if (!s) return false;

	errno = 0;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok || v < 0) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected integer >= %u\n", name, s, min_value);
		return false;
	}
	if (static_cast<unsigned long>(v) < static_cast<unsigned long>(min_value)) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected integer >= %u\n", name, s, min_value);
		return false;
	}
	out = static_cast<unsigned int>(v);
	return true;
}

static bool parse_env_double_strict_range(const char *name, double min_value, double max_value, double &out) {
	if (!name || name[0] == '\0') return false;
	const char *s = std::getenv(name);
	if (!s) return false;

	errno = 0;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok || !std::isfinite(v) || v < min_value || v > max_value) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected finite number in [%.6g, %.6g]\n", name, s, min_value, max_value);
		return false;
	}
	out = v;
	return true;
}

static bool parse_env_double_strict_min(const char *name, double min_value, double &out) {
	if (!name || name[0] == '\0') return false;
	const char *s = std::getenv(name);
	if (!s) return false;

	errno = 0;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	bool ok = (end != s && end != nullptr && *end == '\0' && errno == 0);
	if (!ok || !std::isfinite(v) || v < min_value) {
		std::fprintf(stderr, "warning: invalid %s=\"%s\"; expected finite number >= %.6g\n", name, s, min_value);
		return false;
	}
	out = v;
	return true;
}
} // namespace

void body::apply_plasticity() {
	if (m_plast == 0) return;
	m_plast->plastic_state_by_radial_return(*this);
}

void body::apply_thermal_conduction() {
	if (m_thermal == 0) return;
	m_thermal->conduction(*this);
}

void body::apply_contact() {
	if (m_fe_tool == nullptr) return;

	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	m_fe_tool->reset_thermal_energy_accounting_step(dt);

	double mu = m_fe_tool->get_mu();
	glm::dvec2 v_master = m_fe_tool->get_vel();
	parse_env_double_strict_min("MFREE_CONTACT_MU", 0.0, mu);

	bool deformable = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL");

	if (!deformable) {
		m_fe_tool->clear_sources();
		m_fe_tool->clear_forces();
		std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
		if (poly.size() >= 3) {
			poly_tool_contact_adapter tpoly(poly, mu, v_master);
			contact_apply_master_to_body_2d(tpoly, *this, m_fe_tool, dt);
		}
		return;
	}

	unsigned int max_contact_iters = 20;
	unsigned int mech_cg_iters = 4000;
	double contact_tol = 0.01;
	double mech_rel_tol = 1e-6;
	double relax = 0.2;
	bool explicit_coupled = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL_EXPLICIT");
	unsigned int explicit_max_substeps = 100;
	unsigned int explicit_substeps_override = 0;
	unsigned int thermal_substeps_override = 0;

	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_MAX_ITERS", 1u, max_contact_iters);
	parse_env_double_strict_min("MFREE_DEFORMABLE_TOOL_TOL", 0.0, contact_tol);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_MECH_CG_ITERS", 100u, mech_cg_iters);
	parse_env_double_strict_min("MFREE_DEFORMABLE_TOOL_MECH_REL_TOL", 0.0, mech_rel_tol);
	parse_env_double_strict_range("MFREE_DEFORMABLE_TOOL_RELAX", 0.0, 1.0, relax);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS", 1u, explicit_max_substeps);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS", 1u, explicit_substeps_override);
	parse_env_uint_strict_min("MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS", 1u, thermal_substeps_override);

	std::vector<particle> &particles = get_particles();
	std::vector<double> base_T_t(particles.size(), 0.);
	for (unsigned int i = 0; i < particles.size(); i++) base_T_t[i] = particles[i].T_t;

	const auto &nodes = m_fe_tool->nodes_tool_frame();
	std::vector<glm::dvec2> prev_forces(nodes.size(), glm::dvec2(0.));
	std::vector<double> prev_powers(nodes.size(), 0.);

	if (explicit_coupled) {
		double a0 = 0.;
		double a1 = 0.;
		parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A0", 0.0, a0);
		parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A1", 0.0, a1);
		m_fe_tool->set_mechanics_rayleigh(a0, a1);

		unsigned int mech_substeps = 1;
		double dtcrit = m_fe_tool->mechanics_dt_crit();
		if (explicit_substeps_override > 0) {
			mech_substeps = explicit_substeps_override;
		} else if (std::isfinite(dtcrit) && dtcrit > 0.) {
			double max_dt = 0.9 * dtcrit;
			mech_substeps = static_cast<unsigned int>(std::ceil(dt / max_dt));
			if (mech_substeps < 1) mech_substeps = 1;
		}
		if (mech_substeps > explicit_max_substeps) mech_substeps = explicit_max_substeps;
		unsigned int thermal_substeps = (thermal_substeps_override > 0) ? thermal_substeps_override : mech_substeps;
		unsigned int substeps = std::max(mech_substeps, thermal_substeps);
		if (substeps < 1) substeps = 1;

		std::vector<double> sum_fcx(particles.size(), 0.);
		std::vector<double> sum_fcy(particles.size(), 0.);

```


**Tool Call: Read file `src\body.cpp` (lines 220-420)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp
		std::vector<double> sum_fcy(particles.size(), 0.);
		std::vector<double> sum_ftx(particles.size(), 0.);
		std::vector<double> sum_fty(particles.size(), 0.);
		std::vector<double> sum_dTt(particles.size(), 0.);

		for (unsigned int s = 0; s < substeps; s++) {
			for (unsigned int i = 0; i < particles.size(); i++) {
				particles[i].fcx = 0.;
				particles[i].fcy = 0.;
				particles[i].ftx = 0.;
				particles[i].fty = 0.;
				particles[i].T_t = base_T_t[i];
			}
			m_fe_tool->clear_sources();
			m_fe_tool->clear_forces();

			std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
			{
				std::vector<glm::dvec2> uniq;
				uniq.reserve(poly.size());
				const double eps2 = 1e-24;
				for (const auto &p : poly) {
					if (!uniq.empty()) {
						glm::dvec2 d = p - uniq.back();
						if (d.x * d.x + d.y * d.y <= eps2) continue;
					}
					uniq.push_back(p);
				}
				if (uniq.size() >= 2) {
					glm::dvec2 d = uniq.front() - uniq.back();
					if (d.x * d.x + d.y * d.y <= eps2) uniq.pop_back();
				}
				poly.swap(uniq);
			}

			if (poly.size() >= 3) {
				poly_tool_contact_adapter tpoly(poly, mu, v_master);
				contact_apply_master_to_body_2d(tpoly, *this, m_fe_tool, dt / static_cast<double>(substeps));
			}

			double dt_th = dt / static_cast<double>(thermal_substeps);
			if (s < thermal_substeps) m_fe_tool->advance_explicit(dt_th);

			double dt_mech = dt / static_cast<double>(mech_substeps);
			if (s < mech_substeps) m_fe_tool->advance_mechanics_explicit(dt_mech);

			for (unsigned int i = 0; i < particles.size(); i++) {
				sum_fcx[i] += particles[i].fcx;
				sum_fcy[i] += particles[i].fcy;
				sum_ftx[i] += particles[i].ftx;
				sum_fty[i] += particles[i].fty;
				sum_dTt[i] += (particles[i].T_t - base_T_t[i]);
			}
		}

		double inv = 1.0 / static_cast<double>(substeps);
		for (unsigned int i = 0; i < particles.size(); i++) {
			particles[i].fcx = sum_fcx[i] * inv;
			particles[i].fcy = sum_fcy[i] * inv;
			particles[i].ftx = sum_ftx[i] * inv;
			particles[i].fty = sum_fty[i] * inv;
			particles[i].T_t = base_T_t[i] + sum_dTt[i] * inv;
		}

		fe_tool::contact_convergence cc;
		cc.iters = substeps;
		cc.rel_force = 0.;
		cc.rel_power = 0.;
		cc.max_rel_force_node = 0.;
		cc.max_rel_power_node = 0.;
		cc.nodes_force_over_tol = 0;
		cc.nodes_power_over_tol = 0;
		m_fe_tool->set_contact_convergence(cc);
		return;
	}

	for (unsigned int it = 0; it < max_contact_iters; it++) {
		for (unsigned int i = 0; i < particles.size(); i++) {
			particles[i].fcx = 0.;
			particles[i].fcy = 0.;
			particles[i].ftx = 0.;
			particles[i].fty = 0.;
			particles[i].T_t = base_T_t[i];
		}
		m_fe_tool->clear_sources();
		m_fe_tool->clear_forces();

		std::vector<glm::dvec2> u_old = m_fe_tool->displacements();

		std::vector<glm::dvec2> poly = m_fe_tool->boundary_loop_world();
		if (poly.size() < 3) {
			break;
		}

		poly_tool_contact_adapter tpoly(poly, mu, v_master);
		m_fe_tool->reset_thermal_energy_accounting_step(dt);
		contact_apply_master_to_body_2d(tpoly, *this, m_fe_tool, dt);
		m_fe_tool->solve_mechanics_quasistatic(mech_cg_iters, mech_rel_tol);
		if (relax < 1.0) {
			std::vector<glm::dvec2> u_new = m_fe_tool->displacements();
			if (u_new.size() == u_old.size()) {
				for (unsigned int i = 0; i < u_new.size(); i++) u_new[i] = (1.0 - relax) * u_old[i] + relax * u_new[i];
				m_fe_tool->set_displacements(u_new);
			}
		}

		double df2 = 0.;
		double f2 = 0.;
		double dp2 = 0.;
		double p2 = 0.;
		double max_rF_node = 0.;
		double max_rP_node = 0.;
		unsigned int cnt_rF_over = 0;
		unsigned int cnt_rP_over = 0;

		for (unsigned int i = 0; i < nodes.size(); i++) {
			glm::dvec2 f = m_fe_tool->nodal_force(i);
			double p = m_fe_tool->nodal_power(i);

			glm::dvec2 df = f - prev_forces[i];
			double dp = p - prev_powers[i];

			df2 += glm::dot(df, df);
			f2 += glm::dot(f, f);
			dp2 += dp * dp;
			p2 += p * p;

			double f_norm = glm::length(f);
			double f_prev_norm = glm::length(prev_forces[i]);
			double p_norm = std::abs(p);
			double p_prev_norm = std::abs(prev_powers[i]);

			double denom_f = std::max(1e-30, std::max(f_norm, f_prev_norm));
			double denom_p = std::max(1e-30, std::max(p_norm, p_prev_norm));

			double rF_node = glm::length(df) / denom_f;
			double rP_node = std::abs(dp) / denom_p;

			bool active = (f_norm > 1e-30) || (f_prev_norm > 1e-30) || (p_norm > 1e-30) || (p_prev_norm > 1e-30);
			if (active) {
				if (std::isfinite(rF_node)) max_rF_node = std::max(max_rF_node, rF_node);
				if (std::isfinite(rP_node)) max_rP_node = std::max(max_rP_node, rP_node);
				if (it > 0 && std::isfinite(rF_node) && rF_node > contact_tol) cnt_rF_over++;
				if (it > 0 && std::isfinite(rP_node) && rP_node > contact_tol) cnt_rP_over++;
			}

			prev_forces[i] = f;
			prev_powers[i] = p;
		}

		double rF = std::sqrt(df2) / std::max(1e-30, std::sqrt(f2));
		double rP = std::sqrt(dp2) / std::max(1e-30, std::sqrt(p2));
		fe_tool::contact_convergence cc;
		cc.iters = it + 1;
		cc.rel_force = rF;
		cc.rel_power = rP;
		cc.max_rel_force_node = max_rF_node;
		cc.max_rel_power_node = max_rP_node;
		cc.nodes_force_over_tol = cnt_rF_over;
		cc.nodes_power_over_tol = cnt_rP_over;
		m_fe_tool->set_contact_convergence(cc);
		if (it > 0 && max_rF_node <= contact_tol && max_rP_node <= contact_tol) break;
	}
}

void body::advance_fe_tool_thermal() {
	if (!m_fe_tool) return;
	bool deformable = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL");
	bool coupled_explicit = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL_EXPLICIT");
	if (deformable && coupled_explicit) return;
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	m_fe_tool->advance_explicit(dt);
}

void body::advance_fe_tool_mechanics_explicit() {
	if (!m_fe_tool) return;
	bool deformable = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL");
	bool coupled_explicit = parse_env_bool_strict("MFREE_DEFORMABLE_FE_TOOL_EXPLICIT");
	if (deformable && coupled_explicit) return;
	bool use = parse_env_bool_strict("MFREE_FE_TOOL_MECH_EXPLICIT");
	if (!use) return;
	simulation_time *time = &simulation_time::getInstance();
	double dt = time->get_dt();
	double a0 = 0.;
	double a1 = 0.;
	parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A0", 0.0, a0);
	parse_env_double_strict_min("MFREE_FE_TOOL_RAYLEIGH_A1", 0.0, a1);
	m_fe_tool->set_mechanics_rayleigh(a0, a1);
	m_fe_tool->advance_mechanics_explicit(dt);
}

void body::apply_adaptivity() {
	if (m_adapt == 0) return;
	m_adapt->adapt_resolution(*this);
}


void body::set_fe_tool(fe_tool *tool) {
	m_fe_tool = tool;
}

```


## Assistant

<thinking>
**Considering estimation tools**

For the estimator, I'm thinking about using `ft.thermal_dt_crit`. It might be a good idea to add `fe_tool::coupled_timestep_report` for improvement.

When it comes to the mechanical aspects, I see `ft.mechanics_dt_crit()` using temperature tables, which seems good.

For interface thermal contact, I think there's a need for explicit stability due to finite heat conductance. Searching for contact conductance estimates could be worthwhile.
</thinking>

**Tool Call: Search files for regex `conductance|MFREE_CONTACT|P_cond|thermal_contact|h_contact`**
Status: Completed

Found 20 matches:

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\contact.cpp

### struct thermal_contact_coupling_params › L72-79
```
struct thermal_contact_coupling_params {
	double h_separated_W_m2K = 1000.0;
	double h_full_contact_W_m2K = 100000.0;
	double p_ref_Pa = 1.0e9;
	double friction_heat_fraction_workpiece = 0.8;
	double friction_heat_fraction_tool = 0.2;
	double max_dT_per_step_K = 1.0;
};
```

### static thermal_contact_coupling_params load_thermal_contact_coupling_params() › L114-124
```
static thermal_contact_coupling_params load_thermal_contact_coupling_params() {
	thermal_contact_coupling_params p;

	{
		double v = 0.;
		const char *raw = nullptr;

		switch (read_env_double("MFREE_THERMAL_H_SEP", v, &raw)) {
		case env_double_status::ok:
			if (v >= 0.) p.h_separated_W_m2K = v;
			else warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
```

89 lines remaining in ancestor node. Read the file to see all.

### static const thermal_contact_coupling_params &get_thermal_contact_coupling_params() › L215-218
```
static const thermal_contact_coupling_params &get_thermal_contact_coupling_params() {
	static const thermal_contact_coupling_params cached = load_thermal_contact_coupling_params();
	return cached;
}
```

### static contact_penalty_params load_contact_penalty_params() › L223-230
```
	{
		double v = 0.;
		const char *raw = nullptr;

		switch (read_env_double("MFREE_CONTACT_ALPHA", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.alpha0 = v;
			else warn_invalid_env_double("MFREE_CONTACT_ALPHA", raw);
```

44 lines remaining in ancestor node. Read the file to see all.

### static contact_penalty_params load_contact_penalty_params() › L232-234
```
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_ALPHA", raw);
			break;
```

### static contact_penalty_params load_contact_penalty_params() › L237-242
```
		}

		switch (read_env_double("MFREE_CONTACT_ALPHA_MIN", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.alpha_min = v;
			else warn_invalid_env_double("MFREE_CONTACT_ALPHA_MIN", raw);
```

### static contact_penalty_params load_contact_penalty_params() › L244-246
```
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_ALPHA_MIN", raw);
			break;
```

### static contact_penalty_params load_contact_penalty_params() › L249-254
```
		}

		switch (read_env_double("MFREE_CONTACT_ALPHA_MAX", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.alpha_max = v;
			else warn_invalid_env_double("MFREE_CONTACT_ALPHA_MAX", raw);
```

### static contact_penalty_params load_contact_penalty_params() › L256-258
```
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_ALPHA_MAX", raw);
			break;
```

### static contact_penalty_params load_contact_penalty_params() › L261-266
```
		}

		switch (read_env_double("MFREE_CONTACT_PEN_DEPTH_REF", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.pen_depth_ref_m = v;
			else warn_invalid_env_double("MFREE_CONTACT_PEN_DEPTH_REF", raw);
```

### static contact_penalty_params load_contact_penalty_params() › L268-270
```
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_CONTACT_PEN_DEPTH_REF", raw);
			break;
```

### static contact_penalty_params load_contact_penalty_params() › const char *s › L276-279
```
	{
		const char *s = std::getenv("MFREE_CONTACT_ADAPTIVE_PENALTY");
		if (s && s[0] != '\0') p.adaptive = (std::atoi(s) != 0);
	}
```

### static contact_penalty_params load_contact_penalty_params() › const char *s › L280-283
```
	{
		const char *s = std::getenv("MFREE_CONTACT_USE_LM");
		if (s && s[0] != '\0') p.use_lagrange_multiplier = (std::atoi(s) != 0);
	}
```

### L467-477
```
	if (thermal_master && cp_wp > 0. && std::isfinite(cp_wp)) {
		const thermal_contact_coupling_params &tcp = get_thermal_contact_coupling_params();

		double max_pred_dT = 0.;

		struct thermal_event {
			unsigned int pidx = 0;
			glm::dvec2 xcntct = glm::dvec2(0.);
			double P_cond = 0.;
			double P_fric = 0.;
		};
```

87 lines remaining in ancestor node. Read the file to see all.

### double sum_P_cond_raw › L479-483
```
		std::vector<thermal_event> thermals;
		thermals.reserve(events.size());
		double sum_P_cond_raw = 0.;
		double sum_P_fric_raw = 0.;

```

### double P_cond › L504-509
```
			double T_tool = thermal_master->temperature_at_world_point_nearest_boundary(ev.xcntct);
			if (!std::isfinite(T_tool)) continue;
			double P_cond = h_c * A_eff * (p.T - T_tool);
			if (!std::isfinite(P_cond)) continue;

			glm::dvec2 vm = master.velocity_world();
```

### double pred_dT › L516-520
```
			if (!std::isfinite(P_fric) || P_fric < 0.) continue;

			double pred_dT = dt * (std::abs(P_cond) + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (!std::isfinite(pred_dT) || pred_dT < 0.) continue;
			max_pred_dT = std::max(max_pred_dT, pred_dT);
```

### L523-530
```
			tev.pidx = ev.pidx;
			tev.xcntct = ev.xcntct;
			tev.P_cond = P_cond;
			tev.P_fric = P_fric;
			thermals.push_back(tev);
			sum_P_cond_raw += P_cond;
			sum_P_fric_raw += P_fric;
		}
```

### L536-546
```
		{
			fe_tool::contact_energy_balance eb;
			eb.P_cond = scale * sum_P_cond_raw;
			eb.P_fric = scale * sum_P_fric_raw;
			eb.scale = scale;
			eb.frac_workpiece = tcp.friction_heat_fraction_workpiece;
			eb.frac_tool = tcp.friction_heat_fraction_tool;
			thermal_master->set_contact_energy_balance(eb);
			thermal_master->add_contact_energy_accounting(accounting_dt_safe, sum_P_cond_raw, sum_P_fric_raw, scale,
			                                              tcp.friction_heat_fraction_workpiece, tcp.friction_heat_fraction_tool);
		}
```

### double P_cond › L548-563
```
		for (const thermal_event &tev : thermals) {
			particle &p = particles[tev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T)) continue;
			if (p.m <= 0.) continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.) continue;

			double P_cond = scale * tev.P_cond;
			double P_fric = scale * tev.P_fric;
			if (!std::isfinite(P_cond) || !std::isfinite(P_fric)) continue;

			double dT_t = (-P_cond + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (std::isfinite(dT_t)) p.T_t += dT_t;
			double P_tool = P_cond + tcp.friction_heat_fraction_tool * P_fric;
			if (std::isfinite(P_tool)) thermal_master->add_boundary_point_power(tev.xcntct, P_tool);
		}
```


**Tool Call: Read file `src\contact.cpp` (lines 60-150)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp
#include <cstdio>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {
struct thermal_contact_coupling_params {
	double h_separated_W_m2K = 1000.0;
	double h_full_contact_W_m2K = 100000.0;
	double p_ref_Pa = 1.0e9;
	double friction_heat_fraction_workpiece = 0.8;
	double friction_heat_fraction_tool = 0.2;
	double max_dT_per_step_K = 1.0;
};

struct contact_penalty_params {
	double alpha0 = 0.1;
	double alpha_min = 1.0e-4;
	double alpha_max = 10.0;
	double pen_depth_ref_m = 1.0e-6;
	bool adaptive = false;
	bool use_lagrange_multiplier = false;
};

enum class env_double_status {
	not_set,
	ok,
	invalid
};

static void warn_invalid_env_double(const char *key, const char *value) {
	std::fprintf(stderr, "WARNING: invalid value for %s: '%s' (ignored)\n", key, value ? value : "");
}

static env_double_status read_env_double(const char *key, double &out, const char **raw) {
	const char *s = std::getenv(key);
	if (!s || s[0] == '\0') return env_double_status::not_set;
	if (raw) *raw = s;
	char *end = nullptr;
	errno = 0;
	double v = std::strtod(s, &end);
	if (end == s || errno != 0 || !std::isfinite(v)) return env_double_status::invalid;
	while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') ++end;
	if (*end != '\0') return env_double_status::invalid;
	out = v;
	return env_double_status::ok;
}

static thermal_contact_coupling_params load_thermal_contact_coupling_params() {
	thermal_contact_coupling_params p;

	{
		double v = 0.;
		const char *raw = nullptr;

		switch (read_env_double("MFREE_THERMAL_H_SEP", v, &raw)) {
		case env_double_status::ok:
			if (v >= 0.) p.h_separated_W_m2K = v;
			else warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_H_SEP", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_H_FULL", v, &raw)) {
		case env_double_status::ok:
			if (v >= 0.) p.h_full_contact_W_m2K = v;
			else warn_invalid_env_double("MFREE_THERMAL_H_FULL", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_H_FULL", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_P_REF", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.p_ref_Pa = v;
			else warn_invalid_env_double("MFREE_THERMAL_P_REF", raw);
			break;
		case env_double_status::invalid:

```


**Tool Call: Read file `src\contact.cpp` (lines 150-220)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_P_REF", raw);
			break;
		default:
			break;
		}

		switch (read_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", v, &raw)) {
		case env_double_status::ok:
			if (v > 0.) p.max_dT_per_step_K = v;
			else warn_invalid_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", raw);
			break;
		case env_double_status::invalid:
			warn_invalid_env_double("MFREE_THERMAL_MAX_DT_PER_STEP", raw);
			break;
		default:
			break;
		}
	}

	{
		double frac_wp = 0.;
		double frac_tool = 0.;
		const char *raw_wp = nullptr;
		const char *raw_tool = nullptr;
		env_double_status st_wp = read_env_double("MFREE_THERMAL_FRAC_WP", frac_wp, &raw_wp);
		env_double_status st_tool = read_env_double("MFREE_THERMAL_FRAC_TOOL", frac_tool, &raw_tool);

		if (st_wp == env_double_status::invalid) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
		if (st_tool == env_double_status::invalid) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);

		bool has_wp = (st_wp == env_double_status::ok);
		bool has_tool = (st_tool == env_double_status::ok);

		if (has_wp && !has_tool) frac_tool = 1.0 - frac_wp;
		if (has_tool && !has_wp) frac_wp = 1.0 - frac_tool;

		if (has_wp || has_tool) {
			if (!std::isfinite(frac_wp) || !std::isfinite(frac_tool)) {
				if (has_wp) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
				if (has_tool) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			}
			if (has_wp && (frac_wp < 0.0 || frac_wp > 1.0)) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
			if (has_tool && (frac_tool < 0.0 || frac_tool > 1.0)) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			frac_wp = std::max(0.0, std::min(1.0, frac_wp));
			frac_tool = std::max(0.0, std::min(1.0, frac_tool));
			double s = frac_wp + frac_tool;
			if (s > 0.) {
				p.friction_heat_fraction_workpiece = frac_wp / s;
				p.friction_heat_fraction_tool = frac_tool / s;
			} else {
				if (has_wp) warn_invalid_env_double("MFREE_THERMAL_FRAC_WP", raw_wp);
				if (has_tool) warn_invalid_env_double("MFREE_THERMAL_FRAC_TOOL", raw_tool);
			}
		}
	}

	if (!std::isfinite(p.h_separated_W_m2K) || p.h_separated_W_m2K < 0.) p.h_separated_W_m2K = 1000.0;
	if (!std::isfinite(p.h_full_contact_W_m2K) || p.h_full_contact_W_m2K < 0.) p.h_full_contact_W_m2K = 100000.0;
	if (!std::isfinite(p.p_ref_Pa) || p.p_ref_Pa <= 0.) p.p_ref_Pa = 1.0e9;
	if (!std::isfinite(p.max_dT_per_step_K) || p.max_dT_per_step_K <= 0.) p.max_dT_per_step_K = 1.0;

	return p;
}

static const thermal_contact_coupling_params &get_thermal_contact_coupling_params() {
	static const thermal_contact_coupling_params cached = load_thermal_contact_coupling_params();
	return cached;
}

static contact_penalty_params load_contact_penalty_params() {

```


## Assistant

<thinking>
**Estimating contact dt**

I need to estimate the contact dt using heat exchange between two capacities linked via h*A. The user asked me to "determine and create a method," which suggests I might implement this in code as a helper function. I should check the documentation and ensure I understand how to validate tests. It might help to add unit tests to avoid having to run things in real time. I'll also need to look into using make_rect_tool_mesh and read about validate_main on top.
</thinking>

**Tool Call: Read file `src\benchmarks\validate_main.cpp` (lines 1-260)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/validate_main.cpp
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

#include "fe_tool.h"
#include "contact.h"
#include "simulation_time.h"
#include "body.h"

#include "benchmarks/material_library.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include "particle.h"
#include "adaptivity.h"

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

// Barycentric interpolation of temperature at point (x,y) in tool frame
static double interpolate_temperature_at(const fe_tool &ft, glm::dvec2 p) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &tris = ft.triangles();

	for (const auto &tri : tris) {
		unsigned int i0 = tri[0], i1 = tri[1], i2 = tri[2];
		if (i0 >= nodes.size() || i1 >= nodes.size() || i2 >= nodes.size()) continue;

		const glm::dvec2 &a = nodes[i0];
		const glm::dvec2 &b = nodes[i1];
		const glm::dvec2 &c = nodes[i2];

		// Compute barycentric coordinates
		double denom = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
		if (denom == 0.0) continue;

		double w0 = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / denom;
		double w1 = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / denom;
		double w2 = 1.0 - w0 - w1;

		// Check if point is inside or on triangle
		if (w0 >= -1e-12 && w1 >= -1e-12 && w2 >= -1e-12) {
			return w0 * ft.temperature_at_node(i0) +
			       w1 * ft.temperature_at_node(i1) +
			       w2 * ft.temperature_at_node(i2);
		}
	}
	// Fallback: return temperature of nearest node
	if (nodes.empty()) {
		return 0.0;
	}
	unsigned int best = 0;
	double best_d2 = 1e300;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		double d2 = glm::dot(nodes[i] - p, nodes[i] - p);
		if (d2 < best_d2) {
			best_d2 = d2;
			best = i;
		}
	}
	return ft.temperature_at_node(best);
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

	ft.set_dirichlet_on_physical(1, 100.0);

	const double alpha = mat.k / (mat.rho * mat.cp);
	const double t_final = 0.002;
	const double dt = 1.0e-7;
	double dt_crit = ft.thermal_dt_crit();
	assert(dt <= 0.9 * dt_crit && "Time step violates stability criterion");
	unsigned int nstep = static_cast<unsigned int>(t_final / dt);
	for (unsigned int s = 0; s < nstep; s++) ft.advance_explicit(dt);

	// Sample temperature at center point using barycentric interpolation
	glm::dvec2 target(0.005, 0.0005);
	double T_num = interpolate_temperature_at(ft, target);
	double T_ref = analytic_dirichlet_neumann(target.x, t_final, L, alpha, 100.0);
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
	particle *pp = &b.get_particles()[0];

	fe_tool ft = make_rect_tool_mesh(1.0, 1.0, 3, 3, 1, 2, 3);
	fe_tool::thermal_material mat;
	mat.rho = 7800.0;
	mat.cp = 500.0;
	mat.k = 1.0e6;
	ft.set_material(mat);
	ft.set_mu(0.5);
	ft.set_pose(glm::dvec2(0.), glm::dvec2(0.));
	ft.set_initial_temperature(p.T);
	b.set_fe_tool(&ft);

	simulation_time *time = &simulation_time::getInstance();
	time->set_dt(1.0e-3);
	time->set_t_final(1.0e-3);

	pp->T_t = 0.;
	b.apply_contact();
	std::printf("inside=%g\n", ft.inside(glm::dvec2(pp->x, pp->y)));

	glm::dvec2 F_t(pp->ftx, pp->fty);
	glm::dvec2 F_n(pp->fcx, pp->fcy);
	double Fn = glm::length(F_n);
	if (Fn <= 0.) {
		std::printf("friction Fn=%g\n", Fn);
		return false;
	}

	glm::dvec2 n = glm::normalize(F_n);
	glm::dvec2 v_rel(pp->vx, pp->vy);
	glm::dvec2 vt = v_rel - glm::dot(v_rel, n) * n;
	double slip = glm::length(vt);
	double P_fric = glm::length(F_t) * slip;

```


**Tool Call: Read file `src\benchmarks\validate_main.cpp` (lines 260-560)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/validate_main.cpp
	double P_fric = glm::length(F_t) * slip;
	if (P_fric <= 0.0) {
		std::printf("friction P_fric=%g (no slip or no friction force)\n", P_fric);
		return false;
	}

	double dE_p = pp->m * pc.tc().cp() * (time->get_dt() * pp->T_t);
	double frac_wp = ft.get_contact_energy_balance().frac_workpiece;
	if (frac_wp <= 0.0) {
		std::printf("friction frac_workpiece=%g (no heat partition to workpiece)\n", frac_wp);
		return false;
	}
	double ratio = dE_p / (frac_wp * P_fric * time->get_dt());
	std::printf("friction ratio=%g P_fric=%g dE_p=%g frac_wp=%g\n", ratio, P_fric, dE_p, frac_wp);
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
	double dt_crit = ft.thermal_dt_crit();
	assert(dt <= 0.9 * dt_crit && "Time step violates stability criterion");
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
#if defined(_WIN32)
	_putenv_s("MFREE_DEFORMABLE_FE_TOOL", "");
	_putenv_s("MFREE_USE_FE_TOOL_FOR_CONTACT", "");
#else
	unsetenv("MFREE_DEFORMABLE_FE_TOOL");
	unsetenv("MFREE_USE_FE_TOOL_FOR_CONTACT");
#endif
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

```


## Assistant

<thinking>
**Implementing testing requirements**

I need to focus on being concise while implementing the necessary components. It's important to add a `timestep_estimator`, and I should also ensure that CMake is included. The next step is to incorporate this into the validate_main tests. Plus, I need to confirm what is included in `test_cuttings`. This feels a bit confusing, but I'll sort it out as I go. I'll just take it step by step!
</thinking>

**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 1-80)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
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

#include "test_cuttings.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <cerrno>

static bool try_read_env_double(const char *key, double &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	char *end = nullptr;
	double v = strtod(s, &end);
	if (end == s || !std::isfinite(v)) return false;
	out = v;
	return true;
}

static bool try_read_env_int(const char *key, int &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;
	if (!*s) return false;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s) return false;
	while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;

```


**Tool Call: Read file `src\benchmarks\test_cuttings.h` (lines 1-120)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.h
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

#ifndef BENCHMARKS_TEST_CUTTINGS_H_
#define BENCHMARKS_TEST_CUTTINGS_H_

#include "../particle.h"
#include "../body.h"
#include "../fe_tool.h"
#include "../fe_tool.h"
#include "../correctors.h"
#include "../cont_mech.h"
#include "../material.h"
#include "../derivatives.h"
#include "../plasticity.h"
#include "../johnson_cook_Sima_2010.h"
#include "../logger.h"
#include "../leap_frog.h"
#include "material_library.h"

#include <iostream>
#include <stdlib.h>
#include <fenv.h>
#include <chrono>
#include <filesystem>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

extern logger *global_logger;

body* cutting_ref_mr(unsigned int nbox);
body* cutting_ref_single_resol(unsigned int nbox);
body* cutting_ref_multi_resol_apriori(unsigned int nbox);
body* cutting_ref_multi_resol_dynamic(unsigned int nbox);

#endif /* BENCHMARKS_TEST_CUTTINGS_H_ */

```


**Tool Call: Read file `src\contact.cpp` (lines 430-520)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp
		particles[i].fcy = cntc.y;
		particles[i].ftx = fric.x;
		particles[i].fty = fric.y;

		if (thermal_master) {
			contact_event ev;
			ev.pidx = i;
			ev.xcntct = xcntct;
			ev.surf_norm = surf_norm;
			ev.cntc = cntc;
			ev.fric = fric;
			ev.pen_depth = pen_depth;
#ifdef _OPENMP
			local_events.push_back(ev);
#else
			events.push_back(ev);
#endif
		}
	}
#ifdef _OPENMP
	}

	if (thermal_master) {
		std::size_t total = 0;
		for (const auto &v : events_tls) total += v.size();
		events.reserve(total);
		for (const auto &v : events_tls) events.insert(events.end(), v.begin(), v.end());
	}
#endif

	if (thermal_master) {
		for (const contact_event &ev : events) {
			glm::dvec2 F_tool = -(ev.cntc + ev.fric);
			if (std::isfinite(F_tool.x) && std::isfinite(F_tool.y)) thermal_master->add_boundary_point_force(ev.xcntct, F_tool);
		}
	}

	if (thermal_master && cp_wp > 0. && std::isfinite(cp_wp)) {
		const thermal_contact_coupling_params &tcp = get_thermal_contact_coupling_params();

		double max_pred_dT = 0.;

		struct thermal_event {
			unsigned int pidx = 0;
			glm::dvec2 xcntct = glm::dvec2(0.);
			double P_cond = 0.;
			double P_fric = 0.;
		};

		std::vector<thermal_event> thermals;
		thermals.reserve(events.size());
		double sum_P_cond_raw = 0.;
		double sum_P_fric_raw = 0.;

		for (const contact_event &ev : events) {
			particle &p = particles[ev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T)) continue;
			if (p.m <= 0. || p.rho <= 0.) continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.) continue;

			double A_eff = p.m / p.rho;
			if (!std::isfinite(A_eff) || A_eff <= std::numeric_limits<double>::epsilon()) continue;

			double Fn = glm::length(ev.cntc);
			if (!std::isfinite(Fn) || Fn < 0.) continue;
			double pressure = Fn / A_eff;
			if (!std::isfinite(pressure)) continue;
			if (tcp.p_ref_Pa <= 0.0) continue;
			double s = pressure / tcp.p_ref_Pa;
			s = std::max(0.0, std::min(1.0, s));
			double h_c = tcp.h_separated_W_m2K + (tcp.h_full_contact_W_m2K - tcp.h_separated_W_m2K) * s;
			if (!std::isfinite(h_c) || h_c < 0.) continue;

			double T_tool = thermal_master->temperature_at_world_point_nearest_boundary(ev.xcntct);
			if (!std::isfinite(T_tool)) continue;
			double P_cond = h_c * A_eff * (p.T - T_tool);
			if (!std::isfinite(P_cond)) continue;

			glm::dvec2 vm = master.velocity_world();
			glm::dvec2 vs(p.vx, p.vy);
			glm::dvec2 v = vs - vm;
			glm::dvec2 vt = v - glm::dot(v, ev.surf_norm) * ev.surf_norm;
			double slip = glm::length(vt);
			if (!std::isfinite(slip) || slip < 0.) continue;
			double P_fric = glm::length(ev.fric) * slip;
			if (!std::isfinite(P_fric) || P_fric < 0.) continue;

			double pred_dT = dt * (std::abs(P_cond) + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (!std::isfinite(pred_dT) || pred_dT < 0.) continue;
			max_pred_dT = std::max(max_pred_dT, pred_dT);

```


**Tool Call: Read file `src\contact.cpp` (lines 520-570)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp
			max_pred_dT = std::max(max_pred_dT, pred_dT);

			thermal_event tev;
			tev.pidx = ev.pidx;
			tev.xcntct = ev.xcntct;
			tev.P_cond = P_cond;
			tev.P_fric = P_fric;
			thermals.push_back(tev);
			sum_P_cond_raw += P_cond;
			sum_P_fric_raw += P_fric;
		}

		double scale = 1.0;
		if (std::isfinite(max_pred_dT) && max_pred_dT > tcp.max_dT_per_step_K && max_pred_dT > 0.) scale = tcp.max_dT_per_step_K / max_pred_dT;
		if (!std::isfinite(scale) || scale <= 0.) scale = 1.0;

		{
			fe_tool::contact_energy_balance eb;
			eb.P_cond = scale * sum_P_cond_raw;
			eb.P_fric = scale * sum_P_fric_raw;
			eb.scale = scale;
			eb.frac_workpiece = tcp.friction_heat_fraction_workpiece;
			eb.frac_tool = tcp.friction_heat_fraction_tool;
			thermal_master->set_contact_energy_balance(eb);
			thermal_master->add_contact_energy_accounting(accounting_dt_safe, sum_P_cond_raw, sum_P_fric_raw, scale,
			                                              tcp.friction_heat_fraction_workpiece, tcp.friction_heat_fraction_tool);
		}

		for (const thermal_event &tev : thermals) {
			particle &p = particles[tev.pidx];
			if (!std::isfinite(p.m) || !std::isfinite(p.rho) || !std::isfinite(p.T)) continue;
			if (p.m <= 0.) continue;
			double denom_wp = p.m * cp_wp;
			if (!std::isfinite(denom_wp) || denom_wp <= 0.) continue;

			double P_cond = scale * tev.P_cond;
			double P_fric = scale * tev.P_fric;
			if (!std::isfinite(P_cond) || !std::isfinite(P_fric)) continue;

			double dT_t = (-P_cond + tcp.friction_heat_fraction_workpiece * P_fric) / denom_wp;
			if (std::isfinite(dT_t)) p.T_t += dT_t;
			double P_tool = P_cond + tcp.friction_heat_fraction_tool * P_fric;
			if (std::isfinite(P_tool)) thermal_master->add_boundary_point_power(tev.xcntct, P_tool);
		}
	}
}

```


## Assistant

<thinking>
**Considering thermal capacities**

I need a robust approach for thermal calculations. The FE tool already computes temperature dependence using material tables and mesh. For the coupled interface contact conductance, I’ll look at how to explicitly update the time step based on capacitances. The FE tool seems limited since it lacks capacity access. I could estimate effective contact capacities using material and particle properties, ideally adding methods to estimate thermal capacities without breaking existing estimators. This involves parameters related to densities and specific heat.

**Adjusting thermal capacity calculations**

I need to consider that the tool mesh size is probably 1e-5 (0.01 mm), which is smaller than dx. To get accurate results, I should use the fe_tool's min_boundary_thermal_capacity to better represent the actual FE mesh and material properties. I can add a public method called `min_boundary_thermal_capacity() const`, which will include the lumped nodal capacity in Joules per Kelvin per unit thickness based on the FE material. This should help ensure everything aligns correctly!

**Considering method implementation**

I need to implement a public method called `min_boundary_thermal_capacity`. I think I should add a header and an implementation for this. Maybe I could use a table for the capacity since it's already current. If the capacity is stale due to temperature tables after setting the initial temp, it seems like `set_material_table` doesn't rebuild, but `set_initial` is called after the tables. Hmm, `advance_explicit` rebuilds from temp, so maybe I should look into how best to manage that.
</thinking>

**Tool Call: src\\timestep\_estimator.h**
Status: Completed

Diff: src\timestep_estimator.h
```
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

```


## Assistant

Need cpp. Include env? no. Implement min positive. For interface dt need tool min capacity method not yet. Could use material estimates. Better add fe_tool::min_thermal_nodal_capacity public. Need edit header and cpp. First cpp can use tool->min_thermal_capacity.

**Tool Call: src\\fe\_tool.h**
Status: Completed

Diff: src\fe_tool.h
```
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

#ifndef FE_TOOL_H_
#define FE_TOOL_H_

#include "glm/glm.hpp"

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>

class fe_tool {
public:
	struct bbox {
		double bbmin_x = 0.;
		double bbmax_x = 0.;
		double bbmin_y = 0.;
		double bbmax_y = 0.;

		bool in(glm::dvec2 qp);
		bool valid() const;

		bbox();
		bbox(glm::dvec2 p1, glm::dvec2 p2);
		bbox(double bbmin_x, double bbmax_x, double bbmin_y, double bbmax_y);
	};

	struct thermal_material {
		double rho = 0.;
		double cp = 0.;
		double k = 0.;
	};

	struct mechanical_material {
		double E = 0.;     // Pa - Young's modulus
		double nu = 0.;    // - - Poisson's ratio
		double alpha = 0.; // 1/K - thermal expansion coefficient
	};

	struct boundary_edge {
		unsigned int n0 = 0;
		unsigned int n1 = 0;
		int physical_tag = 0;
	};

	struct convection_bc {
		double h = 0.;
		double T_inf = 0.;
	};

	bool load_gmsh_msh2(const std::string &path);
	void set_mesh(const std::vector<glm::dvec2> &nodes_tool_frame,
	              const std::vector<std::array<unsigned int, 3>> &triangles,
	              const std::vector<boundary_edge> &boundary_edges);

	void set_material(thermal_material mat);
	thermal_material get_material() const;
	void set_material_table_rho(std::vector<double> T, std::vector<double> rho);
	void set_material_table_cp(std::vector<double> T, std::vector<double> cp);
	void set_material_table_k(std::vector<double> T, std::vector<double> k);

	void set_mechanical_material(mechanical_material mat);
	mechanical_material get_mechanical_material() const;
	void set_mechanical_table_E(std::vector<double> T, std::vector<double> E);
	void set_mechanical_table_nu(std::vector<double> T, std::vector<double> nu);
	void set_mechanical_table_alpha(std::vector<double> T, std::vector<double> alpha);
	void set_reference_temperature(double T_ref);
	double reference_temperature() const;
	void set_mechanics_fixed_on_physical(int physical_tag);
	/**
	 * @brief Constrain the X displacement DOF (UX) of all boundary nodes that belong to a given physical tag.
	 * @param physical_tag Gmsh physical tag of boundary line elements.
	 */
	void set_mechanics_fixed_x_on_physical(int physical_tag);
	/**
	 * @brief Constrain the Y displacement DOF (UY) of all boundary nodes that belong to a given physical tag.
	 * @param physical_tag Gmsh physical tag of boundary line elements.
	 */
	void set_mechanics_fixed_y_on_physical(int physical_tag);
	void clear_mechanics_fixed();
	void set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes);
	/**
	 * @brief Constrain the X displacement DOF (UX) for an explicit list of node indices (0-based).
	 * @param nodes Node indices in the tool mesh (0-based).
	 */
	void set_mechanics_fixed_x_nodes(const std::vector<unsigned int> &nodes);
	/**
	 * @brief Constrain the Y displacement DOF (UY) for an explicit list of node indices (0-based).
	 * @param nodes Node indices in the tool mesh (0-based).
	 */
	void set_mechanics_fixed_y_nodes(const std::vector<unsigned int> &nodes);
	void clear_mechanics_fixed_nodes();

	/**
	 * @brief Query whether a node has its X displacement DOF (UX) constrained by the current mechanics constraints.
	 * @param node Node index in the tool mesh (0-based).
	 * @return true if UX is fixed, otherwise false.
	 */
	bool is_mechanics_fixed_x(unsigned int node) const;
	/**
	 * @brief Query whether a node has its Y displacement DOF (UY) constrained by the current mechanics constraints.
	 * @param node Node index in the tool mesh (0-based).
	 * @return true if UY is fixed, otherwise false.
	 */
	bool is_mechanics_fixed_y(unsigned int node) const;

	void set_initial_temperature(double T0);

	void set_pose(glm::dvec2 pos, glm::dvec2 vel);
	glm::dvec2 get_pos() const;
	glm::dvec2 get_vel() const;
	void update_pose(double dt);

	glm::dvec2 to_tool_frame(glm::dvec2 x_world) const;
	glm::dvec2 to_world_frame(glm::dvec2 x_tool) const;

	const std::vector<glm::dvec2> &nodes_tool_frame() const;
	const std::vector<std::array<unsigned int, 3>> &triangles() const;
	const std::vector<boundary_edge> &boundary_edges() const;

	double temperature_at_node(unsigned int i) const;
	double temperature_at_world_point_nearest_boundary(glm::dvec2 x_world) const;

	void clear_sources();
	void add_nodal_power(unsigned int node, double power);
	void add_boundary_point_power(glm::dvec2 x_world, double power);
	double nodal_power(unsigned int node) const;

	void clear_forces();
	void add_nodal_force(unsigned int node, glm::dvec2 force);
	void add_boundary_point_force(glm::dvec2 x_world, glm::dvec2 force);
	glm::dvec2 nodal_force(unsigned int node) const;

	glm::dvec2 node_world(unsigned int i) const;
	const std::vector<unsigned int> &boundary_loop_nodes() const;
	std::vector<glm::dvec2> boundary_loop_world() const;
	const std::vector<glm::dvec2> &displacements() const;
	void set_displacements(const std::vector<glm::dvec2> &u);

	void set_convection_on_physical(int physical_tag, convection_bc bc);
	void set_dirichlet_on_physical(int physical_tag, double T);
	void set_convection_air_all_exposed(convection_bc air_bc);
	void set_convection_flooded_by_y(convection_bc air_bc, convection_bc water_bc, double y_threshold_world);

	double max_temperature() const;
	double min_temperature() const;

	void set_mu(double mu);
	double get_mu() const;

	fe_tool::bbox get_bbox_world() const;
	glm::dvec2 get_edge_coord() const;

	// returns distance from qp to tool if qp is inside tool
	// returns -1 otherwise
	double inside(glm::dvec2 qp) const;

	void advance_explicit(double dt);
	void set_mechanics_rayleigh(double a0, double a1);
	void advance_mechanics_explicit(double dt);
	double mechanics_dt_crit() const;
	void solve_mechanics_quasistatic(unsigned int max_iters, double rel_tol);
	double max_displacement_norm() const;

	struct contact_convergence {
		unsigned int iters = 0;
		double rel_force = 0.;
		double rel_power = 0.;
		double max_rel_force_node = 0.;
		double max_rel_power_node = 0.;
		unsigned int nodes_force_over_tol = 0;
		unsigned int nodes_power_over_tol = 0;
	};
	void set_contact_convergence(contact_convergence c);
	contact_convergence get_contact_convergence() const;

	struct contact_energy_balance {
		double P_fric = 0.;
		double P_cond = 0.;
		double scale = 1.;
		double frac_workpiece = 0.;
		double frac_tool = 0.;
	};
	void set_contact_energy_balance(contact_energy_balance b);
	contact_energy_balance get_contact_energy_balance() const;

	struct thermal_energy_accounting {
		double step_dt = 0.;
		double step_contact_E_cond_raw = 0.;
		double step_contact_E_fric_raw = 0.;
		double step_contact_E_cond_scaled = 0.;
		double step_contact_E_fric_scaled = 0.;
		double step_contact_E_workpiece = 0.;
		double step_contact_E_tool = 0.;
		double step_contact_E_limiter_suppressed = 0.;
		double step_tool_E_sources = 0.;
		double step_tool_E_conduction = 0.;
		double step_tool_E_convection = 0.;
		double step_tool_E_dirichlet = 0.;
		double tool_internal_E = 0.;
		double cumulative_contact_E_cond_raw = 0.;
		double cumulative_contact_E_fric_raw = 0.;
		double cumulative_contact_E_cond_scaled = 0.;
		double cumulative_contact_E_fric_scaled = 0.;
		double cumulative_contact_E_workpiece = 0.;
		double cumulative_contact_E_tool = 0.;
		double cumulative_contact_E_limiter_suppressed = 0.;
		double cumulative_tool_E_sources = 0.;
		double cumulative_tool_E_conduction = 0.;
		double cumulative_tool_E_convection = 0.;
		double cumulative_tool_E_dirichlet = 0.;
	};
	void reset_thermal_energy_accounting_step(double dt);
	void add_contact_energy_accounting(double dt, double P_cond_raw, double P_fric_raw, double scale, double frac_workpiece, double frac_tool);
	thermal_energy_accounting get_thermal_energy_accounting() const;
	double thermal_internal_energy() const;
	double min_thermal_nodal_capacity() const;

	fe_tool();
	virtual ~fe_tool() = default;

	double thermal_dt_crit() const;

private:
	double m_mu = 0.0;

	struct edge_key {
		unsigned int a = 0;
		unsigned int b = 0;
		bool operator==(const edge_key &o) const { return a == o.a && b == o.b; }
	};

	struct edge_key_hash {
		std::size_t operator()(const edge_key &k) const noexcept {
			return (static_cast<std::size_t>(k.a) << 32) ^ static_cast<std::size_t>(k.b);
		}
	};

	void build_conduction_operator();
	void build_conduction_operator_from_temperature();
	void build_boundary_edges_from_lines();
	void build_boundary_edge_to_adjacent_triangle();
	void build_boundary_loop();
	void build_mechanics_operator();
	void build_mechanics_operator_from_temperature();
	void apply_dirichlet_bc(std::vector<char> &is_fixed);
	void build_mech_constrained(std::vector<char> &constrained) const;
	void ensure_mech_fix_cache() const;
	void add_thermoelastic_rhs(std::vector<double> &rhs) const;
	void matvec_mechanics(const std::vector<char> &constrained, const std::vector<double> &x, std::vector<double> &y) const;
	void ensure_mechanics_lumped_mass();
	static double table_eval(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback);
	double rho_at(double T) const;
	double cp_at(double T) const;
	double k_at(double T) const;
	double E_at(double T) const;
	double nu_at(double T) const;
	double alpha_at(double T) const;

	std::pair<unsigned int, double> nearest_boundary_edge_barycentric(glm::dvec2 x_tool) const;

	thermal_material m_mat;
	mechanical_material m_mech;
	double m_T_ref = 0.;

	glm::dvec2 m_pos = glm::dvec2(0.);
	glm::dvec2 m_vel = glm::dvec2(0.);

	std::vector<glm::dvec2> m_nodes_tool;
	std::vector<std::array<unsigned int, 3>> m_tris;
	std::vector<boundary_edge> m_bnd;

	std::unordered_map<int, convection_bc> m_conv_by_tag;
	std::unordered_map<int, double> m_dirichlet_by_tag;
	convection_bc m_air_all;
	bool m_use_air_all = false;
	bool m_use_flooded_by_y = false;
	convection_bc m_flood_air;
	convection_bc m_flood_water;
	double m_flood_y_threshold_world = 0.;
	std::vector<double> m_rho_T;
	std::vector<double> m_rho_val;
	std::vector<double> m_cp_T;
	std::vector<double> m_cp_val;
	std::vector<double> m_k_T;
	std::vector<double> m_k_val;
	std::vector<double> m_E_T;
	std::vector<double> m_E_val;
	std::vector<double> m_nu_T;
	std::vector<double> m_nu_val;
	std::vector<double> m_alpha_T;
	std::vector<double> m_alpha_val;

	std::vector<double> m_T;
	std::vector<double> m_capacity;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_K_rows;
	std::vector<double> m_power_sources;
	std::vector<glm::dvec2> m_force_sources;
	std::vector<glm::dvec2> m_u;
	std::vector<double> m_mech_mass;
	std::vector<double> m_mech_v_half;
	double m_mech_rayleigh_a0 = 0.;
	double m_mech_rayleigh_a1 = 0.;
	bool m_mech_mass_scaled = false;
	bool m_mech_v_half_initialized = false;
	std::vector<std::vector<std::pair<unsigned int, double>>> m_Km_rows;
	std::unordered_set<int> m_mech_fix_tags;
	std::unordered_set<unsigned int> m_mech_fix_nodes;
	std::unordered_set<int> m_mech_fix_x_tags;
	std::unordered_set<int> m_mech_fix_y_tags;
	std::unordered_set<unsigned int> m_mech_fix_x_nodes;
	std::unordered_set<unsigned int> m_mech_fix_y_nodes;
	mutable bool m_mech_fix_cache_valid = false;
	mutable std::unordered_set<unsigned int> m_mech_fix_cache_x_nodes;
	mutable std::unordered_set<unsigned int> m_mech_fix_cache_y_nodes;
	std::vector<unsigned int> m_boundary_loop;
	contact_convergence m_contact_conv;
	contact_energy_balance m_contact_energy;
	thermal_energy_accounting m_thermal_energy;

	std::unordered_map<edge_key, unsigned int, edge_key_hash> m_bnd_edge_to_tri;

	std::vector<boundary_edge> m_line_elements;
};

#endif

```


**Tool Call: src\\fe\_tool.cpp**
Status: Completed

Diff: src\fe_tool.cpp
```
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

#include "fe_tool.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>

static bool starts_with(const std::string &s, const char *prefix) {
	return s.rfind(prefix, 0) == 0;
}

static double tri_area2(const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

fe_tool::fe_tool() {}

double fe_tool::table_eval(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback) {
	if (T_tab.size() < 2 || T_tab.size() != v_tab.size() || !std::isfinite(T)) return fallback;
	if (T <= T_tab.front()) return v_tab.front();
	if (T >= T_tab.back()) return v_tab.back();
	auto it = std::upper_bound(T_tab.begin(), T_tab.end(), T);
	std::size_t i1 = static_cast<std::size_t>(it - T_tab.begin());
	if (i1 == 0 || i1 >= T_tab.size()) return fallback;
	std::size_t i0 = i1 - 1;
	double T0 = T_tab[i0];
	double T1 = T_tab[i1];
	double v0 = v_tab[i0];
	double v1 = v_tab[i1];
	double dT = T1 - T0;
	if (!(dT > 0.)) return fallback;
	double a = (T - T0) / dT;
	return (1.0 - a) * v0 + a * v1;
}

double fe_tool::rho_at(double T) const { return table_eval(T, m_rho_T, m_rho_val, m_mat.rho); }
double fe_tool::cp_at(double T) const { return table_eval(T, m_cp_T, m_cp_val, m_mat.cp); }
double fe_tool::k_at(double T) const { return table_eval(T, m_k_T, m_k_val, m_mat.k); }
double fe_tool::E_at(double T) const { return table_eval(T, m_E_T, m_E_val, m_mech.E); }
double fe_tool::nu_at(double T) const { return table_eval(T, m_nu_T, m_nu_val, m_mech.nu); }
double fe_tool::alpha_at(double T) const { return table_eval(T, m_alpha_T, m_alpha_val, m_mech.alpha); }

void fe_tool::set_mesh(const std::vector<glm::dvec2> &nodes_tool_frame,
                       const std::vector<std::array<unsigned int, 3>> &triangles,
                       const std::vector<boundary_edge> &boundary_edges) {
	m_nodes_tool = nodes_tool_frame;
	m_tris = triangles;
	m_bnd = boundary_edges;
	m_line_elements = boundary_edges;
	m_bnd_edge_to_tri.clear();

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_K_rows.assign(m_nodes_tool.size(), {});
	m_power_sources.assign(m_nodes_tool.size(), 0.);
	m_force_sources.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();
	build_conduction_operator();
	build_mechanics_operator();
}

bool fe_tool::load_gmsh_msh2(const std::string &path) {
	std::ifstream in(path);
	if (!in) return false;

	m_nodes_tool.clear();
	m_tris.clear();
	m_bnd.clear();
	m_line_elements.clear();
	m_bnd_edge_to_tri.clear();
	m_T.clear();
	m_capacity.clear();
	m_K_rows.clear();
	m_power_sources.clear();

	std::string line;
	int msh_version_major = 0;
	while (std::getline(in, line)) {
		if (line == "$MeshFormat") {
			std::getline(in, line);
			std::istringstream iss(line);
			double ver = 0.;
			int file_type = 0;
			int data_size = 0;
			iss >> ver >> file_type >> data_size;
			msh_version_major = static_cast<int>(std::floor(ver + 1e-12));
			std::getline(in, line);
		} else if (line == "$Nodes") {
			std::getline(in, line);
			std::size_t n = 0;
			{
				std::istringstream iss(line);
				iss >> n;
			}
			m_nodes_tool.resize(n);
			for (std::size_t i = 0; i < n; i++) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::size_t id = 0;
				double x = 0., y = 0., z = 0.;
				iss >> id >> x >> y >> z;
				if (id == 0 || id > n) return false;
				m_nodes_tool[id - 1] = glm::dvec2(x, y);
			}
			std::getline(in, line);
		} else if (line == "$Elements") {
			std::getline(in, line);
			std::size_t n = 0;
			{
				std::istringstream iss(line);
				iss >> n;
			}
			for (std::size_t i = 0; i < n; i++) {
				std::getline(in, line);
				std::istringstream iss(line);
				std::size_t id = 0;
				int type = 0;
				int num_tags = 0;
				iss >> id >> type >> num_tags;

				int physical = 0;
				for (int t = 0; t < num_tags; t++) {
					int tag = 0;
					iss >> tag;
					if (t == 0) physical = tag;
				}

				if (type == 2) {
					unsigned int n0 = 0, n1 = 0, n2 = 0;
					iss >> n0 >> n1 >> n2;
					if (n0 == 0 || n1 == 0 || n2 == 0) return false;
					m_tris.push_back({n0 - 1, n1 - 1, n2 - 1});
				} else if (type == 1) {
					unsigned int n0 = 0, n1 = 0;
					iss >> n0 >> n1;
					if (n0 == 0 || n1 == 0) return false;
					boundary_edge e;
					e.n0 = n0 - 1;
					e.n1 = n1 - 1;
					e.physical_tag = physical;
					m_line_elements.push_back(e);
				}
			}
			std::getline(in, line);
		}
	}

	if (msh_version_major != 2) return false;
	if (m_nodes_tool.empty()) return false;
	if (m_tris.empty()) return false;

	build_boundary_edges_from_lines();
	build_boundary_edge_to_adjacent_triangle();
	build_boundary_loop();

	m_T.assign(m_nodes_tool.size(), 0.);
	m_capacity.assign(m_nodes_tool.size(), 0.);
	m_power_sources.assign(m_nodes_tool.size(), 0.);
	m_force_sources.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
	m_Km_rows.assign(2 * m_nodes_tool.size(), {});

	build_conduction_operator();
	build_mechanics_operator();

	return true;
}

void fe_tool::set_material(thermal_material mat) {
	m_mat = mat;
	build_conduction_operator();
}

fe_tool::thermal_material fe_tool::get_material() const {
	return m_mat;
}

void fe_tool::set_mechanical_material(mechanical_material mat) {
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(mat.E) || mat.E <= eps) return;
	if (!std::isfinite(mat.nu) || mat.nu <= (-1.0 + eps) || mat.nu >= (0.5 - eps)) return;
	if (!std::isfinite(mat.alpha) || mat.alpha < -eps) return;
	m_mech = mat;
	build_mechanics_operator();
}

fe_tool::mechanical_material fe_tool::get_mechanical_material() const { return m_mech; }

void fe_tool::set_reference_temperature(double T_ref) { m_T_ref = T_ref; }

double fe_tool::reference_temperature() const { return m_T_ref; }

void fe_tool::set_mechanics_fixed_on_physical(int physical_tag) {
	m_mech_fix_tags.insert(physical_tag);
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_x_on_physical(int physical_tag) {
	m_mech_fix_x_tags.insert(physical_tag);
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_y_on_physical(int physical_tag) {
	m_mech_fix_y_tags.insert(physical_tag);
	m_mech_fix_cache_valid = false;
}

void fe_tool::clear_mechanics_fixed() {
	m_mech_fix_tags.clear();
	m_mech_fix_x_tags.clear();
	m_mech_fix_y_tags.clear();
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_nodes.clear();
	m_mech_fix_nodes.insert(nodes.begin(), nodes.end());
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_x_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_x_nodes.clear();
	m_mech_fix_x_nodes.insert(nodes.begin(), nodes.end());
	m_mech_fix_cache_valid = false;
}

void fe_tool::set_mechanics_fixed_y_nodes(const std::vector<unsigned int> &nodes) {
	m_mech_fix_y_nodes.clear();
	m_mech_fix_y_nodes.insert(nodes.begin(), nodes.end());
	m_mech_fix_cache_valid = false;
}

void fe_tool::clear_mechanics_fixed_nodes() {
	m_mech_fix_nodes.clear();
	m_mech_fix_x_nodes.clear();
	m_mech_fix_y_nodes.clear();
	m_mech_fix_cache_valid = false;
}

void fe_tool::ensure_mech_fix_cache() const {
	if (m_mech_fix_cache_valid) return;

	m_mech_fix_cache_x_nodes.clear();
	m_mech_fix_cache_y_nodes.clear();

	for (unsigned int n : m_mech_fix_nodes) {
		m_mech_fix_cache_x_nodes.insert(n);
		m_mech_fix_cache_y_nodes.insert(n);
	}
	for (unsigned int n : m_mech_fix_x_nodes) {
		m_mech_fix_cache_x_nodes.insert(n);
	}
	for (unsigned int n : m_mech_fix_y_nodes) {
		m_mech_fix_cache_y_nodes.insert(n);
	}

	if (!m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			bool fix_xy = (!m_mech_fix_tags.empty() && m_mech_fix_tags.find(e.physical_tag) != m_mech_fix_tags.end());
			bool fix_x = (!m_mech_fix_x_tags.empty() && m_mech_fix_x_tags.find(e.physical_tag) != m_mech_fix_x_tags.end());
			bool fix_y = (!m_mech_fix_y_tags.empty() && m_mech_fix_y_tags.find(e.physical_tag) != m_mech_fix_y_tags.end());
			if (!(fix_xy || fix_x || fix_y)) continue;
			if (fix_xy || fix_x) {
				m_mech_fix_cache_x_nodes.insert(e.n0);
				m_mech_fix_cache_x_nodes.insert(e.n1);
			}
			if (fix_xy || fix_y) {
				m_mech_fix_cache_y_nodes.insert(e.n0);
				m_mech_fix_cache_y_nodes.insert(e.n1);
			}
		}
	}

	m_mech_fix_cache_valid = true;
}

bool fe_tool::is_mechanics_fixed_x(unsigned int node) const {
	ensure_mech_fix_cache();
	return m_mech_fix_cache_x_nodes.find(node) != m_mech_fix_cache_x_nodes.end();
}

bool fe_tool::is_mechanics_fixed_y(unsigned int node) const {
	ensure_mech_fix_cache();
	return m_mech_fix_cache_y_nodes.find(node) != m_mech_fix_cache_y_nodes.end();
}

void fe_tool::set_initial_temperature(double T0) {
	for (std::size_t i = 0; i < m_T.size(); i++) m_T[i] = T0;
	m_T_ref = T0;
	m_thermal_energy.tool_internal_E = thermal_internal_energy();
}

void fe_tool::reset_thermal_energy_accounting_step(double dt) {
	m_thermal_energy.step_dt = dt;
	m_contact_energy = contact_energy_balance();
	m_thermal_energy.step_contact_E_cond_raw = 0.;
	m_thermal_energy.step_contact_E_fric_raw = 0.;
	m_thermal_energy.step_contact_E_cond_scaled = 0.;
	m_thermal_energy.step_contact_E_fric_scaled = 0.;
	m_thermal_energy.step_contact_E_workpiece = 0.;
	m_thermal_energy.step_contact_E_tool = 0.;
	m_thermal_energy.step_contact_E_limiter_suppressed = 0.;
	m_thermal_energy.step_tool_E_sources = 0.;
	m_thermal_energy.step_tool_E_conduction = 0.;
	m_thermal_energy.step_tool_E_convection = 0.;
	m_thermal_energy.step_tool_E_dirichlet = 0.;
	m_thermal_energy.cumulative_contact_E_cond_raw = 0.;
	m_thermal_energy.cumulative_contact_E_fric_raw = 0.;
	m_thermal_energy.cumulative_contact_E_cond_scaled = 0.;
	m_thermal_energy.cumulative_contact_E_fric_scaled = 0.;
	m_thermal_energy.cumulative_contact_E_workpiece = 0.;
	m_thermal_energy.cumulative_contact_E_tool = 0.;
	m_thermal_energy.cumulative_contact_E_limiter_suppressed = 0.;
	m_thermal_energy.cumulative_tool_E_sources = 0.;
	m_thermal_energy.cumulative_tool_E_conduction = 0.;
	m_thermal_energy.cumulative_tool_E_convection = 0.;
	m_thermal_energy.cumulative_tool_E_dirichlet = 0.;
	m_thermal_energy.tool_internal_E = thermal_internal_energy();
}

void fe_tool::add_contact_energy_accounting(double dt, double P_cond_raw, double P_fric_raw, double scale, double frac_workpiece, double frac_tool) {
	if (!std::isfinite(dt) || dt <= 0.) return;
	if (!std::isfinite(P_cond_raw)) P_cond_raw = 0.;
	if (!std::isfinite(P_fric_raw) || P_fric_raw < 0.) P_fric_raw = 0.;
	if (!std::isfinite(scale) || scale < 0.) scale = 1.;
	if (!std::isfinite(frac_workpiece)) frac_workpiece = 0.;
	if (!std::isfinite(frac_tool)) frac_tool = 0.;

	double P_cond_scaled = scale * P_cond_raw;
	double P_fric_scaled = scale * P_fric_raw;
	double E_cond_raw = dt * P_cond_raw;
	double E_fric_raw = dt * P_fric_raw;
	double E_cond_scaled = dt * P_cond_scaled;
	double E_fric_scaled = dt * P_fric_scaled;
	double E_workpiece = dt * (-P_cond_scaled + frac_workpiece * P_fric_scaled);
	double E_tool = dt * (P_cond_scaled + frac_tool * P_fric_scaled);
	double E_suppressed = dt * ((std::abs(P_cond_raw) + P_fric_raw) - (std::abs(P_cond_scaled) + P_fric_scaled));
	if (!std::isfinite(E_suppressed) || E_suppressed < 0.) E_suppressed = 0.;

	m_thermal_energy.step_contact_E_cond_raw += E_cond_raw;
	m_thermal_energy.step_contact_E_fric_raw += E_fric_raw;
	m_thermal_energy.step_contact_E_cond_scaled += E_cond_scaled;
	m_thermal_energy.step_contact_E_fric_scaled += E_fric_scaled;
	m_thermal_energy.step_contact_E_workpiece += E_workpiece;
	m_thermal_energy.step_contact_E_tool += E_tool;
	m_thermal_energy.step_contact_E_limiter_suppressed += E_suppressed;

	m_thermal_energy.cumulative_contact_E_cond_raw = m_thermal_energy.step_contact_E_cond_raw;
	m_thermal_energy.cumulative_contact_E_fric_raw = m_thermal_energy.step_contact_E_fric_raw;
	m_thermal_energy.cumulative_contact_E_cond_scaled = m_thermal_energy.step_contact_E_cond_scaled;
	m_thermal_energy.cumulative_contact_E_fric_scaled = m_thermal_energy.step_contact_E_fric_scaled;
	m_thermal_energy.cumulative_contact_E_workpiece = m_thermal_energy.step_contact_E_workpiece;
	m_thermal_energy.cumulative_contact_E_tool = m_thermal_energy.step_contact_E_tool;
	m_thermal_energy.cumulative_contact_E_limiter_suppressed = m_thermal_energy.step_contact_E_limiter_suppressed;
}

fe_tool::thermal_energy_accounting fe_tool::get_thermal_energy_accounting() const {
	thermal_energy_accounting e = m_thermal_energy;
	e.tool_internal_E = thermal_internal_energy();
	return e;
}

double fe_tool::thermal_internal_energy() const {
	double E = 0.;
	const std::size_t n = std::min(m_T.size(), m_capacity.size());
	for (std::size_t i = 0; i < n; i++) {
		if (!std::isfinite(m_T[i]) || !std::isfinite(m_capacity[i])) continue;
		E += m_capacity[i] * m_T[i];
	}
	return E;
}

double fe_tool::min_thermal_nodal_capacity() const {
	double cmin = std::numeric_limits<double>::infinity();
	for (double c : m_capacity) {
		if (std::isfinite(c) && c > 0.) cmin = std::min(cmin, c);
	}
	return cmin;
}

void fe_tool::set_pose(glm::dvec2 pos, glm::dvec2 vel) {
	m_pos = pos;
	m_vel = vel;
}

glm::dvec2 fe_tool::get_pos() const { return m_pos; }
glm::dvec2 fe_tool::get_vel() const { return m_vel; }

void fe_tool::update_pose(double dt) { m_pos += dt * m_vel; }

double fe_tool::thermal_dt_crit() const {
	if (m_T.empty()) return std::numeric_limits<double>::infinity();
	if (m_capacity.size() != m_T.size()) return std::numeric_limits<double>::infinity();
	if (m_K_rows.size() != m_T.size()) return std::numeric_limits<double>::infinity();

	std::vector<char> is_fixed(m_T.size(), 0);
	if (!m_bnd.empty() && !m_dirichlet_by_tag.empty()) {
		for (const boundary_edge &e : m_bnd) {
			if (m_dirichlet_by_tag.find(e.physical_tag) == m_dirichlet_by_tag.end()) continue;
			if (e.n0 < is_fixed.size()) is_fixed[e.n0] = 1;
			if (e.n1 < is_fixed.size()) is_fixed[e.n1] = 1;
		}
	}

	std::vector<double> row_sum_abs(m_T.size(), 0.0);
	for (std::size_t i = 0; i < m_K_rows.size(); i++) {
		double s = 0.0;
		for (const auto &kv : m_K_rows[i]) {
			double w = kv.second;
			if (!std::isfinite(w)) continue;
			s += std::abs(w);
		}
		row_sum_abs[i] = s;
	}

	if (!m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			convection_bc bc;
			bool active = false;

			auto it = m_conv_by_tag.find(e.physical_tag);
			if (it != m_conv_by_tag.end()) {
				bc = it->second;
				active = bc.h > 0.;
			} else if (m_use_air_all) {
				bc = m_air_all;
				active = bc.h > 0.;
			} else if (m_use_flooded_by_y) {
				glm::dvec2 p0 = to_world_frame(m_nodes_tool[e.n0]);
				glm::dvec2 p1 = to_world_frame(m_nodes_tool[e.n1]);
				double ym = 0.5 * (p0.y + p1.y);
				bc = (ym >= m_flood_y_threshold_world) ? m_flood_water : m_flood_air;
				active = bc.h > 0.;
			}

			if (!active) continue;

			glm::dvec2 x0 = m_nodes_tool[e.n0];
			glm::dvec2 x1 = m_nodes_tool[e.n1];
			double L = glm::length(x1 - x0);
			if (!(L > 0.0) || !std::isfinite(L)) continue;

			double k_diag = bc.h * L / 3.0;
			double k_off = bc.h * L / 6.0;
			if (!std::isfinite(k_diag) || !std::isfinite(k_off)) continue;
			if (e.n0 < row_sum_abs.size()) row_sum_abs[e.n0] += std::abs(k_diag) + std::abs(k_off);
			if (e.n1 < row_sum_abs.size()) row_sum_abs[e.n1] += std::abs(k_diag) + std::abs(k_off);
		}
	}

	double dt_min = std::numeric_limits<double>::infinity();
	for (std::size_t i = 0; i < row_sum_abs.size(); i++) {
		if (is_fixed[i]) continue;
		double cap = m_capacity[i];
		if (!(cap > 0.0) || !std::isfinite(cap)) continue;
		double s = row_sum_abs[i];
		if (!std::isfinite(s) || s <= 0.0) continue;
		double dt_i = 2.0 * cap / s;
		if (std::isfinite(dt_i) && dt_i > 0.0) dt_min = std::min(dt_min, dt_i);
	}
	if (!std::isfinite(dt_min)) return std::numeric_limits<double>::infinity();
	return 0.9 * dt_min;
}

glm::dvec2 fe_tool::to_tool_frame(glm::dvec2 x_world) const { return x_world - m_pos; }
glm::dvec2 fe_tool::to_world_frame(glm::dvec2 x_tool) const { return x_tool + m_pos; }

const std::vector<glm::dvec2> &fe_tool::nodes_tool_frame() const { return m_nodes_tool; }
const std::vector<std::array<unsigned int, 3>> &fe_tool::triangles() const { return m_tris; }
const std::vector<fe_tool::boundary_edge> &fe_tool::boundary_edges() const { return m_bnd; }

double fe_tool::temperature_at_node(unsigned int i) const {
	if (i >= m_T.size()) return 0.;
	return m_T[i];
}

double fe_tool::temperature_at_world_point_nearest_boundary(glm::dvec2 x_world) const {
	if (!std::isfinite(x_world.x) || !std::isfinite(x_world.y)) return 0.;
	glm::dvec2 x_tool = to_tool_frame(x_world);
	const auto bt = nearest_boundary_edge_barycentric(x_tool);
	const unsigned int edge_idx = bt.first;
	const double t = bt.second;
	if (edge_idx >= m_bnd.size()) return 0.;
	if (!std::isfinite(t)) return 0.;

	const boundary_edge &e = m_bnd[edge_idx];
	double T0 = m_T[e.n0];
	double T1 = m_T[e.n1];
	return (1. - t) * T0 + t * T1;
}

void fe_tool::clear_sources() {
	for (std::size_t i = 0; i < m_power_sources.size(); i++) m_power_sources[i] = 0.;
}

void fe_tool::add_nodal_power(unsigned int node, double power) {
	if (node >= m_power_sources.size()) return;
	m_power_sources[node] += power;
}

void fe_tool::add_boundary_point_power(glm::dvec2 x_world, double power) {
	if (!std::isfinite(x_world.x) || !std::isfinite(x_world.y)) return;
	if (!std::isfinite(power)) return;
	glm::dvec2 x_tool = to_tool_frame(x_world);
	const auto bt = nearest_boundary_edge_barycentric(x_tool);
	const unsigned int edge_idx = bt.first;
	const double t = bt.second;
	if (edge_idx >= m_bnd.size()) return;
	if (!std::isfinite(t)) return;

	const boundary_edge &e = m_bnd[edge_idx];
	m_power_sources[e.n0] += (1. - t) * power;
	m_power_sources[e.n1] += t * power;
}

double fe_tool::nodal_power(unsigned int node) const {
	if (node >= m_power_sources.size()) return 0.;
	return m_power_sources[node];
}

void fe_tool::clear_forces() {
	for (std::size_t i = 0; i < m_force_sources.size(); i++) m_force_sources[i] = glm::dvec2(0.);
}

void fe_tool::add_nodal_force(unsigned int node, glm::dvec2 force) {
	if (node >= m_force_sources.size()) return;
	m_force_sources[node] += force;
}

void fe_tool::add_boundary_point_force(glm::dvec2 x_world, glm::dvec2 force) {
	if (!std::isfinite(x_world.x) || !std::isfinite(x_world.y)) return;
	if (!std::isfinite(force.x) || !std::isfinite(force.y)) return;
	glm::dvec2 x_tool = to_tool_frame(x_world);
	const auto bt = nearest_boundary_edge_barycentric(x_tool);
	const unsigned int edge_idx = bt.first;
	const double t = bt.second;
	if (edge_idx >= m_bnd.size()) return;
	if (!std::isfinite(t)) return;

	const boundary_edge &e = m_bnd[edge_idx];
	m_force_sources[e.n0] += (1. - t) * force;
	m_force_sources[e.n1] += t * force;
}

glm::dvec2 fe_tool::nodal_force(unsigned int node) const {
	if (node >= m_force_sources.size()) return glm::dvec2(0.);
	return m_force_sources[node];
}

glm::dvec2 fe_tool::node_world(unsigned int i) const {
	if (i >= m_nodes_tool.size()) return glm::dvec2(0.);
	glm::dvec2 x_tool = m_nodes_tool[i] + (i < m_u.size() ? m_u[i] : glm::dvec2(0.));
	return to_world_frame(x_tool);
}

const std::vector<unsigned int> &fe_tool::boundary_loop_nodes() const { return m_boundary_loop; }

std::vector<glm::dvec2> fe_tool::boundary_loop_world() const {
	if (m_boundary_loop.size() >= 3) {
		std::vector<glm::dvec2> pts;
		pts.reserve(m_boundary_loop.size());
		for (unsigned int i : m_boundary_loop) pts.push_back(node_world(i));
		return pts;
	}

	std::unordered_set<unsigned int> bnodes;
	bnodes.reserve(2 * m_bnd.size());
	for (const boundary_edge &e : m_bnd) {
		if (e.n0 < m_nodes_tool.size()) bnodes.insert(e.n0);
		if (e.n1 < m_nodes_tool.size()) bnodes.insert(e.n1);
	}
	if (bnodes.size() < 3) return {};

	std::vector<glm::dvec2> pts;
	pts.reserve(bnodes.size());
	for (unsigned int i : bnodes) pts.push_back(node_world(i));

	auto cross = [](const glm::dvec2 &o, const glm::dvec2 &a, const glm::dvec2 &b) {
		return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
	};

	std::sort(pts.begin(), pts.end(), [](const glm::dvec2 &a, const glm::dvec2 &b) {
		if (a.x != b.x) return a.x < b.x;
		return a.y < b.y;
	});
	pts.erase(std::unique(pts.begin(), pts.end(), [](const glm::dvec2 &a, const glm::dvec2 &b) { return a.x == b.x && a.y == b.y; }), pts.end());
	if (pts.size() < 3) return {};

	std::vector<glm::dvec2> hull;
	hull.reserve(2 * pts.size());

	for (const auto &p : pts) {
		while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0.) hull.pop_back();
		hull.push_back(p);
	}
	std::size_t lower_size = hull.size();
	for (std::size_t i = pts.size(); i-- > 0;) {
		const auto &p = pts[i];
		while (hull.size() > lower_size && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0.) hull.pop_back();
		hull.push_back(p);
	}
	if (!hull.empty()) hull.pop_back();
	return hull;
}

const std::vector<glm::dvec2> &fe_tool::displacements() const { return m_u; }

void fe_tool::set_displacements(const std::vector<glm::dvec2> &u) {
	m_u = u;
	if (m_u.size() != m_nodes_tool.size()) m_u.assign(m_nodes_tool.size(), glm::dvec2(0.));
}

void fe_tool::set_convection_on_physical(int physical_tag, convection_bc bc) {
	m_conv_by_tag[physical_tag] = bc;
}

void fe_tool::set_contact_convergence(contact_convergence c) { m_contact_conv = c; }

fe_tool::contact_convergence fe_tool::get_contact_convergence() const { return m_contact_conv; }

void fe_tool::set_contact_energy_balance(contact_energy_balance b) { m_contact_energy = b; }

fe_tool::contact_energy_balance fe_tool::get_contact_energy_balance() const { return m_contact_energy; }

void fe_tool::set_dirichlet_on_physical(int physical_tag, double T) {
	m_dirichlet_by_tag[physical_tag] = T;
}

void fe_tool::set_material_table_rho(std::vector<double> T, std::vector<double> rho) {
	m_rho_T = std::move(T);
	m_rho_val = std::move(rho);
}

void fe_tool::set_material_table_cp(std::vector<double> T, std::vector<double> cp) {
	m_cp_T = std::move(T);
	m_cp_val = std::move(cp);
}

void fe_tool::set_material_table_k(std::vector<double> T, std::vector<double> k) {
	m_k_T = std::move(T);
	m_k_val = std::move(k);
}

void fe_tool::set_mechanical_table_E(std::vector<double> T, std::vector<double> E) {
	m_E_T = std::move(T);
	m_E_val = std::move(E);
}

void fe_tool::set_mechanical_table_nu(std::vector<double> T, std::vector<double> nu) {
	m_nu_T = std::move(T);
	m_nu_val = std::move(nu);
}

void fe_tool::set_mechanical_table_alpha(std::vector<double> T, std::vector<double> alpha) {
	m_alpha_T = std::move(T);
	m_alpha_val = std::move(alpha);
}

void fe_tool::set_convection_air_all_exposed(convection_bc air_bc) {
	m_air_all = air_bc;
	m_use_air_all = true;
	m_use_flooded_by_y = false;
}

void fe_tool::set_convection_flooded_by_y(convection_bc air_bc, convection_bc water_bc, double y_threshold_world) {
	m_flood_air = air_bc;
	m_flood_water = water_bc;
	m_flood_y_threshold_world = y_threshold_world;
	m_use_air_all = false;
	m_use_flooded_by_y = true;
}

double fe_tool::max_temperature() const {
	double mx = -std::numeric_limits<double>::infinity();
	for (double v : m_T) mx = std::max(mx, v);
	return mx;
}

double fe_tool::min_temperature() const {
	double mn = std::numeric_limits<double>::infinity();
	for (double v : m_T) mn = std::min(mn, v);
	return mn;
}

void fe_tool::apply_dirichlet_bc(std::vector<char> &is_fixed) {
	is_fixed.assign(m_T.size(), 0);
	if (m_dirichlet_by_tag.empty() || m_bnd.empty()) return;
	std::unordered_set<int> warned;
	warned.reserve(m_dirichlet_by_tag.size());
	for (const boundary_edge &e : m_bnd) {
		auto it = m_dirichlet_by_tag.find(e.physical_tag);
		if (it == m_dirichlet_by_tag.end()) continue;
		double T = it->second;
		if (!std::isfinite(T)) {
			if (warned.insert(e.physical_tag).second) {
				std::fprintf(stderr, "warning: fe_tool dirichlet bc has non-finite temperature (tag=%d)\n", e.physical_tag);
			}
			continue;
		}
		if (T < -273.15 || std::abs(T) > 1.0e6) {
			if (warned.insert(e.physical_tag).second) {
				std::fprintf(stderr, "warning: fe_tool dirichlet bc has extreme temperature (tag=%d T=%g)\n", e.physical_tag, T);
			}
		}
		m_T[e.n0] = T;
		m_T[e.n1] = T;
		is_fixed[e.n0] = 1;
		is_fixed[e.n1] = 1;
	}
}

void fe_tool::advance_explicit(double dt) {
	if (m_T.empty()) return;
	if (m_capacity.size() != m_T.size()) return;
	if (m_K_rows.size() != m_T.size()) return;
	if (m_mat.rho <= 0. || m_mat.cp <= 0. || m_mat.k < 0.) return;
	if (!std::isfinite(dt) || dt <= 0.) return;

	if (!m_k_T.empty() || !m_cp_T.empty() || !m_rho_T.empty()) build_conduction_operator_from_temperature();

	std::vector<char> is_fixed;
	apply_dirichlet_bc(is_fixed);

	std::vector<double> power(m_T.size(), 0.);
	std::vector<double> power_conduction(m_T.size(), 0.);
	std::vector<double> power_convection(m_T.size(), 0.);

	for (std::size_t i = 0; i < m_T.size(); i++) {
		double pi = 0.;
		for (const auto &kv : m_K_rows[i]) {
			pi -= kv.second * m_T[kv.first];
		}
		power_conduction[i] += pi;
		power[i] += pi;
	}

	if (!m_bnd.empty()) {
		for (const boundary_edge &e : m_bnd) {
			convection_bc bc;
			bool active = false;

			auto it = m_conv_by_tag.find(e.physical_tag);
			if (it != m_conv_by_tag.end()) {
				bc = it->second;
				active = bc.h > 0.;
			} else if (m_use_air_all) {
				bc = m_air_all;
				active = bc.h > 0.;
			} else if (m_use_flooded_by_y) {
				glm::dvec2 p0 = to_world_frame(m_nodes_tool[e.n0]);
				glm::dvec2 p1 = to_world_frame(m_nodes_tool[e.n1]);
				double ym = 0.5 * (p0.y + p1.y);
				bc = (ym >= m_flood_y_threshold_world) ? m_flood_water : m_flood_air;
				active = bc.h > 0.;
			}

			if (!active) continue;

			glm::dvec2 x0 = m_nodes_tool[e.n0];
			glm::dvec2 x1 = m_nodes_tool[e.n1];
			double L = glm::length(x1 - x0);
			if (L <= 0.) continue;

			double Ti = m_T[e.n0];
			double Tj = m_T[e.n1];
			double di = bc.T_inf - Ti;
			double dj = bc.T_inf - Tj;

			double pi = bc.h * L / 6.0 * (2.0 * di + dj);
			double pj = bc.h * L / 6.0 * (di + 2.0 * dj);
			power_convection[e.n0] += pi;
			power_convection[e.n1] += pj;
			power[e.n0] += pi;
			power[e.n1] += pj;
		}
	}

	double E_sources = 0.;
	double E_conduction = 0.;
	double E_convection = 0.;
	double E_dirichlet = 0.;

	for (std::size_t i = 0; i < m_T.size(); i++) {
		power[i] += m_power_sources[i];

		if (std::isfinite(m_power_sources[i])) E_sources += dt * m_power_sources[i];
		if (std::isfinite(power_conduction[i])) E_conduction += dt * power_conduction[i];
		if (std::isfinite(power_convection[i])) E_convection += dt * power_convection[i];

		if (i < is_fixed.size() && is_fixed[i] && std::isfinite(power[i])) {
			E_dirichlet -= dt * power[i];
		}
	}

	m_thermal_energy.step_tool_E_sources += E_sources;
	m_thermal_energy.step_tool_E_conduction += E_conduction;
	m_thermal_energy.step_tool_E_convection += E_convection;
	m_thermal_energy.step_tool_E_dirichlet += E_dirichlet;
	m_thermal_energy.cumulative_tool_E_sources = m_thermal_energy.step_tool_E_sources;
	m_thermal_energy.cumulative_tool_E_conduction = m_thermal_energy.step_tool_E_conduction;
	m_thermal_energy.cumulative_tool_E_convection = m_thermal_energy.step_tool_E_convection;
	m_thermal_energy.cumulative_tool_E_dirichlet = m_thermal_energy.step_tool_E_dirichlet;

	for (std::size_t i = 0; i < m_T.size(); i++) {
		if (is_fixed[i]) continue;
		double cap = m_capacity[i];
		if (cap <= 0.) continue;
		m_T[i] += dt * power[i] / cap;
	}

	m_thermal_energy.tool_internal_E = thermal_internal_energy();
}

void fe_tool::set_mechanics_rayleigh(double a0, double a1) {
	if (!std::isfinite(a0) || a0 < 0.) a0 = 0.;
	if (!std::isfinite(a1) || a1 < 0.) a1 = 0.;
	m_mech_rayleigh_a0 = a0;
	m_mech_rayleigh_a1 = a1;
}

void fe_tool::build_boundary_edges_from_lines() {
	m_bnd.clear();
	std::unordered_map<edge_key, boundary_edge, edge_key_hash> best;
	best.reserve(m_line_elements.size());

	for (const boundary_edge &e : m_line_elements) {
		edge_key k;
		k.a = std::min(e.n0, e.n1);
		k.b = std::max(e.n0, e.n1);

		auto it = best.find(k);
		if (it == best.end()) {
			best.emplace(k, e);
			continue;
		}

		const int old_tag = it->second.physical_tag;
		const int new_tag = e.physical_tag;
		if (old_tag == 100 && new_tag != 100) it->second = e;
	}

	m_bnd.reserve(best.size());
	for (const auto &kv : best) m_bnd.push_back(kv.second);
}

void fe_tool::build_boundary_edge_to_adjacent_triangle() {
	m_bnd_edge_to_tri.clear();
	std::unordered_map<edge_key, unsigned int, edge_key_hash> tri_of_edge;

	for (unsigned int t = 0; t < m_tris.size(); t++) {
		const auto &tri = m_tris[t];
		unsigned int a = tri[0], b = tri[1], c = tri[2];

		auto add = [&](unsigned int i, unsigned int j) {
			edge_key k;
			k.a = std::min(i, j);
			k.b = std::max(i, j);
			if (tri_of_edge.find(k) == tri_of_edge.end()) tri_of_edge[k] = t;
		};
		add(a, b);
		add(b, c);
		add(c, a);
	}

	for (const boundary_edge &e : m_bnd) {
		edge_key k;
		k.a = std::min(e.n0, e.n1);
		k.b = std::max(e.n0, e.n1);
		auto it = tri_of_edge.find(k);
		if (it != tri_of_edge.end()) m_bnd_edge_to_tri[k] = it->second;
	}
}

void fe_tool::build_boundary_loop() {
	m_boundary_loop.clear();
	if (m_bnd.empty() || m_nodes_tool.empty()) return;

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(2 * m_bnd.size());
	std::unordered_set<unsigned int> bnodes;
	bnodes.reserve(2 * m_bnd.size());

	for (const boundary_edge &e : m_bnd) {
		if (e.n0 >= m_nodes_tool.size() || e.n1 >= m_nodes_tool.size()) continue;
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
		bnodes.insert(e.n0);
		bnodes.insert(e.n1);
	}
	if (bnodes.size() < 3) return;

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);
	for (unsigned int i : bnodes) {
		glm::dvec2 p = m_nodes_tool[i];
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = i;
			start_p = p;
		}
	}
	if (!has_start) return;

	std::unordered_set<unsigned int> visited;
	visited.reserve(bnodes.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(bnodes.size()) + 4; it++) {
		if (visited.find(cur) != visited.end()) break;
		visited.insert(cur);
		m_boundary_loop.push_back(cur);

		auto it_adj = adj.find(cur);
		if (it_adj == adj.end()) break;
		const std::vector<unsigned int> &nb = it_adj->second;
		if (nb.empty()) break;

		unsigned int next = nb[0];
		if (prev == std::numeric_limits<unsigned int>::max()) {
			next = nb[0];
		} else if (nb.size() == 1) {
			next = nb[0];
		} else if (nb.size() == 2) {
			next = (nb[0] == prev ? nb[1] : nb[0]);
		} else {
			glm::dvec2 t_prev = m_nodes_tool[cur] - m_nodes_tool[prev];
			double t_prev_n = glm::length(t_prev);
			if (t_prev_n > 0.) t_prev /= t_prev_n;
			double best = -std::numeric_limits<double>::infinity();
			next = prev;
			for (unsigned int cand : nb) {
				if (cand == prev) continue;
				glm::dvec2 t_c = m_nodes_tool[cand] - m_nodes_tool[cur];
				double t_c_n = glm::length(t_c);
				if (t_c_n > 0.) t_c /= t_c_n;
				double score = glm::dot(t_prev, t_c);
				if (score > best) {
					best = score;
					next = cand;
				}
			}
			if (next == prev) next = nb[0];
		}

		if (next == start) break;
		prev = cur;
		cur = next;
	}
}

void fe_tool::build_conduction_operator() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_mat.rho <= 0. || m_mat.cp <= 0. || m_mat.k < 0.) return;

	m_capacity.assign(m_nodes_tool.size(), 0.);

	std::vector<std::unordered_map<unsigned int, double>> rows(m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv4A = 1.0 / (4.0 * A);

		double kfac = m_mat.k * inv4A;

		double ke[3][3];
		double b[3] = {b0, b1, b2};
		double c[3] = {c0, c1, c2};
		for (int a = 0; a < 3; a++) {
			for (int bidx = 0; bidx < 3; bidx++) {
				ke[a][bidx] = kfac * (b[a] * b[bidx] + c[a] * c[bidx]);
			}
		}

		unsigned int idxs[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			unsigned int ia = idxs[a];
			for (int bb = 0; bb < 3; bb++) {
				unsigned int ib = idxs[bb];
				rows[ia][ib] += ke[a][bb];
			}
		}

		double cap = m_mat.rho * m_mat.cp * A / 3.0;
		m_capacity[i0] += cap;
		m_capacity[i1] += cap;
		m_capacity[i2] += cap;
	}

	m_K_rows.assign(m_nodes_tool.size(), {});
	for (unsigned int i = 0; i < rows.size(); i++) {
		m_K_rows[i].reserve(rows[i].size());
		for (const auto &kv : rows[i]) m_K_rows[i].push_back({kv.first, kv.second});
	}
}

void fe_tool::build_conduction_operator_from_temperature() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_T.size() != m_nodes_tool.size()) return;

	m_capacity.assign(m_nodes_tool.size(), 0.);
	std::vector<std::unordered_map<unsigned int, double>> rows(m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double Tavg = (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0;
		double k = k_at(Tavg);
		double rho = rho_at(Tavg);
		double cp = cp_at(Tavg);
		if (!std::isfinite(k) || k < 0.) k = m_mat.k;
		if (!std::isfinite(rho) || rho <= 0.) rho = m_mat.rho;
		if (!std::isfinite(cp) || cp <= 0.) cp = m_mat.cp;

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv4A = 1.0 / (4.0 * A);
		double kfac = k * inv4A;

		double ke[3][3];
		double b[3] = {b0, b1, b2};
		double c[3] = {c0, c1, c2};
		for (int a = 0; a < 3; a++) {
			for (int bidx = 0; bidx < 3; bidx++) {
				ke[a][bidx] = kfac * (b[a] * b[bidx] + c[a] * c[bidx]);
			}
		}

		unsigned int idxs[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			unsigned int ia = idxs[a];
			for (int bb = 0; bb < 3; bb++) {
				unsigned int ib = idxs[bb];
				rows[ia][ib] += ke[a][bb];
			}
		}

		double cap = rho * cp * A / 3.0;
		m_capacity[i0] += cap;
		m_capacity[i1] += cap;
		m_capacity[i2] += cap;
	}

	m_K_rows.assign(m_nodes_tool.size(), {});
	for (unsigned int i = 0; i < rows.size(); i++) {
		m_K_rows[i].reserve(rows[i].size());
		for (const auto &kv : rows[i]) m_K_rows[i].push_back({kv.first, kv.second});
	}
}

void fe_tool::build_mechanics_operator() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	const double eps = std::numeric_limits<double>::epsilon();
	if (!std::isfinite(m_mech.E) || m_mech.E <= eps) return;
	if (!std::isfinite(m_mech.nu) || m_mech.nu <= (-1.0 + eps) || m_mech.nu >= (0.5 - eps)) return;
	if (!std::isfinite(m_mech.alpha) || m_mech.alpha < -eps) return;

	std::vector<std::unordered_map<unsigned int, double>> rows(2 * m_nodes_tool.size());

	double E = m_mech.E;
	double nu = m_mech.nu;
	double c = E / ((1.0 + nu) * (1.0 - 2.0 * nu));
	double D[3][3] = {
		{c * (1.0 - nu), c * nu, 0.0},
		{c * nu, c * (1.0 - nu), 0.0},
		{0.0, 0.0, c * 0.5 * (1.0 - 2.0 * nu)},
	};

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv2A = 1.0 / (2.0 * A);
		double dNdx[3] = {b0 * inv2A, b1 * inv2A, b2 * inv2A};
		double dNdy[3] = {c0 * inv2A, c1 * inv2A, c2 * inv2A};

		double B[3][6] = {
			{dNdx[0], 0., dNdx[1], 0., dNdx[2], 0.},
			{0., dNdy[0], 0., dNdy[1], 0., dNdy[2]},
			{dNdy[0], dNdx[0], dNdy[1], dNdx[1], dNdy[2], dNdx[2]},
		};

		double DB[3][6];
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 6; j++) {
				DB[i][j] = D[i][0] * B[0][j] + D[i][1] * B[1][j] + D[i][2] * B[2][j];
			}
		}

		double ke[6][6];
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++) {
				double v = 0.;
				for (int k = 0; k < 3; k++) v += B[k][i] * DB[k][j];
				ke[i][j] = A * v;
			}
		}

		unsigned int idxn[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			for (int b = 0; b < 3; b++) {
				for (int da = 0; da < 2; da++) {
					for (int db = 0; db < 2; db++) {
						unsigned int ia = 2 * idxn[a] + static_cast<unsigned int>(da);
						unsigned int ib = 2 * idxn[b] + static_cast<unsigned int>(db);
						rows[ia][ib] += ke[2 * a + da][2 * b + db];
					}
				}
			}
		}
	}

	m_Km_rows.assign(2 * m_nodes_tool.size(), {});
	for (unsigned int i = 0; i < rows.size(); i++) {
		m_Km_rows[i].reserve(rows[i].size());
		for (const auto &kv : rows[i]) m_Km_rows[i].push_back({kv.first, kv.second});
	}
}

void fe_tool::build_mechanics_operator_from_temperature() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_T.size() != m_nodes_tool.size()) return;
	const double eps = std::numeric_limits<double>::epsilon();

	std::vector<std::unordered_map<unsigned int, double>> rows(2 * m_nodes_tool.size());

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double Tavg = (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0;
		double E = E_at(Tavg);
		double nu = nu_at(Tavg);
		if (!std::isfinite(E) || E <= eps) E = m_mech.E;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) nu = m_mech.nu;
		if (!std::isfinite(E) || E <= eps) continue;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) continue;

		double c = E / ((1.0 + nu) * (1.0 - 2.0 * nu));
		double D[3][3] = {
			{c * (1.0 - nu), c * nu, 0.0},
			{c * nu, c * (1.0 - nu), 0.0},
			{0.0, 0.0, c * 0.5 * (1.0 - 2.0 * nu)},
		};

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv2A = 1.0 / (2.0 * A);
		double dNdx[3] = {b0 * inv2A, b1 * inv2A, b2 * inv2A};
		double dNdy[3] = {c0 * inv2A, c1 * inv2A, c2 * inv2A};

		double B[3][6] = {
			{dNdx[0], 0., dNdx[1], 0., dNdx[2], 0.},
			{0., dNdy[0], 0., dNdy[1], 0., dNdy[2]},
			{dNdy[0], dNdx[0], dNdy[1], dNdx[1], dNdy[2], dNdx[2]},
		};

		double DB[3][6];
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 6; j++) {
				DB[i][j] = D[i][0] * B[0][j] + D[i][1] * B[1][j] + D[i][2] * B[2][j];
			}
		}

		double ke[6][6];
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++) {
				double v = 0.;
				for (int k = 0; k < 3; k++) v += B[k][i] * DB[k][j];
				ke[i][j] = A * v;
			}
		}

		unsigned int idxn[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			for (int b = 0; b < 3; b++) {
				for (int da = 0; da < 2; da++) {
					for (int db = 0; db < 2; db++) {
						unsigned int ia = 2 * idxn[a] + static_cast<unsigned int>(da);
						unsigned int ib = 2 * idxn[b] + static_cast<unsigned int>(db);
						rows[ia][ib] += ke[2 * a + da][2 * b + db];
					}
				}
			}
		}
	}

	m_Km_rows.assign(2 * m_nodes_tool.size(), {});
	for (unsigned int i = 0; i < rows.size(); i++) {
		m_Km_rows[i].reserve(rows[i].size());
		for (const auto &kv : rows[i]) m_Km_rows[i].push_back({kv.first, kv.second});
	}
	m_mech_mass.clear();
	m_mech_v_half.clear();
	m_mech_v_half_initialized = false;
}

void fe_tool::build_mech_constrained(std::vector<char> &constrained) const {
	constrained.assign(2 * m_nodes_tool.size(), 0);
	if (m_mech_fix_tags.empty() && m_mech_fix_nodes.empty() && m_mech_fix_x_tags.empty() && m_mech_fix_y_tags.empty() && m_mech_fix_x_nodes.empty() && m_mech_fix_y_nodes.empty()) return;

	ensure_mech_fix_cache();

	for (unsigned int n : m_mech_fix_cache_x_nodes) {
		if (n < m_nodes_tool.size()) constrained[2 * n + 0] = 1;
	}
	for (unsigned int n : m_mech_fix_cache_y_nodes) {
		if (n < m_nodes_tool.size()) constrained[2 * n + 1] = 1;
	}
}

void fe_tool::add_thermoelastic_rhs(std::vector<double> &rhs) const {
	const double eps = std::numeric_limits<double>::epsilon();
	if (m_T.empty()) return;
	if (!std::isfinite(m_mech.alpha) || m_mech.alpha < -eps) return;

	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;

		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];

		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;

		double b0 = x1.y - x2.y;
		double c0 = x2.x - x1.x;
		double b1 = x2.y - x0.y;
		double c1 = x0.x - x2.x;
		double b2 = x0.y - x1.y;
		double c2 = x1.x - x0.x;

		double inv2A = 1.0 / (2.0 * A);
		double dNdx[3] = {b0 * inv2A, b1 * inv2A, b2 * inv2A};
		double dNdy[3] = {c0 * inv2A, c1 * inv2A, c2 * inv2A};

		double B[3][6] = {
			{dNdx[0], 0., dNdx[1], 0., dNdx[2], 0.},
			{0., dNdy[0], 0., dNdy[1], 0., dNdy[2]},
			{dNdy[0], dNdx[0], dNdy[1], dNdx[1], dNdy[2], dNdx[2]},
		};

		double Tavg = (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0;
		double dT = Tavg - m_T_ref;
		double alpha = alpha_at(Tavg);
		if (!std::isfinite(alpha) || alpha < -eps) alpha = m_mech.alpha;
		if (std::abs(alpha) <= eps) continue;

		double E = E_at(Tavg);
		double nu = nu_at(Tavg);
		if (!std::isfinite(E) || E <= eps) E = m_mech.E;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) nu = m_mech.nu;
		if (!std::isfinite(E) || E <= eps) continue;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) continue;

		double c = E / ((1.0 + nu) * (1.0 - 2.0 * nu));
		double D[3][3] = {
			{c * (1.0 - nu), c * nu, 0.0},
			{c * nu, c * (1.0 - nu), 0.0},
			{0.0, 0.0, c * 0.5 * (1.0 - 2.0 * nu)},
		};
		double eps_th[3] = {alpha * dT, alpha * dT, 0.0};

		double sig_th[3] = {
			D[0][0] * eps_th[0] + D[0][1] * eps_th[1] + D[0][2] * eps_th[2],
			D[1][0] * eps_th[0] + D[1][1] * eps_th[1] + D[1][2] * eps_th[2],
			D[2][0] * eps_th[0] + D[2][1] * eps_th[1] + D[2][2] * eps_th[2],
		};

		double fe[6] = {0., 0., 0., 0., 0., 0.};
		for (int a = 0; a < 6; a++) {
			double v = 0.;
			for (int k = 0; k < 3; k++) v += B[k][a] * sig_th[k];
			fe[a] = A * v;
		}

		unsigned int idxn[3] = {i0, i1, i2};
		for (int a = 0; a < 3; a++) {
			rhs[2 * idxn[a] + 0] += fe[2 * a + 0];
			rhs[2 * idxn[a] + 1] += fe[2 * a + 1];
		}
	}
}

void fe_tool::matvec_mechanics(const std::vector<char> &constrained, const std::vector<double> &x, std::vector<double> &y) const {
	y.assign(x.size(), 0.);
	for (unsigned int i = 0; i < m_Km_rows.size(); i++) {
		if (i < constrained.size() && constrained[i]) {
			y[i] = x[i];
			continue;
		}
		double s = 0.;
		for (const auto &kv : m_Km_rows[i]) s += kv.second * x[kv.first];
		y[i] = s;
	}
}

void fe_tool::ensure_mechanics_lumped_mass() {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_mat.rho <= 0.) return;
	const std::size_t ndof = 2 * m_nodes_tool.size();
	if (m_mech_mass.size() == ndof && m_mech_v_half.size() == ndof) return;

	std::vector<double> nodal_mass(m_nodes_tool.size(), 0.);
	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;
		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];
		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;
		double Tavg = m_T.size() == m_nodes_tool.size() ? (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0 : m_T_ref;
		double rho = rho_at(Tavg);
		if (!std::isfinite(rho) || rho <= 0.) rho = m_mat.rho;
		double m = rho * A / 3.0;
		nodal_mass[i0] += m;
		nodal_mass[i1] += m;
		nodal_mass[i2] += m;
	}

	m_mech_mass.assign(ndof, 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		m_mech_mass[2 * i + 0] = nodal_mass[i];
		m_mech_mass[2 * i + 1] = nodal_mass[i];
	}
	m_mech_v_half.assign(ndof, 0.);
	m_mech_mass_scaled = false;
	m_mech_v_half_initialized = false;
}

double fe_tool::mechanics_dt_crit() const {
	if (m_nodes_tool.empty() || m_tris.empty()) return std::numeric_limits<double>::infinity();
	const double eps = std::numeric_limits<double>::epsilon();

	double dtmin = std::numeric_limits<double>::infinity();
	for (const auto &tri : m_tris) {
		unsigned int i0 = tri[0];
		unsigned int i1 = tri[1];
		unsigned int i2 = tri[2];
		if (i0 >= m_nodes_tool.size() || i1 >= m_nodes_tool.size() || i2 >= m_nodes_tool.size()) continue;
		const glm::dvec2 &x0 = m_nodes_tool[i0];
		const glm::dvec2 &x1 = m_nodes_tool[i1];
		const glm::dvec2 &x2 = m_nodes_tool[i2];
		double area2 = tri_area2(x0, x1, x2);
		double A = 0.5 * std::abs(area2);
		if (A <= 0.) continue;
		double Tavg = m_T.size() == m_nodes_tool.size() ? (m_T[i0] + m_T[i1] + m_T[i2]) / 3.0 : m_T_ref;
		double rho = rho_at(Tavg);
		double E = E_at(Tavg);
		double nu = nu_at(Tavg);
		if (!std::isfinite(rho) || rho <= 0.) rho = m_mat.rho;
		if (!std::isfinite(E) || E <= eps) E = m_mech.E;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) nu = m_mech.nu;
		if (!std::isfinite(rho) || rho <= 0.) continue;
		if (!std::isfinite(E) || E <= eps) continue;
		if (!std::isfinite(nu) || nu <= (-1.0 + eps) || nu >= (0.5 - eps)) continue;

		double mu = E / (2.0 * (1.0 + nu));
		double lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
		double c = std::sqrt((lambda + 2.0 * mu) / rho);
		if (!(c > 0.) || !std::isfinite(c)) continue;
		double e01 = glm::length(x1 - x0);
		double e12 = glm::length(x2 - x1);
		double e20 = glm::length(x0 - x2);
		double emax = std::max(e01, std::max(e12, e20));
		if (!(emax > 0.)) continue;
		double h = 2.0 * A / emax;
		if (!(h > 0.)) continue;
		double dt = h / c;
		if (dt > 0. && std::isfinite(dt)) dtmin = std::min(dtmin, dt);
	}
	return dtmin;
}

void fe_tool::advance_mechanics_explicit(double dt) {
	if (dt <= 0.) return;
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_Km_rows.size() != 2 * m_nodes_tool.size()) return;
	if (m_force_sources.size() != m_nodes_tool.size()) return;

	if (!m_E_T.empty() || !m_nu_T.empty()) build_mechanics_operator_from_temperature();

	ensure_mechanics_lumped_mass();
	if (m_mech_mass.size() != 2 * m_nodes_tool.size()) return;

	bool hard_fail = false;
	{
		const char *s = std::getenv("MFREE_FE_TOOL_HARD_FAIL_ON_INVALID");
		if (s && s[0] != '\0') hard_fail = (std::atoi(s) != 0);
	}

	const double dtcrit = mechanics_dt_crit();
	if (std::isfinite(dtcrit) && dtcrit > 0. && dt > 0.9 * dtcrit) {
		std::fprintf(stderr, "warning: fe_tool explicit mechanics dt=%g exceeds 0.9*dtcrit=%g\n", dt, 0.9 * dtcrit);
	}

	std::vector<char> constrained;
	build_mech_constrained(constrained);

	std::vector<double> rhs(2 * m_nodes_tool.size(), 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		rhs[2 * i + 0] += m_force_sources[i].x;
		rhs[2 * i + 1] += m_force_sources[i].y;
	}
	add_thermoelastic_rhs(rhs);
	if (std::getenv("MFREE_DEBUG_FE_TOOL_MECH_RHS")) {
		double mx = 0.;
		for (double v : rhs) mx = std::max(mx, std::abs(v));
		std::fprintf(stderr, "warning: fe_tool mech rhs max_abs=%g dt=%g\n", mx, dt);
	}
	for (unsigned int i = 0; i < rhs.size(); i++) if (constrained[i]) rhs[i] = 0.;

	std::vector<double> u(2 * m_nodes_tool.size(), 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size() && i < m_u.size(); i++) {
		u[2 * i + 0] = m_u[i].x;
		u[2 * i + 1] = m_u[i].y;
	}
	for (unsigned int i = 0; i < u.size(); i++) if (constrained[i]) u[i] = 0.;

	if (m_mech_v_half.size() != u.size()) m_mech_v_half.assign(u.size(), 0.);
	for (unsigned int i = 0; i < m_mech_v_half.size(); i++) if (constrained[i]) m_mech_v_half[i] = 0.;

	std::vector<double> Ku;
	matvec_mechanics(constrained, u, Ku);

	std::vector<double> damp(u.size(), 0.);
	if (m_mech_rayleigh_a0 != 0.) {
		for (unsigned int i = 0; i < damp.size(); i++) damp[i] += m_mech_rayleigh_a0 * m_mech_mass[i] * m_mech_v_half[i];
	}
	if (m_mech_rayleigh_a1 != 0.) {
		std::vector<double> Kv;
		matvec_mechanics(constrained, m_mech_v_half, Kv);
		for (unsigned int i = 0; i < damp.size(); i++) damp[i] += m_mech_rayleigh_a1 * Kv[i];
	}

	auto hard_fail_now = [&](unsigned int idx, double ui, double vi) {
		double max_abs_u = 0.;
		for (double v : u) {
			if (!std::isfinite(v)) continue;
			max_abs_u = std::max(max_abs_u, std::abs(v));
		}
		double mi = idx < m_mech_mass.size() ? m_mech_mass[idx] : 0.;
		double ri = idx < rhs.size() ? rhs[idx] : 0.;
		double kui = idx < Ku.size() ? Ku[idx] : 0.;
		double di = idx < damp.size() ? damp[idx] : 0.;
		std::fprintf(stderr,
		             "error: fe_tool explicit mechanics hard fail (idx=%u u=%g v=%g max_abs_u=%g dt=%g dtcrit=%g m=%g rhs=%g Ku=%g damp=%g)\n",
		             idx, ui, vi, max_abs_u, dt, dtcrit, mi, ri, kui, di);
		std::fflush(stderr);
		std::abort();
	};

	if (hard_fail) {
		for (unsigned int i = 0; i < u.size(); i++) {
			if (constrained[i]) continue;
			if (!std::isfinite(u[i]) || !std::isfinite(m_mech_v_half[i]) || std::abs(u[i]) > 1.0) hard_fail_now(i, u[i], m_mech_v_half[i]);
		}
	}

	if (!m_mech_v_half_initialized) {
		for (unsigned int i = 0; i < u.size(); i++) {
			if (constrained[i]) continue;
			double m = m_mech_mass[i];
			if (!(m > 0.)) continue;
			double a = (rhs[i] - Ku[i] - damp[i]) / m;
			if (std::isfinite(a)) m_mech_v_half[i] = -0.5 * dt * a;
		}
		m_mech_v_half_initialized = true;
	}

	for (unsigned int i = 0; i < u.size(); i++) {
		double m = m_mech_mass[i];
		if (!(m > 0.)) continue;
		double a = (rhs[i] - Ku[i] - damp[i]) / m;
		m_mech_v_half[i] += dt * a;
		u[i] += dt * m_mech_v_half[i];
		if (hard_fail && (!std::isfinite(u[i]) || !std::isfinite(m_mech_v_half[i]) || std::abs(u[i]) > 1.0)) hard_fail_now(i, u[i], m_mech_v_half[i]);
		if (!std::isfinite(u[i]) || !std::isfinite(m_mech_v_half[i]) || std::abs(u[i]) > 1.0) {
			std::fprintf(stderr, "warning: fe_tool explicit mechanics produced invalid state (i=%u u=%g v=%g)\n", i, u[i], m_mech_v_half[i]);
			u[i] = 0.;
			m_mech_v_half[i] = 0.;
		}
	}

	for (unsigned int i = 0; i < u.size(); i++) if (constrained[i]) u[i] = 0.;
	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		if (i >= m_u.size()) break;
		m_u[i].x = u[2 * i + 0];
		m_u[i].y = u[2 * i + 1];
	}
}

void fe_tool::solve_mechanics_quasistatic(unsigned int max_iters, double rel_tol) {
	if (m_nodes_tool.empty() || m_tris.empty()) return;
	if (m_Km_rows.size() != 2 * m_nodes_tool.size()) return;
	if (m_force_sources.size() != m_nodes_tool.size()) return;

	if (!m_E_T.empty() || !m_nu_T.empty()) build_mechanics_operator_from_temperature();

	const double eps = std::numeric_limits<double>::epsilon();

	std::vector<char> constrained;
	build_mech_constrained(constrained);

	unsigned int fixed_nodes = 0;
	unsigned int fixed_nodes_x = 0;
	unsigned int fixed_nodes_y = 0;
	unsigned int fixed_dofs = 0;
	{
		std::unordered_set<unsigned int> uniq;
		std::unordered_set<unsigned int> uniq_x;
		std::unordered_set<unsigned int> uniq_y;
		uniq.reserve(m_nodes_tool.size() / 4 + 4);
		uniq_x.reserve(m_nodes_tool.size() / 4 + 4);
		uniq_y.reserve(m_nodes_tool.size() / 4 + 4);
		for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
			if (constrained[2 * i + 0] || constrained[2 * i + 1]) uniq.insert(i);
			if (constrained[2 * i + 0]) uniq_x.insert(i);
			if (constrained[2 * i + 1]) uniq_y.insert(i);
		}
		fixed_nodes = static_cast<unsigned int>(uniq.size());
		fixed_nodes_x = static_cast<unsigned int>(uniq_x.size());
		fixed_nodes_y = static_cast<unsigned int>(uniq_y.size());
		for (unsigned int i = 0; i < constrained.size(); i++) fixed_dofs += (constrained[i] ? 1u : 0u);
	}
	if (fixed_nodes < 3 || fixed_dofs < 3 || fixed_nodes_x == 0 || fixed_nodes_y == 0) {
		std::fprintf(stderr,
		             "warning: fe_tool mechanics solve has insufficient constraints (fixed_nodes=%u fixed_dofs=%u fixed_nodes_x=%u fixed_nodes_y=%u vel_x=%g)\n",
		             fixed_nodes, fixed_dofs, fixed_nodes_x, fixed_nodes_y, m_vel.x);
		return;
	}

	std::vector<double> rhs(2 * m_nodes_tool.size(), 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		rhs[2 * i + 0] += m_force_sources[i].x;
		rhs[2 * i + 1] += m_force_sources[i].y;
	}
	add_thermoelastic_rhs(rhs);

	for (unsigned int i = 0; i < rhs.size(); i++) {
		if (constrained[i]) rhs[i] = 0.;
	}

	auto matvec = [&](const std::vector<double> &x, std::vector<double> &y) { matvec_mechanics(constrained, x, y); };

	auto dot = [&](const std::vector<double> &a, const std::vector<double> &b) {
		double s = 0.;
		for (unsigned int i = 0; i < a.size(); i++) s += a[i] * b[i];
		return s;
	};

	auto norm = [&](const std::vector<double> &a) { return std::sqrt(dot(a, a)); };

	std::vector<double> x(2 * m_nodes_tool.size(), 0.);
	for (unsigned int i = 0; i < m_nodes_tool.size() && i < m_u.size(); i++) {
		x[2 * i + 0] = m_u[i].x;
		x[2 * i + 1] = m_u[i].y;
	}
	for (unsigned int i = 0; i < x.size(); i++) if (constrained[i]) x[i] = 0.;

	std::vector<double> Ax;
	matvec(x, Ax);
	std::vector<double> r(x.size(), 0.);
	for (unsigned int i = 0; i < x.size(); i++) r[i] = rhs[i] - Ax[i];
	for (unsigned int i = 0; i < r.size(); i++) if (constrained[i]) r[i] = 0.;

	double rhs_norm = norm(rhs);
	if (!(rhs_norm > 0.)) rhs_norm = 1.0;
	double r_norm0 = norm(r);
	if (r_norm0 / rhs_norm <= rel_tol) return;

	std::vector<double> p = r;
	std::vector<double> Ap;
	double rr = dot(r, r);

	for (unsigned int it = 0; it < max_iters; it++) {
		matvec(p, Ap);
		double pAp = dot(p, Ap);
		if (!(pAp > 0.)) break;
		double alpha = rr / pAp;
		for (unsigned int i = 0; i < x.size(); i++) x[i] += alpha * p[i];
		for (unsigned int i = 0; i < r.size(); i++) r[i] -= alpha * Ap[i];
		for (unsigned int i = 0; i < r.size(); i++) if (constrained[i]) r[i] = 0.;
		double rr_new = dot(r, r);
		double rel = std::sqrt(rr_new) / rhs_norm;
		if (rel <= rel_tol) break;
		double beta = rr_new / rr;
		for (unsigned int i = 0; i < p.size(); i++) p[i] = r[i] + beta * p[i];
		rr = rr_new;
	}

	for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
		m_u[i].x = x[2 * i + 0];
		m_u[i].y = x[2 * i + 1];
	}
}

double fe_tool::max_displacement_norm() const {
	double mx = 0.;
	for (const auto &u : m_u) mx = std::max(mx, glm::length(u));
	return mx;
}

std::pair<unsigned int, double> fe_tool::nearest_boundary_edge_barycentric(glm::dvec2 x_tool) const {
	double best_d2 = std::numeric_limits<double>::infinity();
	unsigned int best_e = static_cast<unsigned int>(m_bnd.size());
	double best_t = 0.;

	for (unsigned int ei = 0; ei < m_bnd.size(); ei++) {
		const boundary_edge &e = m_bnd[ei];
		glm::dvec2 a = m_nodes_tool[e.n0] + (e.n0 < m_u.size() ? m_u[e.n0] : glm::dvec2(0.));
		glm::dvec2 b = m_nodes_tool[e.n1] + (e.n1 < m_u.size() ? m_u[e.n1] : glm::dvec2(0.));
		glm::dvec2 ab = b - a;
		double ab2 = glm::dot(ab, ab);
		if (ab2 <= 0.) continue;
		double t = glm::dot(x_tool - a, ab) / ab2;
		t = std::max(0.0, std::min(1.0, t));
		glm::dvec2 p = a + t * ab;
		glm::dvec2 d = x_tool - p;
		double d2 = glm::dot(d, d);
		if (d2 < best_d2) {
			best_d2 = d2;
			best_e = ei;
			best_t = t;
		}
	}

	return {best_e, best_t};
}
void fe_tool::set_mu(double mu) { m_mu = mu; }
double fe_tool::get_mu() const { return m_mu; }

bool fe_tool::bbox::in(glm::dvec2 qp) {
bool in_x = qp.x >= bbmin_x && qp.x <= bbmax_x;
bool in_y = qp.y >= bbmin_y && qp.y <= bbmax_y;
return in_x && in_y;
}

bool fe_tool::bbox::valid() const {
bool invalid_x = bbmax_x - bbmin_x  < 1e-12;
bool invalid_y = bbmax_y - bbmin_y  < 1e-12;
return !(invalid_x || invalid_y);
}

fe_tool::bbox::bbox() {}

fe_tool::bbox::bbox(glm::dvec2 p1, glm::dvec2 p2) {
bbmin_x = std::fmin(p1.x, p2.x);
bbmax_x = std::fmax(p1.x, p2.x);
bbmin_y = std::fmin(p1.y, p2.y);
bbmax_y = std::fmax(p1.y, p2.y);
}

fe_tool::bbox::bbox(double bbmin_x, double bbmax_x, double bbmin_y, double bbmax_y) :
bbmin_x(bbmin_x), bbmax_x(bbmax_x), bbmin_y(bbmin_y), bbmax_y(bbmax_y) {}

fe_tool::bbox fe_tool::get_bbox_world() const {
fe_tool::bbox bb;
bb.bbmin_x = std::numeric_limits<double>::infinity();
bb.bbmin_y = std::numeric_limits<double>::infinity();
bb.bbmax_x = -std::numeric_limits<double>::infinity();
bb.bbmax_y = -std::numeric_limits<double>::infinity();

for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
glm::dvec2 p = to_world_frame(m_nodes_tool[i]);
if (m_u.size() == m_nodes_tool.size()) {
p += m_u[i];
}
bb.bbmin_x = std::min(bb.bbmin_x, p.x);
bb.bbmin_y = std::min(bb.bbmin_y, p.y);
bb.bbmax_x = std::max(bb.bbmax_x, p.x);
bb.bbmax_y = std::max(bb.bbmax_y, p.y);
}
return bb;
}

glm::dvec2 fe_tool::get_edge_coord() const {
glm::dvec2 best(0.);
double min_y = std::numeric_limits<double>::infinity();
for (unsigned int i = 0; i < m_nodes_tool.size(); i++) {
glm::dvec2 p = to_world_frame(m_nodes_tool[i]);
if (m_u.size() == m_nodes_tool.size()) {
p += m_u[i];
}
if (p.y < min_y) {
min_y = p.y;
best = p;
}
}
return best;
}

double fe_tool::inside(glm::dvec2 qp) const {
std::vector<glm::dvec2> poly = boundary_loop_world();
if (poly.size() < 3) return -1.0;

bool c = false;
for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
if (((poly[i].y > qp.y) != (poly[j].y > qp.y)) &&
(qp.x < (poly[j].x - poly[i].x) * (qp.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x))
c = !c;
}

if (!c) return -1.0;

double min_dist = std::numeric_limits<double>::infinity();
for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
glm::dvec2 a = poly[j];
glm::dvec2 b = poly[i];
glm::dvec2 dir = b - a;
double len2 = dir.x*dir.x + dir.y*dir.y;
double t = 0.0;
if (len2 > 0.0) {
t = glm::dot(qp - a, dir) / len2;
t = std::max(0.0, std::min(1.0, t));
}
glm::dvec2 proj = a + t * dir;
double dist = glm::length(qp - proj);
min_dist = std::min(min_dist, dist);
}
return min_dist;
}

```


**Tool Call: src\\timestep\_estimator.cpp**
Status: Completed

Diff: src\timestep_estimator.cpp
```
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

```


**Tool Call: CMakeLists.txt**
Status: Failed

Diff: CMakeLists.txt
```
cmake_minimum_required(VERSION 3.16)

project(mfree_iwf_ul_cut_refine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mfree_iwf
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/refine_cut_main.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/test_benches.cpp
  src/benchmarks/test_cuttings.cpp
  src/benchmarks/test_density.cpp
)

target_include_directories(mfree_iwf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf PRIVATE OpenMP::OpenMP_CXX)
endif()

enable_testing()

add_executable(mfree_iwf_validate
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/validate_main.cpp
)

target_include_directories(mfree_iwf_validate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf_validate PRIVATE OpenMP::OpenMP_CXX)
endif()

add_test(NAME mfree_iwf_validate COMMAND mfree_iwf_validate)

add_test(NAME mfree_iwf_preprocess_model1_rigid COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_rigid PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_rigid"
)

add_test(NAME mfree_iwf_preprocess_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh"
)

add_test(NAME mfree_iwf_geom_validate_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_geom_validate_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/geom_model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh;MFREE_GEOM_VALIDATE=1"
)

add_executable(mfree_geom_validation_math_test
  src/benchmarks/test_geom_validation_math.cpp
)

target_include_directories(mfree_geom_validation_math_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

add_test(NAME mfree_geom_validation_math_test COMMAND mfree_geom_validation_math_test)

```


**Tool Call: CMakeLists.txt**
Status: Completed

Diff: CMakeLists.txt
```
cmake_minimum_required(VERSION 3.16)

project(mfree_iwf_ul_cut_refine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mfree_iwf
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/refine_cut_main.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/timestep_estimator.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/test_benches.cpp
  src/benchmarks/test_cuttings.cpp
  src/benchmarks/test_density.cpp
)

target_include_directories(mfree_iwf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf PRIVATE OpenMP::OpenMP_CXX)
endif()

enable_testing()

add_executable(mfree_iwf_validate
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/timestep_estimator.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/validate_main.cpp
)

target_include_directories(mfree_iwf_validate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf_validate PRIVATE OpenMP::OpenMP_CXX)
endif()

add_test(NAME mfree_iwf_validate COMMAND mfree_iwf_validate)

add_test(NAME mfree_iwf_preprocess_model1_rigid COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_rigid PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_rigid"
)

add_test(NAME mfree_iwf_preprocess_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh"
)

add_test(NAME mfree_iwf_geom_validate_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_geom_validate_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/geom_model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh;MFREE_GEOM_VALIDATE=1"
)

add_executable(mfree_geom_validation_math_test
  src/benchmarks/test_geom_validation_math.cpp
)

target_include_directories(mfree_geom_validation_math_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

add_test(NAME mfree_geom_validation_math_test COMMAND mfree_geom_validation_math_test)

```


## Assistant

<thinking>
**Considering test integration**

I'm thinking about integrating the `test_cuttings` functionality. I believe I might need to include a header file named `test_cuttings.h` to get everything working properly. It's important to ensure I've got all the necessary components in place before moving forward with this integration. I’ll take a close look at how the tests interact to make sure I'm executing everything correctly. Let's confirm that I'm on the right track here!
</thinking>

**Tool Call: src\\benchmarks\\test\_cuttings.h**
Status: Failed

Diff: src\benchmarks\test_cuttings.h
```
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

#ifndef BENCHMARKS_TEST_CUTTINGS_H_
#define BENCHMARKS_TEST_CUTTINGS_H_

#include "../particle.h"
#include "../body.h"
#include "../fe_tool.h"
#include "../fe_tool.h"
#include "../correctors.h"
#include "../cont_mech.h"
#include "../material.h"
#include "../derivatives.h"
#include "../plasticity.h"
#include "../johnson_cook_Sima_2010.h"
#include "../logger.h"
#include "../leap_frog.h"
#include "material_library.h"

#include <iostream>
#include <stdlib.h>
#include <fenv.h>
#include <chrono>
#include <filesystem>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

extern logger *global_logger;

body* cutting_ref_mr(unsigned int nbox);
body* cutting_ref_single_resol(unsigned int nbox);
body* cutting_ref_multi_resol_apriori(unsigned int nbox);
body* cutting_ref_multi_resol_dynamic(unsigned int nbox);

#endif /* BENCHMARKS_TEST_CUTTINGS_H_ */

```


**Tool Call: src\\benchmarks\\test\_cuttings.h**
Status: Completed

Diff: src\benchmarks\test_cuttings.h
```
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

#ifndef BENCHMARKS_TEST_CUTTINGS_H_
#define BENCHMARKS_TEST_CUTTINGS_H_

#include "../particle.h"
#include "../body.h"
#include "../fe_tool.h"
#include "../fe_tool.h"
#include "../correctors.h"
#include "../cont_mech.h"
#include "../material.h"
#include "../derivatives.h"
#include "../plasticity.h"
#include "../timestep_estimator.h"
#include "../johnson_cook_Sima_2010.h"
#include "../logger.h"
#include "../leap_frog.h"
#include "material_library.h"

#include <iostream>
#include <stdlib.h>
#include <fenv.h>
#include <chrono>
#include <filesystem>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

extern logger *global_logger;

body* cutting_ref_mr(unsigned int nbox);
body* cutting_ref_single_resol(unsigned int nbox);
body* cutting_ref_multi_resol_apriori(unsigned int nbox);
body* cutting_ref_multi_resol_dynamic(unsigned int nbox);

#endif /* BENCHMARKS_TEST_CUTTINGS_H_ */

```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 820-910)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}

	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}

		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;

```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 80-150)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
	if (*end != '\0') return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_read_env_table(const char *key, std::vector<double> &T_out, std::vector<double> &v_out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r') ++p;
		if (!*p) break;

		char *end = nullptr;
		double T = strtod(p, &end);
		if (end == p || !std::isfinite(T)) return false;
		p = end;

		while (*p == ' ' || *p == '\t') ++p;
		if (*p != ':' && *p != '=') return false;
		++p;
		while (*p == ' ' || *p == '\t') ++p;

		end = nullptr;
		double v = strtod(p, &end);
		if (end == p || !std::isfinite(v)) return false;
		p = end;

		pairs.push_back({T, v});
	}

	if (pairs.size() < 2) return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

	T_out.clear();
	v_out.clear();
	for (const auto &kv : pairs) {
		if (!T_out.empty() && kv.first == T_out.back()) {
			v_out.back() = kv.second;
			continue;
		}
		T_out.push_back(kv.first);
		v_out.push_back(kv.second);
	}
	return T_out.size() >= 2;
}

static void adjust_workpiece_y_bounds_for_feed(double base_lo_y, double base_hi_y, unsigned int base_ny,
											   double target_feed, unsigned int safety_layers,
											   double &lo_y, double &hi_y, unsigned int &ny, double &dy) {
	hi_y = base_hi_y;
	double base_thickness = base_hi_y - base_lo_y;
	dy = base_thickness / (base_ny - 1);
	double required_thickness = target_feed + safety_layers * dy;

	unsigned int extra_layers = 0;
	if (required_thickness > base_thickness) {
		double raw = (required_thickness - base_thickness) / dy;
		extra_layers = (unsigned int)std::ceil(raw - 1e-12);
	}
	ny = base_ny + extra_layers;
	lo_y = hi_y - (ny - 1) * dy;
}

static double read_coupled_motion_ratio() {
	const char *enable_env = getenv("MFREE_COUPLED_MOTION");
	bool enabled = (enable_env && atoi(enable_env) != 0);

```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 150-230)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	bool enabled = (enable_env && atoi(enable_env) != 0);
	if (!enabled) return 1.0;

	double ratio = 1.0;
	if (try_read_env_double("MFREE_COUPLED_MOTION_RATIO", ratio)) {
		if (!std::isfinite(ratio)) ratio = 1.0;
		ratio = std::max(0.0, std::min(1.0, ratio));
		return ratio;
	}

	const char *primary = getenv("MFREE_PRIMARY_MOVING_BODY");
	if (!primary || primary[0] == '\0') return 1.0;
	std::string s(primary);
	for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s == "workpiece" || s == "wp") return 0.0;
	if (s == "both" || s == "coupled") return 0.5;
	return 1.0;
}

static void apply_mech_fix_tags_from_env(fe_tool &ft) {
	auto apply_tag_list = [&](const char *env_key, auto apply_tag) -> bool {
		const char *tags = getenv(env_key);
		if (!tags || tags[0] == '\0') return false;
		std::string s(tags);
		std::size_t i = 0;
		bool any = false;
		while (i < s.size()) {
			while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
			if (i >= s.size()) break;
			std::size_t j = i;
			while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
			std::string tok = s.substr(i, j - i);
			errno = 0;
			char *end = nullptr;
			long v = std::strtol(tok.c_str(), &end, 10);
			if (end != tok.c_str() && end != nullptr && *end == '\0' && errno == 0) {
				if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max() && v != 0) {
					apply_tag(static_cast<int>(v));
				any = true;
				}
			}
			i = j;
		}
		return any;
	};

	bool any = false;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_Y_TAGS", [&](int tag) { ft.set_mechanics_fixed_y_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_X_TAGS", [&](int tag) { ft.set_mechanics_fixed_x_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_TAGS", [&](int tag) { ft.set_mechanics_fixed_on_physical(tag); }) || any;
	if (any) {
		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		int anchor_tag = 0;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_TAG", v)) anchor_tag = v;
		}
		if (anchor_ux && anchor_tag != 0) {
			std::unordered_set<unsigned int> nodes;
			for (const auto &e : ft.boundary_edges()) {
				if (e.physical_tag != anchor_tag) continue;
				nodes.insert(e.n0);
				nodes.insert(e.n1);
			}
			if (!nodes.empty()) {
				unsigned int anchor = *nodes.begin();
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : nodes) {
					glm::dvec2 pw = ft.node_world(n);
					if (!std::isfinite(pw.x)) continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft.set_mechanics_fixed_x_nodes({anchor});
			}

```


**Tool Call: src\\benchmarks\\test\_cuttings.cpp**
Status: Completed

Diff: src\benchmarks\test_cuttings.cpp
```
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

#include "test_cuttings.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <cerrno>

static bool try_read_env_double(const char *key, double &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	char *end = nullptr;
	double v = strtod(s, &end);
	if (end == s || !std::isfinite(v)) return false;
	out = v;
	return true;
}

static bool try_read_env_int(const char *key, int &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;
	if (!*s) return false;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s) return false;
	while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
	if (*end != '\0') return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_read_env_table(const char *key, std::vector<double> &T_out, std::vector<double> &v_out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r') ++p;
		if (!*p) break;

		char *end = nullptr;
		double T = strtod(p, &end);
		if (end == p || !std::isfinite(T)) return false;
		p = end;

		while (*p == ' ' || *p == '\t') ++p;
		if (*p != ':' && *p != '=') return false;
		++p;
		while (*p == ' ' || *p == '\t') ++p;

		end = nullptr;
		double v = strtod(p, &end);
		if (end == p || !std::isfinite(v)) return false;
		p = end;

		pairs.push_back({T, v});
	}

	if (pairs.size() < 2) return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

	T_out.clear();
	v_out.clear();
	for (const auto &kv : pairs) {
		if (!T_out.empty() && kv.first == T_out.back()) {
			v_out.back() = kv.second;
			continue;
		}
		T_out.push_back(kv.first);
		v_out.push_back(kv.second);
	}
	return T_out.size() >= 2;
}

static void adjust_workpiece_y_bounds_for_feed(double base_lo_y, double base_hi_y, unsigned int base_ny,
											   double target_feed, unsigned int safety_layers,
											   double &lo_y, double &hi_y, unsigned int &ny, double &dy) {
	hi_y = base_hi_y;
	double base_thickness = base_hi_y - base_lo_y;
	dy = base_thickness / (base_ny - 1);
	double required_thickness = target_feed + safety_layers * dy;

	unsigned int extra_layers = 0;
	if (required_thickness > base_thickness) {
		double raw = (required_thickness - base_thickness) / dy;
		extra_layers = (unsigned int)std::ceil(raw - 1e-12);
	}
	ny = base_ny + extra_layers;
	lo_y = hi_y - (ny - 1) * dy;
}

static double read_coupled_motion_ratio() {
	const char *enable_env = getenv("MFREE_COUPLED_MOTION");
	bool enabled = (enable_env && atoi(enable_env) != 0);
	if (!enabled) return 1.0;

	double ratio = 1.0;
	if (try_read_env_double("MFREE_COUPLED_MOTION_RATIO", ratio)) {
		if (!std::isfinite(ratio)) ratio = 1.0;
		ratio = std::max(0.0, std::min(1.0, ratio));
		return ratio;
	}

	const char *primary = getenv("MFREE_PRIMARY_MOVING_BODY");
	if (!primary || primary[0] == '\0') return 1.0;
	std::string s(primary);
	for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s == "workpiece" || s == "wp") return 0.0;
	if (s == "both" || s == "coupled") return 0.5;
	return 1.0;
}

static double estimate_dt_for_cutting(const physical_constants &pc, double dx, double hdx, double relative_speed, double empirical_cap, const fe_tool *ft) {
	coupled_timestep_config cfg;
	cfg.particle_spacing = dx;
	cfg.smoothing_length_ratio = hdx;
	cfg.max_relative_speed = relative_speed;
	cfg.empirical_dt_cap = empirical_cap;
	cfg.interface_contact_area = dx * dx;
	try_read_env_double("MFREE_TIMESTEP_WP_MECH_SAFETY", cfg.workpiece_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_WP_THERM_SAFETY", cfg.workpiece_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_MECH_SAFETY", cfg.tool_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_THERM_SAFETY", cfg.tool_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_SAFETY", cfg.interface_thermal_safety);
	try_read_env_double("MFREE_THERMAL_H_FULL", cfg.contact_conductance_full);
	double area_factor = 1.0;
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_AREA_FACTOR", area_factor);
	if (std::isfinite(area_factor) && area_factor > 0.) cfg.interface_contact_area *= area_factor;

	coupled_timestep_limits limits = estimate_coupled_timestep(pc, cfg, ft);
	int print_limits = 1;
	try_read_env_int("MFREE_TIMESTEP_PRINT", print_limits);
	if (print_limits != 0) print_coupled_timestep_limits(limits);
	return (std::isfinite(limits.maximum_dt) && limits.maximum_dt > 0.) ? limits.maximum_dt : empirical_cap;
}

static void apply_mech_fix_tags_from_env(fe_tool &ft) {
	auto apply_tag_list = [&](const char *env_key, auto apply_tag) -> bool {
		const char *tags = getenv(env_key);
		if (!tags || tags[0] == '\0') return false;
		std::string s(tags);
		std::size_t i = 0;
		bool any = false;
		while (i < s.size()) {
			while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
			if (i >= s.size()) break;
			std::size_t j = i;
			while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
			std::string tok = s.substr(i, j - i);
			errno = 0;
			char *end = nullptr;
			long v = std::strtol(tok.c_str(), &end, 10);
			if (end != tok.c_str() && end != nullptr && *end == '\0' && errno == 0) {
				if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max() && v != 0) {
					apply_tag(static_cast<int>(v));
				any = true;
				}
			}
			i = j;
		}
		return any;
	};

	bool any = false;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_Y_TAGS", [&](int tag) { ft.set_mechanics_fixed_y_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_X_TAGS", [&](int tag) { ft.set_mechanics_fixed_x_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_TAGS", [&](int tag) { ft.set_mechanics_fixed_on_physical(tag); }) || any;
	if (any) {
		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		int anchor_tag = 0;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_TAG", v)) anchor_tag = v;
		}
		if (anchor_ux && anchor_tag != 0) {
			std::unordered_set<unsigned int> nodes;
			for (const auto &e : ft.boundary_edges()) {
				if (e.physical_tag != anchor_tag) continue;
				nodes.insert(e.n0);
				nodes.insert(e.n1);
			}
			if (!nodes.empty()) {
				unsigned int anchor = *nodes.begin();
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : nodes) {
					glm::dvec2 pw = ft.node_world(n);
					if (!std::isfinite(pw.x)) continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft.set_mechanics_fixed_x_nodes({anchor});
			}
		}
		return;
	}

	const char *tags = getenv("MFREE_FE_TOOL_FIX_TAGS");
	if (!tags || tags[0] == '\0') {
		std::unordered_set<unsigned int> bnodes;
		for (const auto &e : ft.boundary_edges()) {
			bnodes.insert(e.n0);
			bnodes.insert(e.n1);
		}
		if (bnodes.empty()) return;

		double x_max = -std::numeric_limits<double>::infinity();
		double x_min = std::numeric_limits<double>::infinity();
		for (unsigned int i : bnodes) x_max = std::max(x_max, ft.nodes_tool_frame()[i].x);
		for (unsigned int i : bnodes) x_min = std::min(x_min, ft.nodes_tool_frame()[i].x);

		std::vector<unsigned int> fixed;
		double width = x_max - x_min;
		double tol = 0.01 * width;
		try_read_env_double("MFREE_FE_TOOL_FIX_X_TOL", tol);
		if (!std::isfinite(tol) || tol <= 0.) tol = 0.01 * width;

		for (int attempt = 0; attempt < 4; attempt++) {
			fixed.clear();
			for (unsigned int i : bnodes) {
				if (ft.nodes_tool_frame()[i].x >= x_max - tol) fixed.push_back(i);
			}
			if (fixed.size() >= 2) break;
			tol *= 5.0;
		}
		ft.set_mechanics_fixed_nodes(fixed);
		return;
	}
	std::string s(tags);
	std::size_t i = 0;
	while (i < s.size()) {
		while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
		if (i >= s.size()) break;
		std::size_t j = i;
		while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
		int tag = std::atoi(s.substr(i, j - i).c_str());
		if (tag != 0) ft.set_mechanics_fixed_on_physical(tag);
		i = j;
	}
}

static std::vector<glm::dvec2> extract_boundary_loop_world(const fe_tool &ft) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &edges = ft.boundary_edges();

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(edges.size());

	for (const auto &e : edges) {
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
	}

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);

	for (const auto &kv : adj) {
		unsigned int idx = kv.first;
		if (idx >= nodes.size()) continue;
		glm::dvec2 p = ft.to_world_frame(nodes[idx]);
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = idx;
			start_p = p;
		}
	}

	if (!has_start) return {};

	std::vector<unsigned int> loop;
	loop.reserve(adj.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(adj.size()) + 2; it++) {
		loop.push_back(cur);
		const auto &nb = adj[cur];
		if (nb.empty()) break;
		unsigned int next = nb[0];
		if (nb.size() >= 2 && next == prev) next = nb[1];
		if (next == start) break;
		prev = cur;
		cur = next;
	}

	std::vector<glm::dvec2> pts;
	pts.reserve(loop.size());
	for (unsigned int idx : loop) {
		if (idx >= nodes.size()) continue;
		pts.push_back(ft.to_world_frame(nodes[idx]));
	}
	return pts;
}

static glm::dvec2 compute_nominal_tool_center(glm::dvec2 tl, double length, double height, double rake_angle, double clearance_angle) {
	glm::dvec2 tr(tl.x + length, tl.y);
	glm::dvec2 bl(tl.x, tl.y - height);

	double alpha_rake = rake_angle * M_PI / 180.;
	double alpha_free = (180 - 90 - clearance_angle) * M_PI / 180.;

	glm::dmat2x2 rot_rake(cos(alpha_rake), -sin(alpha_rake), sin(alpha_rake), cos(alpha_rake));
	glm::dmat2x2 rot_free(cos(alpha_free), -sin(alpha_free), sin(alpha_free), cos(alpha_free));

	glm::dvec2 down(0., -1.);
	glm::dvec2 trc = tr + down * rot_rake;
	glm::dvec2 blc = bl + down * rot_free;

	double x1 = tr.x, y1 = tr.y;
	double x2 = trc.x, y2 = trc.y;
	double x3 = bl.x, y3 = bl.y;
	double x4 = blc.x, y4 = blc.y;
	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	glm::dvec2 br = tr;
	if (std::isfinite(denom) && std::abs(denom) > 1e-30) {
		double d12 = x1 * y2 - y1 * x2;
		double d34 = x3 * y4 - y3 * x4;
		double px = (d12 * (x3 - x4) - (x1 - x2) * d34) / denom;
		double py = (d12 * (y3 - y4) - (y1 - y2) * d34) / denom;
		if (std::isfinite(px) && std::isfinite(py)) br = glm::dvec2(px, py);
	}

	return 0.25 * (tl + tr + br + bl);
}

static glm::dvec2 closest_point_on_segment(glm::dvec2 p, glm::dvec2 a, glm::dvec2 b) {
	glm::dvec2 ab = b - a;
	double ab2 = ab.x * ab.x + ab.y * ab.y;
	if (!(ab2 > 0.0) || !std::isfinite(ab2)) return a;
	double t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
	if (!std::isfinite(t)) t = 0.0;
	t = std::max(0.0, std::min(1.0, t));
	return a + t * ab;
}

static glm::dvec2 closest_point_on_polyline(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	glm::dvec2 best(0.);
	double best_d2 = std::numeric_limits<double>::infinity();
	if (poly.size() < 2) return best;
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 cp = closest_point_on_segment(p, a, b);
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best = cp;
		}
	}
	return best;
}

static double poly_min_y(const std::vector<glm::dvec2> &poly) {
	double low = std::numeric_limits<double>::infinity();
	for (const auto &p : poly) low = std::min(low, p.y);
	if (!std::isfinite(low)) return 0.0;
	return low;
}

static void enforce_fe_tool_corner_clearance(fe_tool &ft, glm::dvec2 wp_corner, double clearance_target_m, unsigned int iters) {
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;

		double y_bottom = poly_min_y(poly);
		double dy = (wp_corner.y - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft.get_pos();
		pos.y += dy;
		ft.set_pose(pos, ft.get_vel());

		poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;
		glm::dvec2 cp = closest_point_on_polyline(wp_corner, poly);
		double dx = wp_corner.x - cp.x;
		pos = ft.get_pos();
		pos.x += dx;
		ft.set_pose(pos, ft.get_vel());
	}
}

static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
                                     glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def)) msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
			exit(1);
		}
	}

	fe_tool *ft = new fe_tool();
	if (!ft->load_gmsh_msh2(msh)) {
		delete ft;
		std::fprintf(stderr, "Failed to load MFREE_FE_TOOL_MSH\n");
		exit(1);
	}

	fe_tool::thermal_material mat;
	mat.rho = 14500.0;
	mat.cp = 200.0;
	mat.k = 80.0;
	try_read_env_double("MFREE_FE_TOOL_RHO", mat.rho);
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_RHO_TABLE", T, v)) ft->set_material_table_rho(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_CP_TABLE", T, v)) ft->set_material_table_cp(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_K_TABLE", T, v)) ft->set_material_table_k(std::move(T), std::move(v));
	}

	fe_tool::mechanical_material mech;
	mech.E = 600e9;
	mech.nu = 0.22;
	mech.alpha = 4.5e-6;
	try_read_env_double("MFREE_FE_TOOL_E", mech.E);
	try_read_env_double("MFREE_FE_TOOL_NU", mech.nu);
	try_read_env_double("MFREE_FE_TOOL_ALPHA", mech.alpha);
	ft->set_mechanical_material(mech);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_E_TABLE", T, v)) ft->set_mechanical_table_E(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_NU_TABLE", T, v)) ft->set_mechanical_table_nu(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_ALPHA_TABLE", T, v)) ft->set_mechanical_table_alpha(std::move(T), std::move(v));
	}
	ft->set_reference_temperature(T0);
	bool bc_validate = false;
	{
		int v = 0;
		if (try_read_env_int("MFREE_FE_BC_VALIDATE", v) && v != 0) bc_validate = true;
	}
	if (!bc_validate) {
		apply_mech_fix_tags_from_env(*ft);
	}

	ft->set_initial_temperature(T0);
	glm::dvec2 pos(0.);
	const char *align_env = getenv("MFREE_FE_TOOL_ALIGN_CENTER");
	bool align = true;
	if (align_env) align = (atoi(align_env) != 0);
	if (align) {
		glm::dvec2 mesh_center(0.);
		const auto &nodes = ft->nodes_tool_frame();
		for (const auto &p : nodes) mesh_center += p;
		if (!nodes.empty()) mesh_center /= static_cast<double>(nodes.size());
		pos = desired_center - mesh_center;
	} else {
		try_read_env_double("MFREE_FE_TOOL_POS_X", pos.x);
		try_read_env_double("MFREE_FE_TOOL_POS_Y", pos.y);
	}
	ft->set_pose(pos, desired_vel);

	fe_tool::convection_bc air;
	air.h = 20.0;
	air.T_inf = 298.15;

	fe_tool::convection_bc water;
	water.h = 5000.0;
	water.T_inf = 293.15;

	double y_thresh = desired_edge_y;
	const char *y_env = getenv("MFREE_COOLANT_Y_THRESHOLD");
	if (y_env) y_thresh = atof(y_env);
	ft->set_convection_flooded_by_y(air, water, y_thresh);

	enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target_m, 5);

	if (bc_validate) {
		int top_tag = 110;
		int rear_tag = 114;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_TOP_TAG", v)) top_tag = v;
			if (try_read_env_int("MFREE_FE_BC_REAR_TAG", v)) rear_tag = v;
		}
		double Tamb_C = 25.0;
		try_read_env_double("MFREE_FE_BC_AMBIENT_C", Tamb_C);
		double Tamb_K = Tamb_C + 273.15;

		ft->clear_mechanics_fixed();
		ft->clear_mechanics_fixed_nodes();
		bool top_found = false;
		bool rear_found = false;
		for (const auto &e : ft->boundary_edges()) {
			if (e.physical_tag == top_tag) top_found = true;
			if (e.physical_tag == rear_tag) rear_found = true;
		}
		if (top_found) ft->set_mechanics_fixed_y_on_physical(top_tag);
		else std::fprintf(stderr, "warning: FE BC validation top_tag=%d not found in FE tool boundary edges\n", top_tag);
		if (rear_found) ft->set_mechanics_fixed_y_on_physical(rear_tag);
		else std::fprintf(stderr, "warning: FE BC validation rear_tag=%d not found in FE tool boundary edges\n", rear_tag);

		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		if (anchor_ux && rear_found) {
			std::unordered_set<unsigned int> rear_nodes;
			for (const auto &e : ft->boundary_edges()) {
				if (e.physical_tag != rear_tag) continue;
				rear_nodes.insert(e.n0);
				rear_nodes.insert(e.n1);
			}
			if (rear_nodes.empty()) {
				std::fprintf(stderr, "warning: FE BC validation rear_tag=%d has no nodes; skipping UX anchor\n", rear_tag);
			} else {
			unsigned int anchor = 0;
			double best_x = -std::numeric_limits<double>::infinity();
			for (unsigned int n : rear_nodes) {
				glm::dvec2 pw = ft->node_world(n);
				if (!std::isfinite(pw.x)) continue;
				if (pw.x > best_x) {
					best_x = pw.x;
					anchor = n;
				}
			}
			ft->set_mechanics_fixed_x_nodes({anchor});
			}
		}

		if (top_found) ft->set_dirichlet_on_physical(top_tag, Tamb_K);
		if (rear_found) ft->set_dirichlet_on_physical(rear_tag, Tamb_K);
	}

	return ft;
}

 body *cutting_ref_mr(unsigned int ny) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_cm_musec_g();

	double speed = 83.333328*1e-5;
	double mu_fric = 0.35;

	double hi_x = 0.100; double hi_y =  0.060;
	double lo_x = 0.000; double lo_y =  0.030;

	double dy = (hi_y-lo_y)/(ny-1);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx;
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx*ny);

	particle *particles = new particle[nx*ny];

	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	double rho0 = physical_constants.rho0();
	double T0 = physical_constants.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

		// fix bottom
		particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants correction_constants(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(physical_constants, correction_constants);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(physical_constants)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.) feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	return b;
}

 body *cutting_ref_single_resol(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 1 & 4 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;

	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nx*ny];

	srand(0);
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dx);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}

	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}

		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, nx*ny);
	printf("<<< single-resolution simulation >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_apriori(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.1*dxh)) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +++++++++++++++++
			   +               +
			   +-------CL------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=py_split) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			/*
			 * low res
			 *

			   +-------CL------+
			   +               +
			   +++++++++++++++++

			 */

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	printf("n_single_resolution=%d   n_multi_resolution=%d   \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
			};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// Keep Model 3's seeded refined block on the same lattice as the high-resolution particles.
	// The old literal limit (0.000117 m) landed between columns for the default 61-layer setup and
	// left a visible clearance strip at the refinement front. Snap to an integer dxh column instead.
	double initial_refined_x_max = dxh * std::ceil((0.000117 - 1e-12) / dxh);

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.9*dxh) || pxh>initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +---------------+
			   + HR |          +
			   +-----    LR    +
			   +               +
			   +---------------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=(py_split-1.9*dxh) && pxl<=initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	// slight modification for reserved CHILD particles!
	for (unsigned int i = n; i < nxh*nyh; i++) {
		particles[part_iter] = particle(part_iter);
	}

	printf("n_single_resolution=%d   n_current=%d  \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// adaptivity settings

	// default settings +-+-++-+-+-+-+-+-+-+-+-+-+-
	double alpha_dx = 0.50;
	double beta_h = 0.50;
	double v_cr = 0.40;
	double div_v_cr = 2e+5;
	double SvM_cr = 1e+7;
	double eps_cr = 110;
	double T_cr = 700.;
	glm::dvec2 xy_min = {0.25, 0.25};
	glm::dvec2 xy_max = {0.75, 0.75};
	double frame_width =  0.000350;
	double frame_height = 0.000060;
	unsigned int n_nbh = 10;
	double l_eff = lc + 0.1*lx;
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	adaptivity *adapt = new adaptivity(alpha_dx, beta_h, v_cr, div_v_cr, SvM_cr, eps_cr,
									   T_cr, xy_min, xy_max, frame_width, frame_height,
									   n_nbh, l_eff, true);

	adapt->set_refine_criterion(adaptivity::refine_criteria::moving_frame);
	adapt->set_refine_pattern(adaptivity::pattern::cubic_basic);

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);
	b->set_adaptivity(adapt);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< dynamic refinement model >>>\n");

	return b;
}

```


## Assistant

<thinking>
**Refining calculations for dt**

I’m thinking about replacing the dt calculation with a manual approach before introducing the ft tool. The plan is to do initial estimations without tools, which might not be critical, and then refine it after attachment. I need to replace manual dt with an estimator and call set_dt afterwards while ensuring the time is appropriately set. I should also check if MR uses centimeters and whether it’s safe to skip to SI units since the user mentioned the FE tool.
</thinking>

**Tool Call: src\\benchmarks\\test\_cuttings.cpp**
Status: Failed

Diff: src\benchmarks\test_cuttings.cpp
```
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

#include "test_cuttings.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <cerrno>

static bool try_read_env_double(const char *key, double &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	char *end = nullptr;
	double v = strtod(s, &end);
	if (end == s || !std::isfinite(v)) return false;
	out = v;
	return true;
}

static bool try_read_env_int(const char *key, int &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;
	if (!*s) return false;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s) return false;
	while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
	if (*end != '\0') return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_read_env_table(const char *key, std::vector<double> &T_out, std::vector<double> &v_out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r') ++p;
		if (!*p) break;

		char *end = nullptr;
		double T = strtod(p, &end);
		if (end == p || !std::isfinite(T)) return false;
		p = end;

		while (*p == ' ' || *p == '\t') ++p;
		if (*p != ':' && *p != '=') return false;
		++p;
		while (*p == ' ' || *p == '\t') ++p;

		end = nullptr;
		double v = strtod(p, &end);
		if (end == p || !std::isfinite(v)) return false;
		p = end;

		pairs.push_back({T, v});
	}

	if (pairs.size() < 2) return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

	T_out.clear();
	v_out.clear();
	for (const auto &kv : pairs) {
		if (!T_out.empty() && kv.first == T_out.back()) {
			v_out.back() = kv.second;
			continue;
		}
		T_out.push_back(kv.first);
		v_out.push_back(kv.second);
	}
	return T_out.size() >= 2;
}

static void adjust_workpiece_y_bounds_for_feed(double base_lo_y, double base_hi_y, unsigned int base_ny,
											   double target_feed, unsigned int safety_layers,
											   double &lo_y, double &hi_y, unsigned int &ny, double &dy) {
	hi_y = base_hi_y;
	double base_thickness = base_hi_y - base_lo_y;
	dy = base_thickness / (base_ny - 1);
	double required_thickness = target_feed + safety_layers * dy;

	unsigned int extra_layers = 0;
	if (required_thickness > base_thickness) {
		double raw = (required_thickness - base_thickness) / dy;
		extra_layers = (unsigned int)std::ceil(raw - 1e-12);
	}
	ny = base_ny + extra_layers;
	lo_y = hi_y - (ny - 1) * dy;
}

static double read_coupled_motion_ratio() {
	const char *enable_env = getenv("MFREE_COUPLED_MOTION");
	bool enabled = (enable_env && atoi(enable_env) != 0);
	if (!enabled) return 1.0;

	double ratio = 1.0;
	if (try_read_env_double("MFREE_COUPLED_MOTION_RATIO", ratio)) {
		if (!std::isfinite(ratio)) ratio = 1.0;
		ratio = std::max(0.0, std::min(1.0, ratio));
		return ratio;
	}

	const char *primary = getenv("MFREE_PRIMARY_MOVING_BODY");
	if (!primary || primary[0] == '\0') return 1.0;
	std::string s(primary);
	for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s == "workpiece" || s == "wp") return 0.0;
	if (s == "both" || s == "coupled") return 0.5;
	return 1.0;
}

static double estimate_dt_for_cutting(const physical_constants &pc, double dx, double hdx, double relative_speed, double empirical_cap, const fe_tool *ft) {
	coupled_timestep_config cfg;
	cfg.particle_spacing = dx;
	cfg.smoothing_length_ratio = hdx;
	cfg.max_relative_speed = relative_speed;
	cfg.empirical_dt_cap = empirical_cap;
	cfg.interface_contact_area = dx * dx;
	try_read_env_double("MFREE_TIMESTEP_WP_MECH_SAFETY", cfg.workpiece_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_WP_THERM_SAFETY", cfg.workpiece_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_MECH_SAFETY", cfg.tool_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_THERM_SAFETY", cfg.tool_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_SAFETY", cfg.interface_thermal_safety);
	try_read_env_double("MFREE_THERMAL_H_FULL", cfg.contact_conductance_full);
	double area_factor = 1.0;
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_AREA_FACTOR", area_factor);
	if (std::isfinite(area_factor) && area_factor > 0.) cfg.interface_contact_area *= area_factor;

	coupled_timestep_limits limits = estimate_coupled_timestep(pc, cfg, ft);
	int print_limits = 1;
	try_read_env_int("MFREE_TIMESTEP_PRINT", print_limits);
	if (print_limits != 0) print_coupled_timestep_limits(limits);
	return (std::isfinite(limits.maximum_dt) && limits.maximum_dt > 0.) ? limits.maximum_dt : empirical_cap;
}

static void apply_mech_fix_tags_from_env(fe_tool &ft) {
	auto apply_tag_list = [&](const char *env_key, auto apply_tag) -> bool {
		const char *tags = getenv(env_key);
		if (!tags || tags[0] == '\0') return false;
		std::string s(tags);
		std::size_t i = 0;
		bool any = false;
		while (i < s.size()) {
			while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
			if (i >= s.size()) break;
			std::size_t j = i;
			while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
			std::string tok = s.substr(i, j - i);
			errno = 0;
			char *end = nullptr;
			long v = std::strtol(tok.c_str(), &end, 10);
			if (end != tok.c_str() && end != nullptr && *end == '\0' && errno == 0) {
				if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max() && v != 0) {
					apply_tag(static_cast<int>(v));
				any = true;
				}
			}
			i = j;
		}
		return any;
	};

	bool any = false;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_Y_TAGS", [&](int tag) { ft.set_mechanics_fixed_y_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_X_TAGS", [&](int tag) { ft.set_mechanics_fixed_x_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_TAGS", [&](int tag) { ft.set_mechanics_fixed_on_physical(tag); }) || any;
	if (any) {
		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		int anchor_tag = 0;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_TAG", v)) anchor_tag = v;
		}
		if (anchor_ux && anchor_tag != 0) {
			std::unordered_set<unsigned int> nodes;
			for (const auto &e : ft.boundary_edges()) {
				if (e.physical_tag != anchor_tag) continue;
				nodes.insert(e.n0);
				nodes.insert(e.n1);
			}
			if (!nodes.empty()) {
				unsigned int anchor = *nodes.begin();
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : nodes) {
					glm::dvec2 pw = ft.node_world(n);
					if (!std::isfinite(pw.x)) continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft.set_mechanics_fixed_x_nodes({anchor});
			}
		}
		return;
	}

	const char *tags = getenv("MFREE_FE_TOOL_FIX_TAGS");
	if (!tags || tags[0] == '\0') {
		std::unordered_set<unsigned int> bnodes;
		for (const auto &e : ft.boundary_edges()) {
			bnodes.insert(e.n0);
			bnodes.insert(e.n1);
		}
		if (bnodes.empty()) return;

		double x_max = -std::numeric_limits<double>::infinity();
		double x_min = std::numeric_limits<double>::infinity();
		for (unsigned int i : bnodes) x_max = std::max(x_max, ft.nodes_tool_frame()[i].x);
		for (unsigned int i : bnodes) x_min = std::min(x_min, ft.nodes_tool_frame()[i].x);

		std::vector<unsigned int> fixed;
		double width = x_max - x_min;
		double tol = 0.01 * width;
		try_read_env_double("MFREE_FE_TOOL_FIX_X_TOL", tol);
		if (!std::isfinite(tol) || tol <= 0.) tol = 0.01 * width;

		for (int attempt = 0; attempt < 4; attempt++) {
			fixed.clear();
			for (unsigned int i : bnodes) {
				if (ft.nodes_tool_frame()[i].x >= x_max - tol) fixed.push_back(i);
			}
			if (fixed.size() >= 2) break;
			tol *= 5.0;
		}
		ft.set_mechanics_fixed_nodes(fixed);
		return;
	}
	std::string s(tags);
	std::size_t i = 0;
	while (i < s.size()) {
		while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
		if (i >= s.size()) break;
		std::size_t j = i;
		while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
		int tag = std::atoi(s.substr(i, j - i).c_str());
		if (tag != 0) ft.set_mechanics_fixed_on_physical(tag);
		i = j;
	}
}

static std::vector<glm::dvec2> extract_boundary_loop_world(const fe_tool &ft) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &edges = ft.boundary_edges();

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(edges.size());

	for (const auto &e : edges) {
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
	}

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);

	for (const auto &kv : adj) {
		unsigned int idx = kv.first;
		if (idx >= nodes.size()) continue;
		glm::dvec2 p = ft.to_world_frame(nodes[idx]);
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = idx;
			start_p = p;
		}
	}

	if (!has_start) return {};

	std::vector<unsigned int> loop;
	loop.reserve(adj.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(adj.size()) + 2; it++) {
		loop.push_back(cur);
		const auto &nb = adj[cur];
		if (nb.empty()) break;
		unsigned int next = nb[0];
		if (nb.size() >= 2 && next == prev) next = nb[1];
		if (next == start) break;
		prev = cur;
		cur = next;
	}

	std::vector<glm::dvec2> pts;
	pts.reserve(loop.size());
	for (unsigned int idx : loop) {
		if (idx >= nodes.size()) continue;
		pts.push_back(ft.to_world_frame(nodes[idx]));
	}
	return pts;
}

static glm::dvec2 compute_nominal_tool_center(glm::dvec2 tl, double length, double height, double rake_angle, double clearance_angle) {
	glm::dvec2 tr(tl.x + length, tl.y);
	glm::dvec2 bl(tl.x, tl.y - height);

	double alpha_rake = rake_angle * M_PI / 180.;
	double alpha_free = (180 - 90 - clearance_angle) * M_PI / 180.;

	glm::dmat2x2 rot_rake(cos(alpha_rake), -sin(alpha_rake), sin(alpha_rake), cos(alpha_rake));
	glm::dmat2x2 rot_free(cos(alpha_free), -sin(alpha_free), sin(alpha_free), cos(alpha_free));

	glm::dvec2 down(0., -1.);
	glm::dvec2 trc = tr + down * rot_rake;
	glm::dvec2 blc = bl + down * rot_free;

	double x1 = tr.x, y1 = tr.y;
	double x2 = trc.x, y2 = trc.y;
	double x3 = bl.x, y3 = bl.y;
	double x4 = blc.x, y4 = blc.y;
	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	glm::dvec2 br = tr;
	if (std::isfinite(denom) && std::abs(denom) > 1e-30) {
		double d12 = x1 * y2 - y1 * x2;
		double d34 = x3 * y4 - y3 * x4;
		double px = (d12 * (x3 - x4) - (x1 - x2) * d34) / denom;
		double py = (d12 * (y3 - y4) - (y1 - y2) * d34) / denom;
		if (std::isfinite(px) && std::isfinite(py)) br = glm::dvec2(px, py);
	}

	return 0.25 * (tl + tr + br + bl);
}

static glm::dvec2 closest_point_on_segment(glm::dvec2 p, glm::dvec2 a, glm::dvec2 b) {
	glm::dvec2 ab = b - a;
	double ab2 = ab.x * ab.x + ab.y * ab.y;
	if (!(ab2 > 0.0) || !std::isfinite(ab2)) return a;
	double t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
	if (!std::isfinite(t)) t = 0.0;
	t = std::max(0.0, std::min(1.0, t));
	return a + t * ab;
}

static glm::dvec2 closest_point_on_polyline(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	glm::dvec2 best(0.);
	double best_d2 = std::numeric_limits<double>::infinity();
	if (poly.size() < 2) return best;
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 cp = closest_point_on_segment(p, a, b);
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best = cp;
		}
	}
	return best;
}

static double poly_min_y(const std::vector<glm::dvec2> &poly) {
	double low = std::numeric_limits<double>::infinity();
	for (const auto &p : poly) low = std::min(low, p.y);
	if (!std::isfinite(low)) return 0.0;
	return low;
}

static void enforce_fe_tool_corner_clearance(fe_tool &ft, glm::dvec2 wp_corner, double clearance_target_m, unsigned int iters) {
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;

		double y_bottom = poly_min_y(poly);
		double dy = (wp_corner.y - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft.get_pos();
		pos.y += dy;
		ft.set_pose(pos, ft.get_vel());

		poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;
		glm::dvec2 cp = closest_point_on_polyline(wp_corner, poly);
		double dx = wp_corner.x - cp.x;
		pos = ft.get_pos();
		pos.x += dx;
		ft.set_pose(pos, ft.get_vel());
	}
}

static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
                                     glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def)) msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
			exit(1);
		}
	}

	fe_tool *ft = new fe_tool();
	if (!ft->load_gmsh_msh2(msh)) {
		delete ft;
		std::fprintf(stderr, "Failed to load MFREE_FE_TOOL_MSH\n");
		exit(1);
	}

	fe_tool::thermal_material mat;
	mat.rho = 14500.0;
	mat.cp = 200.0;
	mat.k = 80.0;
	try_read_env_double("MFREE_FE_TOOL_RHO", mat.rho);
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_RHO_TABLE", T, v)) ft->set_material_table_rho(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_CP_TABLE", T, v)) ft->set_material_table_cp(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_K_TABLE", T, v)) ft->set_material_table_k(std::move(T), std::move(v));
	}

	fe_tool::mechanical_material mech;
	mech.E = 600e9;
	mech.nu = 0.22;
	mech.alpha = 4.5e-6;
	try_read_env_double("MFREE_FE_TOOL_E", mech.E);
	try_read_env_double("MFREE_FE_TOOL_NU", mech.nu);
	try_read_env_double("MFREE_FE_TOOL_ALPHA", mech.alpha);
	ft->set_mechanical_material(mech);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_E_TABLE", T, v)) ft->set_mechanical_table_E(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_NU_TABLE", T, v)) ft->set_mechanical_table_nu(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_ALPHA_TABLE", T, v)) ft->set_mechanical_table_alpha(std::move(T), std::move(v));
	}
	ft->set_reference_temperature(T0);
	bool bc_validate = false;
	{
		int v = 0;
		if (try_read_env_int("MFREE_FE_BC_VALIDATE", v) && v != 0) bc_validate = true;
	}
	if (!bc_validate) {
		apply_mech_fix_tags_from_env(*ft);
	}

	ft->set_initial_temperature(T0);
	glm::dvec2 pos(0.);
	const char *align_env = getenv("MFREE_FE_TOOL_ALIGN_CENTER");
	bool align = true;
	if (align_env) align = (atoi(align_env) != 0);
	if (align) {
		glm::dvec2 mesh_center(0.);
		const auto &nodes = ft->nodes_tool_frame();
		for (const auto &p : nodes) mesh_center += p;
		if (!nodes.empty()) mesh_center /= static_cast<double>(nodes.size());
		pos = desired_center - mesh_center;
	} else {
		try_read_env_double("MFREE_FE_TOOL_POS_X", pos.x);
		try_read_env_double("MFREE_FE_TOOL_POS_Y", pos.y);
	}
	ft->set_pose(pos, desired_vel);

	fe_tool::convection_bc air;
	air.h = 20.0;
	air.T_inf = 298.15;

	fe_tool::convection_bc water;
	water.h = 5000.0;
	water.T_inf = 293.15;

	double y_thresh = desired_edge_y;
	const char *y_env = getenv("MFREE_COOLANT_Y_THRESHOLD");
	if (y_env) y_thresh = atof(y_env);
	ft->set_convection_flooded_by_y(air, water, y_thresh);

	enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target_m, 5);

	if (bc_validate) {
		int top_tag = 110;
		int rear_tag = 114;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_TOP_TAG", v)) top_tag = v;
			if (try_read_env_int("MFREE_FE_BC_REAR_TAG", v)) rear_tag = v;
		}
		double Tamb_C = 25.0;
		try_read_env_double("MFREE_FE_BC_AMBIENT_C", Tamb_C);
		double Tamb_K = Tamb_C + 273.15;

		ft->clear_mechanics_fixed();
		ft->clear_mechanics_fixed_nodes();
		bool top_found = false;
		bool rear_found = false;
		for (const auto &e : ft->boundary_edges()) {
			if (e.physical_tag == top_tag) top_found = true;
			if (e.physical_tag == rear_tag) rear_found = true;
		}
		if (top_found) ft->set_mechanics_fixed_y_on_physical(top_tag);
		else std::fprintf(stderr, "warning: FE BC validation top_tag=%d not found in FE tool boundary edges\n", top_tag);
		if (rear_found) ft->set_mechanics_fixed_y_on_physical(rear_tag);
		else std::fprintf(stderr, "warning: FE BC validation rear_tag=%d not found in FE tool boundary edges\n", rear_tag);

		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		if (anchor_ux && rear_found) {
			std::unordered_set<unsigned int> rear_nodes;
			for (const auto &e : ft->boundary_edges()) {
				if (e.physical_tag != rear_tag) continue;
				rear_nodes.insert(e.n0);
				rear_nodes.insert(e.n1);
			}
			if (rear_nodes.empty()) {
				std::fprintf(stderr, "warning: FE BC validation rear_tag=%d has no nodes; skipping UX anchor\n", rear_tag);
			} else {
			unsigned int anchor = 0;
			double best_x = -std::numeric_limits<double>::infinity();
			for (unsigned int n : rear_nodes) {
				glm::dvec2 pw = ft->node_world(n);
				if (!std::isfinite(pw.x)) continue;
				if (pw.x > best_x) {
					best_x = pw.x;
					anchor = n;
				}
			}
			ft->set_mechanics_fixed_x_nodes({anchor});
			}
		}

		if (top_found) ft->set_dirichlet_on_physical(top_tag, Tamb_K);
		if (rear_found) ft->set_dirichlet_on_physical(rear_tag, Tamb_K);
	}

	return ft;
}

 body *cutting_ref_mr(unsigned int ny) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_cm_musec_g();

	double speed = 83.333328*1e-5;
	double mu_fric = 0.35;

	double hi_x = 0.100; double hi_y =  0.060;
	double lo_x = 0.000; double lo_y =  0.030;

	double dy = (hi_y-lo_y)/(ny-1);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx;
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx*ny);

	particle *particles = new particle[nx*ny];

	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	double rho0 = physical_constants.rho0();
	double T0 = physical_constants.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

		// fix bottom
		particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants correction_constants(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(physical_constants, correction_constants);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(physical_constants)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.) feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	return b;
}

 body *cutting_ref_single_resol(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 1 & 4 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;

	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nx*ny];

	srand(0);
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dx);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}

	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}

		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, nx*ny);
	printf("<<< single-resolution simulation >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_apriori(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.1*dxh)) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +++++++++++++++++
			   +               +
			   +-------CL------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=py_split) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			/*
			 * low res
			 *

			   +-------CL------+
			   +               +
			   +++++++++++++++++

			 */

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	printf("n_single_resolution=%d   n_multi_resolution=%d   \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
			};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// Keep Model 3's seeded refined block on the same lattice as the high-resolution particles.
	// The old literal limit (0.000117 m) landed between columns for the default 61-layer setup and
	// left a visible clearance strip at the refinement front. Snap to an integer dxh column instead.
	double initial_refined_x_max = dxh * std::ceil((0.000117 - 1e-12) / dxh);

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.9*dxh) || pxh>initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +---------------+
			   + HR |          +
			   +-----    LR    +
			   +               +
			   +---------------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=(py_split-1.9*dxh) && pxl<=initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	// slight modification for reserved CHILD particles!
	for (unsigned int i = n; i < nxh*nyh; i++) {
		particles[part_iter] = particle(part_iter);
	}

	printf("n_single_resolution=%d   n_current=%d  \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// adaptivity settings

	// default settings +-+-++-+-+-+-+-+-+-+-+-+-+-
	double alpha_dx = 0.50;
	double beta_h = 0.50;
	double v_cr = 0.40;
	double div_v_cr = 2e+5;
	double SvM_cr = 1e+7;
	double eps_cr = 110;
	double T_cr = 700.;
	glm::dvec2 xy_min = {0.25, 0.25};
	glm::dvec2 xy_max = {0.75, 0.75};
	double frame_width =  0.000350;
	double frame_height = 0.000060;
	unsigned int n_nbh = 10;
	double l_eff = lc + 0.1*lx;
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	adaptivity *adapt = new adaptivity(alpha_dx, beta_h, v_cr, div_v_cr, SvM_cr, eps_cr,
									   T_cr, xy_min, xy_max, frame_width, frame_height,
									   n_nbh, l_eff, true);

	adapt->set_refine_criterion(adaptivity::refine_criteria::moving_frame);
	adapt->set_refine_pattern(adaptivity::pattern::cubic_basic);

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);
	b->set_adaptivity(adapt);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< dynamic refinement model >>>\n");

	return b;
}

```


## Assistant

<thinking>
**Evaluating text replacement options**

I need to replace all occurrences, but I’m not sure if they’ll be the same. Can I replace them all exactly? I’m thinking about multiple occurrences. So, is the edit tool capable of handling this? It complains when trying to match multiple instances. Should I use a terminal with Perl? But I shouldn't resort to shell substitutions. Maybe a simple Python one-liner could work without complications. Alternatively, I could try to edit with a larger context around the line.
</thinking>

**Tool Call: src\\benchmarks\\test\_cuttings.cpp**
Status: Failed

Diff: src\benchmarks\test_cuttings.cpp
```
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

#include "test_cuttings.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <cerrno>

static bool try_read_env_double(const char *key, double &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	char *end = nullptr;
	double v = strtod(s, &end);
	if (end == s || !std::isfinite(v)) return false;
	out = v;
	return true;
}

static bool try_read_env_int(const char *key, int &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;
	if (!*s) return false;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s) return false;
	while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
	if (*end != '\0') return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_read_env_table(const char *key, std::vector<double> &T_out, std::vector<double> &v_out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r') ++p;
		if (!*p) break;

		char *end = nullptr;
		double T = strtod(p, &end);
		if (end == p || !std::isfinite(T)) return false;
		p = end;

		while (*p == ' ' || *p == '\t') ++p;
		if (*p != ':' && *p != '=') return false;
		++p;
		while (*p == ' ' || *p == '\t') ++p;

		end = nullptr;
		double v = strtod(p, &end);
		if (end == p || !std::isfinite(v)) return false;
		p = end;

		pairs.push_back({T, v});
	}

	if (pairs.size() < 2) return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

	T_out.clear();
	v_out.clear();
	for (const auto &kv : pairs) {
		if (!T_out.empty() && kv.first == T_out.back()) {
			v_out.back() = kv.second;
			continue;
		}
		T_out.push_back(kv.first);
		v_out.push_back(kv.second);
	}
	return T_out.size() >= 2;
}

static void adjust_workpiece_y_bounds_for_feed(double base_lo_y, double base_hi_y, unsigned int base_ny,
											   double target_feed, unsigned int safety_layers,
											   double &lo_y, double &hi_y, unsigned int &ny, double &dy) {
	hi_y = base_hi_y;
	double base_thickness = base_hi_y - base_lo_y;
	dy = base_thickness / (base_ny - 1);
	double required_thickness = target_feed + safety_layers * dy;

	unsigned int extra_layers = 0;
	if (required_thickness > base_thickness) {
		double raw = (required_thickness - base_thickness) / dy;
		extra_layers = (unsigned int)std::ceil(raw - 1e-12);
	}
	ny = base_ny + extra_layers;
	lo_y = hi_y - (ny - 1) * dy;
}

static double read_coupled_motion_ratio() {
	const char *enable_env = getenv("MFREE_COUPLED_MOTION");
	bool enabled = (enable_env && atoi(enable_env) != 0);
	if (!enabled) return 1.0;

	double ratio = 1.0;
	if (try_read_env_double("MFREE_COUPLED_MOTION_RATIO", ratio)) {
		if (!std::isfinite(ratio)) ratio = 1.0;
		ratio = std::max(0.0, std::min(1.0, ratio));
		return ratio;
	}

	const char *primary = getenv("MFREE_PRIMARY_MOVING_BODY");
	if (!primary || primary[0] == '\0') return 1.0;
	std::string s(primary);
	for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s == "workpiece" || s == "wp") return 0.0;
	if (s == "both" || s == "coupled") return 0.5;
	return 1.0;
}

static double estimate_dt_for_cutting(const physical_constants &pc, double dx, double hdx, double relative_speed, double empirical_cap, const fe_tool *ft) {
	coupled_timestep_config cfg;
	cfg.particle_spacing = dx;
	cfg.smoothing_length_ratio = hdx;
	cfg.max_relative_speed = relative_speed;
	cfg.empirical_dt_cap = empirical_cap;
	cfg.interface_contact_area = dx * dx;
	try_read_env_double("MFREE_TIMESTEP_WP_MECH_SAFETY", cfg.workpiece_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_WP_THERM_SAFETY", cfg.workpiece_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_MECH_SAFETY", cfg.tool_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_THERM_SAFETY", cfg.tool_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_SAFETY", cfg.interface_thermal_safety);
	try_read_env_double("MFREE_THERMAL_H_FULL", cfg.contact_conductance_full);
	double area_factor = 1.0;
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_AREA_FACTOR", area_factor);
	if (std::isfinite(area_factor) && area_factor > 0.) cfg.interface_contact_area *= area_factor;

	coupled_timestep_limits limits = estimate_coupled_timestep(pc, cfg, ft);
	int print_limits = 1;
	try_read_env_int("MFREE_TIMESTEP_PRINT", print_limits);
	if (print_limits != 0) print_coupled_timestep_limits(limits);
	return (std::isfinite(limits.maximum_dt) && limits.maximum_dt > 0.) ? limits.maximum_dt : empirical_cap;
}

static void apply_mech_fix_tags_from_env(fe_tool &ft) {
	auto apply_tag_list = [&](const char *env_key, auto apply_tag) -> bool {
		const char *tags = getenv(env_key);
		if (!tags || tags[0] == '\0') return false;
		std::string s(tags);
		std::size_t i = 0;
		bool any = false;
		while (i < s.size()) {
			while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
			if (i >= s.size()) break;
			std::size_t j = i;
			while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
			std::string tok = s.substr(i, j - i);
			errno = 0;
			char *end = nullptr;
			long v = std::strtol(tok.c_str(), &end, 10);
			if (end != tok.c_str() && end != nullptr && *end == '\0' && errno == 0) {
				if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max() && v != 0) {
					apply_tag(static_cast<int>(v));
				any = true;
				}
			}
			i = j;
		}
		return any;
	};

	bool any = false;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_Y_TAGS", [&](int tag) { ft.set_mechanics_fixed_y_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_X_TAGS", [&](int tag) { ft.set_mechanics_fixed_x_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_TAGS", [&](int tag) { ft.set_mechanics_fixed_on_physical(tag); }) || any;
	if (any) {
		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		int anchor_tag = 0;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_TAG", v)) anchor_tag = v;
		}
		if (anchor_ux && anchor_tag != 0) {
			std::unordered_set<unsigned int> nodes;
			for (const auto &e : ft.boundary_edges()) {
				if (e.physical_tag != anchor_tag) continue;
				nodes.insert(e.n0);
				nodes.insert(e.n1);
			}
			if (!nodes.empty()) {
				unsigned int anchor = *nodes.begin();
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : nodes) {
					glm::dvec2 pw = ft.node_world(n);
					if (!std::isfinite(pw.x)) continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft.set_mechanics_fixed_x_nodes({anchor});
			}
		}
		return;
	}

	const char *tags = getenv("MFREE_FE_TOOL_FIX_TAGS");
	if (!tags || tags[0] == '\0') {
		std::unordered_set<unsigned int> bnodes;
		for (const auto &e : ft.boundary_edges()) {
			bnodes.insert(e.n0);
			bnodes.insert(e.n1);
		}
		if (bnodes.empty()) return;

		double x_max = -std::numeric_limits<double>::infinity();
		double x_min = std::numeric_limits<double>::infinity();
		for (unsigned int i : bnodes) x_max = std::max(x_max, ft.nodes_tool_frame()[i].x);
		for (unsigned int i : bnodes) x_min = std::min(x_min, ft.nodes_tool_frame()[i].x);

		std::vector<unsigned int> fixed;
		double width = x_max - x_min;
		double tol = 0.01 * width;
		try_read_env_double("MFREE_FE_TOOL_FIX_X_TOL", tol);
		if (!std::isfinite(tol) || tol <= 0.) tol = 0.01 * width;

		for (int attempt = 0; attempt < 4; attempt++) {
			fixed.clear();
			for (unsigned int i : bnodes) {
				if (ft.nodes_tool_frame()[i].x >= x_max - tol) fixed.push_back(i);
			}
			if (fixed.size() >= 2) break;
			tol *= 5.0;
		}
		ft.set_mechanics_fixed_nodes(fixed);
		return;
	}
	std::string s(tags);
	std::size_t i = 0;
	while (i < s.size()) {
		while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
		if (i >= s.size()) break;
		std::size_t j = i;
		while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
		int tag = std::atoi(s.substr(i, j - i).c_str());
		if (tag != 0) ft.set_mechanics_fixed_on_physical(tag);
		i = j;
	}
}

static std::vector<glm::dvec2> extract_boundary_loop_world(const fe_tool &ft) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &edges = ft.boundary_edges();

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(edges.size());

	for (const auto &e : edges) {
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
	}

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);

	for (const auto &kv : adj) {
		unsigned int idx = kv.first;
		if (idx >= nodes.size()) continue;
		glm::dvec2 p = ft.to_world_frame(nodes[idx]);
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = idx;
			start_p = p;
		}
	}

	if (!has_start) return {};

	std::vector<unsigned int> loop;
	loop.reserve(adj.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(adj.size()) + 2; it++) {
		loop.push_back(cur);
		const auto &nb = adj[cur];
		if (nb.empty()) break;
		unsigned int next = nb[0];
		if (nb.size() >= 2 && next == prev) next = nb[1];
		if (next == start) break;
		prev = cur;
		cur = next;
	}

	std::vector<glm::dvec2> pts;
	pts.reserve(loop.size());
	for (unsigned int idx : loop) {
		if (idx >= nodes.size()) continue;
		pts.push_back(ft.to_world_frame(nodes[idx]));
	}
	return pts;
}

static glm::dvec2 compute_nominal_tool_center(glm::dvec2 tl, double length, double height, double rake_angle, double clearance_angle) {
	glm::dvec2 tr(tl.x + length, tl.y);
	glm::dvec2 bl(tl.x, tl.y - height);

	double alpha_rake = rake_angle * M_PI / 180.;
	double alpha_free = (180 - 90 - clearance_angle) * M_PI / 180.;

	glm::dmat2x2 rot_rake(cos(alpha_rake), -sin(alpha_rake), sin(alpha_rake), cos(alpha_rake));
	glm::dmat2x2 rot_free(cos(alpha_free), -sin(alpha_free), sin(alpha_free), cos(alpha_free));

	glm::dvec2 down(0., -1.);
	glm::dvec2 trc = tr + down * rot_rake;
	glm::dvec2 blc = bl + down * rot_free;

	double x1 = tr.x, y1 = tr.y;
	double x2 = trc.x, y2 = trc.y;
	double x3 = bl.x, y3 = bl.y;
	double x4 = blc.x, y4 = blc.y;
	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	glm::dvec2 br = tr;
	if (std::isfinite(denom) && std::abs(denom) > 1e-30) {
		double d12 = x1 * y2 - y1 * x2;
		double d34 = x3 * y4 - y3 * x4;
		double px = (d12 * (x3 - x4) - (x1 - x2) * d34) / denom;
		double py = (d12 * (y3 - y4) - (y1 - y2) * d34) / denom;
		if (std::isfinite(px) && std::isfinite(py)) br = glm::dvec2(px, py);
	}

	return 0.25 * (tl + tr + br + bl);
}

static glm::dvec2 closest_point_on_segment(glm::dvec2 p, glm::dvec2 a, glm::dvec2 b) {
	glm::dvec2 ab = b - a;
	double ab2 = ab.x * ab.x + ab.y * ab.y;
	if (!(ab2 > 0.0) || !std::isfinite(ab2)) return a;
	double t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
	if (!std::isfinite(t)) t = 0.0;
	t = std::max(0.0, std::min(1.0, t));
	return a + t * ab;
}

static glm::dvec2 closest_point_on_polyline(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	glm::dvec2 best(0.);
	double best_d2 = std::numeric_limits<double>::infinity();
	if (poly.size() < 2) return best;
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 cp = closest_point_on_segment(p, a, b);
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best = cp;
		}
	}
	return best;
}

static double poly_min_y(const std::vector<glm::dvec2> &poly) {
	double low = std::numeric_limits<double>::infinity();
	for (const auto &p : poly) low = std::min(low, p.y);
	if (!std::isfinite(low)) return 0.0;
	return low;
}

static void enforce_fe_tool_corner_clearance(fe_tool &ft, glm::dvec2 wp_corner, double clearance_target_m, unsigned int iters) {
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;

		double y_bottom = poly_min_y(poly);
		double dy = (wp_corner.y - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft.get_pos();
		pos.y += dy;
		ft.set_pose(pos, ft.get_vel());

		poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;
		glm::dvec2 cp = closest_point_on_polyline(wp_corner, poly);
		double dx = wp_corner.x - cp.x;
		pos = ft.get_pos();
		pos.x += dx;
		ft.set_pose(pos, ft.get_vel());
	}
}

static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
                                     glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def)) msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
			exit(1);
		}
	}

	fe_tool *ft = new fe_tool();
	if (!ft->load_gmsh_msh2(msh)) {
		delete ft;
		std::fprintf(stderr, "Failed to load MFREE_FE_TOOL_MSH\n");
		exit(1);
	}

	fe_tool::thermal_material mat;
	mat.rho = 14500.0;
	mat.cp = 200.0;
	mat.k = 80.0;
	try_read_env_double("MFREE_FE_TOOL_RHO", mat.rho);
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_RHO_TABLE", T, v)) ft->set_material_table_rho(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_CP_TABLE", T, v)) ft->set_material_table_cp(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_K_TABLE", T, v)) ft->set_material_table_k(std::move(T), std::move(v));
	}

	fe_tool::mechanical_material mech;
	mech.E = 600e9;
	mech.nu = 0.22;
	mech.alpha = 4.5e-6;
	try_read_env_double("MFREE_FE_TOOL_E", mech.E);
	try_read_env_double("MFREE_FE_TOOL_NU", mech.nu);
	try_read_env_double("MFREE_FE_TOOL_ALPHA", mech.alpha);
	ft->set_mechanical_material(mech);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_E_TABLE", T, v)) ft->set_mechanical_table_E(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_NU_TABLE", T, v)) ft->set_mechanical_table_nu(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_ALPHA_TABLE", T, v)) ft->set_mechanical_table_alpha(std::move(T), std::move(v));
	}
	ft->set_reference_temperature(T0);
	bool bc_validate = false;
	{
		int v = 0;
		if (try_read_env_int("MFREE_FE_BC_VALIDATE", v) && v != 0) bc_validate = true;
	}
	if (!bc_validate) {
		apply_mech_fix_tags_from_env(*ft);
	}

	ft->set_initial_temperature(T0);
	glm::dvec2 pos(0.);
	const char *align_env = getenv("MFREE_FE_TOOL_ALIGN_CENTER");
	bool align = true;
	if (align_env) align = (atoi(align_env) != 0);
	if (align) {
		glm::dvec2 mesh_center(0.);
		const auto &nodes = ft->nodes_tool_frame();
		for (const auto &p : nodes) mesh_center += p;
		if (!nodes.empty()) mesh_center /= static_cast<double>(nodes.size());
		pos = desired_center - mesh_center;
	} else {
		try_read_env_double("MFREE_FE_TOOL_POS_X", pos.x);
		try_read_env_double("MFREE_FE_TOOL_POS_Y", pos.y);
	}
	ft->set_pose(pos, desired_vel);

	fe_tool::convection_bc air;
	air.h = 20.0;
	air.T_inf = 298.15;

	fe_tool::convection_bc water;
	water.h = 5000.0;
	water.T_inf = 293.15;

	double y_thresh = desired_edge_y;
	const char *y_env = getenv("MFREE_COOLANT_Y_THRESHOLD");
	if (y_env) y_thresh = atof(y_env);
	ft->set_convection_flooded_by_y(air, water, y_thresh);

	enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target_m, 5);

	if (bc_validate) {
		int top_tag = 110;
		int rear_tag = 114;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_TOP_TAG", v)) top_tag = v;
			if (try_read_env_int("MFREE_FE_BC_REAR_TAG", v)) rear_tag = v;
		}
		double Tamb_C = 25.0;
		try_read_env_double("MFREE_FE_BC_AMBIENT_C", Tamb_C);
		double Tamb_K = Tamb_C + 273.15;

		ft->clear_mechanics_fixed();
		ft->clear_mechanics_fixed_nodes();
		bool top_found = false;
		bool rear_found = false;
		for (const auto &e : ft->boundary_edges()) {
			if (e.physical_tag == top_tag) top_found = true;
			if (e.physical_tag == rear_tag) rear_found = true;
		}
		if (top_found) ft->set_mechanics_fixed_y_on_physical(top_tag);
		else std::fprintf(stderr, "warning: FE BC validation top_tag=%d not found in FE tool boundary edges\n", top_tag);
		if (rear_found) ft->set_mechanics_fixed_y_on_physical(rear_tag);
		else std::fprintf(stderr, "warning: FE BC validation rear_tag=%d not found in FE tool boundary edges\n", rear_tag);

		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		if (anchor_ux && rear_found) {
			std::unordered_set<unsigned int> rear_nodes;
			for (const auto &e : ft->boundary_edges()) {
				if (e.physical_tag != rear_tag) continue;
				rear_nodes.insert(e.n0);
				rear_nodes.insert(e.n1);
			}
			if (rear_nodes.empty()) {
				std::fprintf(stderr, "warning: FE BC validation rear_tag=%d has no nodes; skipping UX anchor\n", rear_tag);
			} else {
			unsigned int anchor = 0;
			double best_x = -std::numeric_limits<double>::infinity();
			for (unsigned int n : rear_nodes) {
				glm::dvec2 pw = ft->node_world(n);
				if (!std::isfinite(pw.x)) continue;
				if (pw.x > best_x) {
					best_x = pw.x;
					anchor = n;
				}
			}
			ft->set_mechanics_fixed_x_nodes({anchor});
			}
		}

		if (top_found) ft->set_dirichlet_on_physical(top_tag, Tamb_K);
		if (rear_found) ft->set_dirichlet_on_physical(rear_tag, Tamb_K);
	}

	return ft;
}

 body *cutting_ref_mr(unsigned int ny) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_cm_musec_g();

	double speed = 83.333328*1e-5;
	double mu_fric = 0.35;

	double hi_x = 0.100; double hi_y =  0.060;
	double lo_x = 0.000; double lo_y =  0.030;

	double dy = (hi_y-lo_y)/(ny-1);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx;
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx*ny);

	particle *particles = new particle[nx*ny];

	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	double rho0 = physical_constants.rho0();
	double T0 = physical_constants.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

		// fix bottom
		particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants correction_constants(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(physical_constants, correction_constants);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(physical_constants)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.) feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	return b;
}

 body *cutting_ref_single_resol(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 1 & 4 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;

	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nx*ny];

	srand(0);
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dx);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}

	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}

		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, nx*ny);
	printf("<<< single-resolution simulation >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_apriori(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.1*dxh)) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +++++++++++++++++
			   +               +
			   +-------CL------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=py_split) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			/*
			 * low res
			 *

			   +-------CL------+
			   +               +
			   +++++++++++++++++

			 */

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	printf("n_single_resolution=%d   n_multi_resolution=%d   \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
			};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);
	double heat_CFL = 0.4*dx*dx/(thermal_diffusivity);
	double dt_mech = fmin(dt_empirical, 0.50*mech_CFL);
	double dt_heat = fmin(dt_empirical, 0.50*heat_CFL);
	double dt = fmin(dt_mech,dt_heat);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// Keep Model 3's seeded refined block on the same lattice as the high-resolution particles.
	// The old literal limit (0.000117 m) landed between columns for the default 61-layer setup and
	// left a visible clearance strip at the refinement front. Snap to an integer dxh column instead.
	double initial_refined_x_max = dxh * std::ceil((0.000117 - 1e-12) / dxh);

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.9*dxh) || pxh>initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +---------------+
			   + HR |          +
			   +-----    LR    +
			   +               +
			   +---------------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=(py_split-1.9*dxh) && pxl<=initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	// slight modification for reserved CHILD particles!
	for (unsigned int i = n; i < nxh*nyh; i++) {
		particles[part_iter] = particle(part_iter);
	}

	printf("n_single_resolution=%d   n_current=%d  \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// adaptivity settings

	// default settings +-+-++-+-+-+-+-+-+-+-+-+-+-
	double alpha_dx = 0.50;
	double beta_h = 0.50;
	double v_cr = 0.40;
	double div_v_cr = 2e+5;
	double SvM_cr = 1e+7;
	double eps_cr = 110;
	double T_cr = 700.;
	glm::dvec2 xy_min = {0.25, 0.25};
	glm::dvec2 xy_max = {0.75, 0.75};
	double frame_width =  0.000350;
	double frame_height = 0.000060;
	unsigned int n_nbh = 10;
	double l_eff = lc + 0.1*lx;
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	adaptivity *adapt = new adaptivity(alpha_dx, beta_h, v_cr, div_v_cr, SvM_cr, eps_cr,
									   T_cr, xy_min, xy_max, frame_width, frame_height,
									   n_nbh, l_eff, true);

	adapt->set_refine_criterion(adaptivity::refine_criteria::moving_frame);
	adapt->set_refine_pattern(adaptivity::pattern::cubic_basic);

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);
	b->set_adaptivity(adapt);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< dynamic refinement model >>>\n");

	return b;
}

```


**Tool Call: Search files for regex `body \*cutting_ref_single_resol|body \*cutting_ref_multi_resol_apriori|body \*cutting_ref_multi_resol_dynamic` (case-sensitive)**
Status: Completed

Found 3 matches:

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp

### body *cutting_ref_single_resol( ) › L712-716
```
}

 body *cutting_ref_single_resol(unsigned int nbox) {
	/*
	 * ===========================================================
```

### body *cutting_ref_multi_resol_apriori( ) › L941-945
```
}

 body *cutting_ref_multi_resol_apriori(unsigned int nbox) {
	/*
	 * ===========================================================
```

### body *cutting_ref_multi_resol_dynamic( ) › L1235-1239
```
}

 body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
```


## Assistant

Can use terminal Python script? Allowed. Need no shell substitutions. Here-doc uses << not variable substitution? It is shell redirection, not prohibited? tool says do not generate shell substitutions/interpolations such as <(...) or >(...). heredoc? Safer use python -c with escaped script hard.

Maybe use edit with more unique by including function line and following? big old. Hard.

Use perl? Could use python file modify. Command no `$` or backticks. Must no shell substitutions. Python -c string. Use path literal. Let's do replace globally for exact snippet.

**Tool Call: python -c "from pathlib import Path; p=Path('src/benchmarks/test_cuttings.cpp'); s=p.read_text(); old='\tdouble dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;\n\tdouble mech_CFL = 0.5*hdx*dx/(pc.c0() + vc);\n\tdouble heat_CFL = 0.4*dx*dx/(thermal_diffusivity);\n\tdouble dt_mech = fmin(dt_empirical, 0.50*mech_CFL);\n\tdouble dt_heat = fmin(dt_empirical, 0.50*heat_CFL);\n\tdouble dt = fmin(dt_mech,dt_heat);'; new='\tdouble dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;\n\tdouble dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);'; c=s.count(old); print(c); s=s.replace(old,new); p.write_text(s)"**
Status: Completed

Terminal:
```
3





```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 560-700)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
			if (e.physical_tag == rear_tag) rear_found = true;
		}
		if (top_found) ft->set_mechanics_fixed_y_on_physical(top_tag);
		else std::fprintf(stderr, "warning: FE BC validation top_tag=%d not found in FE tool boundary edges\n", top_tag);
		if (rear_found) ft->set_mechanics_fixed_y_on_physical(rear_tag);
		else std::fprintf(stderr, "warning: FE BC validation rear_tag=%d not found in FE tool boundary edges\n", rear_tag);

		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		if (anchor_ux && rear_found) {
			std::unordered_set<unsigned int> rear_nodes;
			for (const auto &e : ft->boundary_edges()) {
				if (e.physical_tag != rear_tag) continue;
				rear_nodes.insert(e.n0);
				rear_nodes.insert(e.n1);
			}
			if (rear_nodes.empty()) {
				std::fprintf(stderr, "warning: FE BC validation rear_tag=%d has no nodes; skipping UX anchor\n", rear_tag);
			} else {
			unsigned int anchor = 0;
			double best_x = -std::numeric_limits<double>::infinity();
			for (unsigned int n : rear_nodes) {
				glm::dvec2 pw = ft->node_world(n);
				if (!std::isfinite(pw.x)) continue;
				if (pw.x > best_x) {
					best_x = pw.x;
					anchor = n;
				}
			}
			ft->set_mechanics_fixed_x_nodes({anchor});
			}
		}

		if (top_found) ft->set_dirichlet_on_physical(top_tag, Tamb_K);
		if (rear_found) ft->set_dirichlet_on_physical(rear_tag, Tamb_K);
	}

	return ft;
}

 body *cutting_ref_mr(unsigned int ny) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_cm_musec_g();

	double speed = 83.333328*1e-5;
	double mu_fric = 0.35;

	double hi_x = 0.100; double hi_y =  0.060;
	double lo_x = 0.000; double lo_y =  0.030;

	double dy = (hi_y-lo_y)/(ny-1);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx;
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx*ny);

	particle *particles = new particle[nx*ny];

	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	double rho0 = physical_constants.rho0();
	double T0 = physical_constants.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

		// fix bottom
		particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants correction_constants(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(physical_constants, correction_constants);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(physical_constants)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.) feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;

```


## Assistant

Could refine MR after attach? But units cm? maybe skip to avoid SI mix.

Need after attach refine dt for SI functions. In single attach, after corrections can recompute and set_dt. Need do for all three. Use time variable accessible. Yes defined before particles. Add after ft adjustments block before logger.

Use replace with exact block after ft block ending enforce. Need multiple instances maybe same. We'll insert after block for all. But variables in all functions: pc, dx, hdx, vc, dt_empirical, time, ft. OK.



**Tool Call: python -c "from pathlib import Path; p=Path('src/benchmarks/test_cuttings.cpp'); s=p.read_text(); old='\t\tft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));\n\t\tenforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);\n\t}\n\n\tglobal_logger = new logger(\"cutting\");'; new='\t\tft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));\n\t\tenforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);\n\t}\n\n\tdt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);\n\ttime->set_dt(dt);\n\n\tglobal_logger = new logger(\"cutting\");'; c=s.count(old); print(c); s=s.replace(old,new); p.write_text(s)"**
Status: Completed

Terminal:
```
1





```


## Assistant

Only one (single). Need inspect apriori block variations.

**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 1140-1205)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
			};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {

```


**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 1205-1235)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955

```


## Assistant

Indent weird. Need insert.

**Tool Call: python -c "from pathlib import Path; p=Path('src/benchmarks/test_cuttings.cpp'); s=p.read_text(); old='\tglobal_logger = new logger(\"cutting\");\n\tglobal_logger->set_fe_tool(ft);\n\tglobal_logger->set_log_vtk(true);'; new='\tdt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);\n\ttime->set_dt(dt);\n\n\tglobal_logger = new logger(\"cutting\");\n\tglobal_logger->set_fe_tool(ft);\n\tglobal_logger->set_log_vtk(true);'; c=s.count(old); print(c); s=s.replace(old,new); p.write_text(s)"**
Status: Completed

Terminal:
```
4





```


## Assistant

<thinking>
**Inspecting for duplicates**

Oh no, I think I accidentally inserted duplicates everywhere! It seems that even the single entries still have a global logger hanging around. I need to check around for those single instances to make sure I'm not matching everything and creating unnecessary duplicates. It's probably a matter of cleaning up after the previous replacement I did. Let's inspect that area closely!
</thinking>

**Tool Call: Search files for regex `dt = estimate_dt_for_cutting` (case-sensitive)**
Status: Completed

Found 7 matches:

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp

### body *cutting_ref_mr( ) › L705-709
```
	b->set_fe_tool(ft);

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```

### body *cutting_ref_single_resol( ) › double dt › L780-784
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
```

### body *cutting_ref_single_resol( ) › L922-929
```
	}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```

### body *cutting_ref_multi_resol_apriori( ) › double dt › L1027-1031
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
```

### body *cutting_ref_multi_resol_apriori( ) › L1218-1222
```
		}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```

### body *cutting_ref_multi_resol_dynamic( ) › double dt › L1320-1324
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
```

### body *cutting_ref_multi_resol_dynamic( ) › L1539-1543
```
		}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```


## Assistant

<thinking>
**Inspecting variables**

I noticed that while we inserted something into MR, the variables pc, vc, and dt_empirical aren't actually in MR. So it looks like I need to remove them from MR. Plus, there’s a duplication in the single. I think I should take a closer look around MR to figure out what’s going on. It’s important to ensure everything is correct and tidy, so nothing gets confused. Let's keep going to sort this out!
</thinking>

**Tool Call: Read file `src\benchmarks\test_cuttings.cpp` (lines 690-715)**
Status: Completed

```mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.) feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	return b;
}

```


**Tool Call: src\\benchmarks\\test\_cuttings.cpp**
Status: Completed

Diff: src\benchmarks\test_cuttings.cpp
```
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

#include "test_cuttings.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <cerrno>

static bool try_read_env_double(const char *key, double &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	char *end = nullptr;
	double v = strtod(s, &end);
	if (end == s || !std::isfinite(v)) return false;
	out = v;
	return true;
}

static bool try_read_env_int(const char *key, int &out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;
	while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;
	if (!*s) return false;
	char *end = nullptr;
	long v = std::strtol(s, &end, 10);
	if (end == s) return false;
	while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
	if (*end != '\0') return false;
	if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
	out = static_cast<int>(v);
	return true;
}

static bool try_read_env_table(const char *key, std::vector<double> &T_out, std::vector<double> &v_out) {
	const char *s = getenv(key);
	if (!s || s[0] == '\0') return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r') ++p;
		if (!*p) break;

		char *end = nullptr;
		double T = strtod(p, &end);
		if (end == p || !std::isfinite(T)) return false;
		p = end;

		while (*p == ' ' || *p == '\t') ++p;
		if (*p != ':' && *p != '=') return false;
		++p;
		while (*p == ' ' || *p == '\t') ++p;

		end = nullptr;
		double v = strtod(p, &end);
		if (end == p || !std::isfinite(v)) return false;
		p = end;

		pairs.push_back({T, v});
	}

	if (pairs.size() < 2) return false;
	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

	T_out.clear();
	v_out.clear();
	for (const auto &kv : pairs) {
		if (!T_out.empty() && kv.first == T_out.back()) {
			v_out.back() = kv.second;
			continue;
		}
		T_out.push_back(kv.first);
		v_out.push_back(kv.second);
	}
	return T_out.size() >= 2;
}

static void adjust_workpiece_y_bounds_for_feed(double base_lo_y, double base_hi_y, unsigned int base_ny,
											   double target_feed, unsigned int safety_layers,
											   double &lo_y, double &hi_y, unsigned int &ny, double &dy) {
	hi_y = base_hi_y;
	double base_thickness = base_hi_y - base_lo_y;
	dy = base_thickness / (base_ny - 1);
	double required_thickness = target_feed + safety_layers * dy;

	unsigned int extra_layers = 0;
	if (required_thickness > base_thickness) {
		double raw = (required_thickness - base_thickness) / dy;
		extra_layers = (unsigned int)std::ceil(raw - 1e-12);
	}
	ny = base_ny + extra_layers;
	lo_y = hi_y - (ny - 1) * dy;
}

static double read_coupled_motion_ratio() {
	const char *enable_env = getenv("MFREE_COUPLED_MOTION");
	bool enabled = (enable_env && atoi(enable_env) != 0);
	if (!enabled) return 1.0;

	double ratio = 1.0;
	if (try_read_env_double("MFREE_COUPLED_MOTION_RATIO", ratio)) {
		if (!std::isfinite(ratio)) ratio = 1.0;
		ratio = std::max(0.0, std::min(1.0, ratio));
		return ratio;
	}

	const char *primary = getenv("MFREE_PRIMARY_MOVING_BODY");
	if (!primary || primary[0] == '\0') return 1.0;
	std::string s(primary);
	for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s == "workpiece" || s == "wp") return 0.0;
	if (s == "both" || s == "coupled") return 0.5;
	return 1.0;
}

static double estimate_dt_for_cutting(const physical_constants &pc, double dx, double hdx, double relative_speed, double empirical_cap, const fe_tool *ft) {
	coupled_timestep_config cfg;
	cfg.particle_spacing = dx;
	cfg.smoothing_length_ratio = hdx;
	cfg.max_relative_speed = relative_speed;
	cfg.empirical_dt_cap = empirical_cap;
	cfg.interface_contact_area = dx * dx;
	try_read_env_double("MFREE_TIMESTEP_WP_MECH_SAFETY", cfg.workpiece_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_WP_THERM_SAFETY", cfg.workpiece_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_MECH_SAFETY", cfg.tool_mechanical_safety);
	try_read_env_double("MFREE_TIMESTEP_TOOL_THERM_SAFETY", cfg.tool_thermal_safety);
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_SAFETY", cfg.interface_thermal_safety);
	try_read_env_double("MFREE_THERMAL_H_FULL", cfg.contact_conductance_full);
	double area_factor = 1.0;
	try_read_env_double("MFREE_TIMESTEP_INTERFACE_AREA_FACTOR", area_factor);
	if (std::isfinite(area_factor) && area_factor > 0.) cfg.interface_contact_area *= area_factor;

	coupled_timestep_limits limits = estimate_coupled_timestep(pc, cfg, ft);
	int print_limits = 1;
	try_read_env_int("MFREE_TIMESTEP_PRINT", print_limits);
	if (print_limits != 0) print_coupled_timestep_limits(limits);
	return (std::isfinite(limits.maximum_dt) && limits.maximum_dt > 0.) ? limits.maximum_dt : empirical_cap;
}

static void apply_mech_fix_tags_from_env(fe_tool &ft) {
	auto apply_tag_list = [&](const char *env_key, auto apply_tag) -> bool {
		const char *tags = getenv(env_key);
		if (!tags || tags[0] == '\0') return false;
		std::string s(tags);
		std::size_t i = 0;
		bool any = false;
		while (i < s.size()) {
			while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
			if (i >= s.size()) break;
			std::size_t j = i;
			while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
			std::string tok = s.substr(i, j - i);
			errno = 0;
			char *end = nullptr;
			long v = std::strtol(tok.c_str(), &end, 10);
			if (end != tok.c_str() && end != nullptr && *end == '\0' && errno == 0) {
				if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max() && v != 0) {
					apply_tag(static_cast<int>(v));
				any = true;
				}
			}
			i = j;
		}
		return any;
	};

	bool any = false;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_Y_TAGS", [&](int tag) { ft.set_mechanics_fixed_y_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_X_TAGS", [&](int tag) { ft.set_mechanics_fixed_x_on_physical(tag); }) || any;
	any = apply_tag_list("MFREE_FE_TOOL_FIX_TAGS", [&](int tag) { ft.set_mechanics_fixed_on_physical(tag); }) || any;
	if (any) {
		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		int anchor_tag = 0;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_TOOL_ANCHOR_TAG", v)) anchor_tag = v;
		}
		if (anchor_ux && anchor_tag != 0) {
			std::unordered_set<unsigned int> nodes;
			for (const auto &e : ft.boundary_edges()) {
				if (e.physical_tag != anchor_tag) continue;
				nodes.insert(e.n0);
				nodes.insert(e.n1);
			}
			if (!nodes.empty()) {
				unsigned int anchor = *nodes.begin();
				double best_x = -std::numeric_limits<double>::infinity();
				for (unsigned int n : nodes) {
					glm::dvec2 pw = ft.node_world(n);
					if (!std::isfinite(pw.x)) continue;
					if (pw.x > best_x) {
						best_x = pw.x;
						anchor = n;
					}
				}
				ft.set_mechanics_fixed_x_nodes({anchor});
			}
		}
		return;
	}

	const char *tags = getenv("MFREE_FE_TOOL_FIX_TAGS");
	if (!tags || tags[0] == '\0') {
		std::unordered_set<unsigned int> bnodes;
		for (const auto &e : ft.boundary_edges()) {
			bnodes.insert(e.n0);
			bnodes.insert(e.n1);
		}
		if (bnodes.empty()) return;

		double x_max = -std::numeric_limits<double>::infinity();
		double x_min = std::numeric_limits<double>::infinity();
		for (unsigned int i : bnodes) x_max = std::max(x_max, ft.nodes_tool_frame()[i].x);
		for (unsigned int i : bnodes) x_min = std::min(x_min, ft.nodes_tool_frame()[i].x);

		std::vector<unsigned int> fixed;
		double width = x_max - x_min;
		double tol = 0.01 * width;
		try_read_env_double("MFREE_FE_TOOL_FIX_X_TOL", tol);
		if (!std::isfinite(tol) || tol <= 0.) tol = 0.01 * width;

		for (int attempt = 0; attempt < 4; attempt++) {
			fixed.clear();
			for (unsigned int i : bnodes) {
				if (ft.nodes_tool_frame()[i].x >= x_max - tol) fixed.push_back(i);
			}
			if (fixed.size() >= 2) break;
			tol *= 5.0;
		}
		ft.set_mechanics_fixed_nodes(fixed);
		return;
	}
	std::string s(tags);
	std::size_t i = 0;
	while (i < s.size()) {
		while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == ';' || s[i] == ',')) i++;
		if (i >= s.size()) break;
		std::size_t j = i;
		while (j < s.size() && s[j] != ';' && s[j] != ',' && s[j] != ' ' && s[j] != '\t') j++;
		int tag = std::atoi(s.substr(i, j - i).c_str());
		if (tag != 0) ft.set_mechanics_fixed_on_physical(tag);
		i = j;
	}
}

static std::vector<glm::dvec2> extract_boundary_loop_world(const fe_tool &ft) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &edges = ft.boundary_edges();

	std::unordered_map<unsigned int, std::vector<unsigned int>> adj;
	adj.reserve(edges.size());

	for (const auto &e : edges) {
		adj[e.n0].push_back(e.n1);
		adj[e.n1].push_back(e.n0);
	}

	unsigned int start = 0;
	bool has_start = false;
	glm::dvec2 start_p(0.);

	for (const auto &kv : adj) {
		unsigned int idx = kv.first;
		if (idx >= nodes.size()) continue;
		glm::dvec2 p = ft.to_world_frame(nodes[idx]);
		if (!has_start || p.x < start_p.x || (p.x == start_p.x && p.y < start_p.y)) {
			has_start = true;
			start = idx;
			start_p = p;
		}
	}

	if (!has_start) return {};

	std::vector<unsigned int> loop;
	loop.reserve(adj.size());

	unsigned int prev = std::numeric_limits<unsigned int>::max();
	unsigned int cur = start;
	for (unsigned int it = 0; it < static_cast<unsigned int>(adj.size()) + 2; it++) {
		loop.push_back(cur);
		const auto &nb = adj[cur];
		if (nb.empty()) break;
		unsigned int next = nb[0];
		if (nb.size() >= 2 && next == prev) next = nb[1];
		if (next == start) break;
		prev = cur;
		cur = next;
	}

	std::vector<glm::dvec2> pts;
	pts.reserve(loop.size());
	for (unsigned int idx : loop) {
		if (idx >= nodes.size()) continue;
		pts.push_back(ft.to_world_frame(nodes[idx]));
	}
	return pts;
}

static glm::dvec2 compute_nominal_tool_center(glm::dvec2 tl, double length, double height, double rake_angle, double clearance_angle) {
	glm::dvec2 tr(tl.x + length, tl.y);
	glm::dvec2 bl(tl.x, tl.y - height);

	double alpha_rake = rake_angle * M_PI / 180.;
	double alpha_free = (180 - 90 - clearance_angle) * M_PI / 180.;

	glm::dmat2x2 rot_rake(cos(alpha_rake), -sin(alpha_rake), sin(alpha_rake), cos(alpha_rake));
	glm::dmat2x2 rot_free(cos(alpha_free), -sin(alpha_free), sin(alpha_free), cos(alpha_free));

	glm::dvec2 down(0., -1.);
	glm::dvec2 trc = tr + down * rot_rake;
	glm::dvec2 blc = bl + down * rot_free;

	double x1 = tr.x, y1 = tr.y;
	double x2 = trc.x, y2 = trc.y;
	double x3 = bl.x, y3 = bl.y;
	double x4 = blc.x, y4 = blc.y;
	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	glm::dvec2 br = tr;
	if (std::isfinite(denom) && std::abs(denom) > 1e-30) {
		double d12 = x1 * y2 - y1 * x2;
		double d34 = x3 * y4 - y3 * x4;
		double px = (d12 * (x3 - x4) - (x1 - x2) * d34) / denom;
		double py = (d12 * (y3 - y4) - (y1 - y2) * d34) / denom;
		if (std::isfinite(px) && std::isfinite(py)) br = glm::dvec2(px, py);
	}

	return 0.25 * (tl + tr + br + bl);
}

static glm::dvec2 closest_point_on_segment(glm::dvec2 p, glm::dvec2 a, glm::dvec2 b) {
	glm::dvec2 ab = b - a;
	double ab2 = ab.x * ab.x + ab.y * ab.y;
	if (!(ab2 > 0.0) || !std::isfinite(ab2)) return a;
	double t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / ab2;
	if (!std::isfinite(t)) t = 0.0;
	t = std::max(0.0, std::min(1.0, t));
	return a + t * ab;
}

static glm::dvec2 closest_point_on_polyline(glm::dvec2 p, const std::vector<glm::dvec2> &poly) {
	glm::dvec2 best(0.);
	double best_d2 = std::numeric_limits<double>::infinity();
	if (poly.size() < 2) return best;
	for (std::size_t i = 0; i < poly.size(); i++) {
		glm::dvec2 a = poly[i];
		glm::dvec2 b = poly[(i + 1) % poly.size()];
		glm::dvec2 cp = closest_point_on_segment(p, a, b);
		glm::dvec2 d = p - cp;
		double d2 = d.x * d.x + d.y * d.y;
		if (std::isfinite(d2) && d2 < best_d2) {
			best_d2 = d2;
			best = cp;
		}
	}
	return best;
}

static double poly_min_y(const std::vector<glm::dvec2> &poly) {
	double low = std::numeric_limits<double>::infinity();
	for (const auto &p : poly) low = std::min(low, p.y);
	if (!std::isfinite(low)) return 0.0;
	return low;
}

static void enforce_fe_tool_corner_clearance(fe_tool &ft, glm::dvec2 wp_corner, double clearance_target_m, unsigned int iters) {
	for (unsigned int it = 0; it < iters; it++) {
		std::vector<glm::dvec2> poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;

		double y_bottom = poly_min_y(poly);
		double dy = (wp_corner.y - clearance_target_m) - y_bottom;

		glm::dvec2 pos = ft.get_pos();
		pos.y += dy;
		ft.set_pose(pos, ft.get_vel());

		poly = ft.boundary_loop_world();
		if (poly.size() < 3) return;
		glm::dvec2 cp = closest_point_on_polyline(wp_corner, poly);
		double dx = wp_corner.x - cp.x;
		pos = ft.get_pos();
		pos.x += dx;
		ft.set_pose(pos, ft.get_vel());
	}
}

static fe_tool *attach_fe_tool_from_env(double T0, glm::dvec2 desired_center, glm::dvec2 desired_vel, double desired_edge_y,
                                     glm::dvec2 wp_corner, double clearance_target_m) {
	const char *msh_env = getenv("MFREE_FE_TOOL_MSH");
	std::string msh;
	if (msh_env && msh_env[0] != '\0') {
		msh = msh_env;
	} else {
		const std::string def = "./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh";
		if (std::filesystem::exists(def)) msh = def;
		else {
			std::fprintf(stderr, "Missing MFREE_FE_TOOL_MSH\n");
			exit(1);
		}
	}

	fe_tool *ft = new fe_tool();
	if (!ft->load_gmsh_msh2(msh)) {
		delete ft;
		std::fprintf(stderr, "Failed to load MFREE_FE_TOOL_MSH\n");
		exit(1);
	}

	fe_tool::thermal_material mat;
	mat.rho = 14500.0;
	mat.cp = 200.0;
	mat.k = 80.0;
	try_read_env_double("MFREE_FE_TOOL_RHO", mat.rho);
	try_read_env_double("MFREE_FE_TOOL_CP", mat.cp);
	try_read_env_double("MFREE_FE_TOOL_K", mat.k);
	ft->set_material(mat);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_RHO_TABLE", T, v)) ft->set_material_table_rho(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_CP_TABLE", T, v)) ft->set_material_table_cp(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_K_TABLE", T, v)) ft->set_material_table_k(std::move(T), std::move(v));
	}

	fe_tool::mechanical_material mech;
	mech.E = 600e9;
	mech.nu = 0.22;
	mech.alpha = 4.5e-6;
	try_read_env_double("MFREE_FE_TOOL_E", mech.E);
	try_read_env_double("MFREE_FE_TOOL_NU", mech.nu);
	try_read_env_double("MFREE_FE_TOOL_ALPHA", mech.alpha);
	ft->set_mechanical_material(mech);

	{
		std::vector<double> T;
		std::vector<double> v;
		if (try_read_env_table("MFREE_FE_TOOL_E_TABLE", T, v)) ft->set_mechanical_table_E(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_NU_TABLE", T, v)) ft->set_mechanical_table_nu(std::move(T), std::move(v));
		if (try_read_env_table("MFREE_FE_TOOL_ALPHA_TABLE", T, v)) ft->set_mechanical_table_alpha(std::move(T), std::move(v));
	}
	ft->set_reference_temperature(T0);
	bool bc_validate = false;
	{
		int v = 0;
		if (try_read_env_int("MFREE_FE_BC_VALIDATE", v) && v != 0) bc_validate = true;
	}
	if (!bc_validate) {
		apply_mech_fix_tags_from_env(*ft);
	}

	ft->set_initial_temperature(T0);
	glm::dvec2 pos(0.);
	const char *align_env = getenv("MFREE_FE_TOOL_ALIGN_CENTER");
	bool align = true;
	if (align_env) align = (atoi(align_env) != 0);
	if (align) {
		glm::dvec2 mesh_center(0.);
		const auto &nodes = ft->nodes_tool_frame();
		for (const auto &p : nodes) mesh_center += p;
		if (!nodes.empty()) mesh_center /= static_cast<double>(nodes.size());
		pos = desired_center - mesh_center;
	} else {
		try_read_env_double("MFREE_FE_TOOL_POS_X", pos.x);
		try_read_env_double("MFREE_FE_TOOL_POS_Y", pos.y);
	}
	ft->set_pose(pos, desired_vel);

	fe_tool::convection_bc air;
	air.h = 20.0;
	air.T_inf = 298.15;

	fe_tool::convection_bc water;
	water.h = 5000.0;
	water.T_inf = 293.15;

	double y_thresh = desired_edge_y;
	const char *y_env = getenv("MFREE_COOLANT_Y_THRESHOLD");
	if (y_env) y_thresh = atof(y_env);
	ft->set_convection_flooded_by_y(air, water, y_thresh);

	enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target_m, 5);

	if (bc_validate) {
		int top_tag = 110;
		int rear_tag = 114;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_TOP_TAG", v)) top_tag = v;
			if (try_read_env_int("MFREE_FE_BC_REAR_TAG", v)) rear_tag = v;
		}
		double Tamb_C = 25.0;
		try_read_env_double("MFREE_FE_BC_AMBIENT_C", Tamb_C);
		double Tamb_K = Tamb_C + 273.15;

		ft->clear_mechanics_fixed();
		ft->clear_mechanics_fixed_nodes();
		bool top_found = false;
		bool rear_found = false;
		for (const auto &e : ft->boundary_edges()) {
			if (e.physical_tag == top_tag) top_found = true;
			if (e.physical_tag == rear_tag) rear_found = true;
		}
		if (top_found) ft->set_mechanics_fixed_y_on_physical(top_tag);
		else std::fprintf(stderr, "warning: FE BC validation top_tag=%d not found in FE tool boundary edges\n", top_tag);
		if (rear_found) ft->set_mechanics_fixed_y_on_physical(rear_tag);
		else std::fprintf(stderr, "warning: FE BC validation rear_tag=%d not found in FE tool boundary edges\n", rear_tag);

		bool anchor_ux = true;
		{
			int v = 0;
			if (try_read_env_int("MFREE_FE_BC_ANCHOR_UX", v)) anchor_ux = (v != 0);
		}
		if (anchor_ux && rear_found) {
			std::unordered_set<unsigned int> rear_nodes;
			for (const auto &e : ft->boundary_edges()) {
				if (e.physical_tag != rear_tag) continue;
				rear_nodes.insert(e.n0);
				rear_nodes.insert(e.n1);
			}
			if (rear_nodes.empty()) {
				std::fprintf(stderr, "warning: FE BC validation rear_tag=%d has no nodes; skipping UX anchor\n", rear_tag);
			} else {
			unsigned int anchor = 0;
			double best_x = -std::numeric_limits<double>::infinity();
			for (unsigned int n : rear_nodes) {
				glm::dvec2 pw = ft->node_world(n);
				if (!std::isfinite(pw.x)) continue;
				if (pw.x > best_x) {
					best_x = pw.x;
					anchor = n;
				}
			}
			ft->set_mechanics_fixed_x_nodes({anchor});
			}
		}

		if (top_found) ft->set_dirichlet_on_physical(top_tag, Tamb_K);
		if (rear_found) ft->set_dirichlet_on_physical(rear_tag, Tamb_K);
	}

	return ft;
}

 body *cutting_ref_mr(unsigned int ny) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_cm_musec_g();

	double speed = 83.333328*1e-5;
	double mu_fric = 0.35;

	double hi_x = 0.100; double hi_y =  0.060;
	double lo_x = 0.000; double lo_y =  0.030;

	double dy = (hi_y-lo_y)/(ny-1);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx;
	double hdx = 1.5;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(speed + c0);
	double t_final =  0.1/speed*0.5; // 1mm of cut

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	printf("using timestep %e with %d particles\n", dt, nx*ny);

	particle *particles = new particle[nx*ny];

	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	double rho0 = physical_constants.rho0();
	double T0 = physical_constants.jc().Tref();
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;

		// fix bottom
		particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants correction_constants(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(physical_constants, correction_constants);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(physical_constants)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);
	b->set_plasticity(plast);

	glm::dvec2 desired_vel = glm::dvec2(speed, 0.);
	glm::dvec2 desired_center = glm::dvec2(-0.025, 0.075);
	double desired_edge_y = 0.0486;

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm <= 0.) feed_per_rev_mm = 0.2;
	double clearance_target = feed_per_rev_mm * 1e-3;
	glm::dvec2 wp_corner(0.0, 0.060);

	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_fric);
	b->set_fe_tool(ft);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	return b;
}

 body *cutting_ref_single_resol(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 1 & 4 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;

	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double dx = dy;
	unsigned int nx = (hi_x-lo_x)/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nx*ny];

	srand(0);
	unsigned int part_iter = 0;
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < ny; j++) {
			double px = i*dx; double py = j*dx;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = px + lo_x;
			particles[part_iter].y = py + lo_y;

			particles[part_iter].X = px + lo_x;
			particles[part_iter].Y = py + lo_y;

			part_iter++;
		}
	}

	unsigned int n = nx*ny;
	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].h = hdx*dx;
		particles[i].m = dx*dx*rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dy) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dx);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}

	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

		double current_feed = hi_y - low_y();
		double dist_to_target_feed = fabs(current_feed - target_feed);
		double correction_time = dist_to_target_feed / vc;
		double sign = (current_feed > target_feed) ? 1 : -1.;

		ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
		ft->update_pose(correction_time * sign);

		if (std::isfinite(y_offset) && y_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
			ft->update_pose(y_offset);
		}

		if (std::isfinite(x_offset) && x_offset != 0.) {
			ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
			ft->update_pose(x_offset);
		}

		ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
		enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
	}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, nx*ny);
	printf("<<< single-resolution simulation >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_apriori(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.1*dxh)) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +++++++++++++++++
			   +               +
			   +-------CL------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=py_split) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			/*
			 * low res
			 *

			   +-------CL------+
			   +               +
			   +++++++++++++++++

			 */

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	printf("n_single_resolution=%d   n_multi_resolution=%d   \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
			};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< a-priori refinement model >>>\n");

	return b;
}

 body *cutting_ref_multi_resol_dynamic(unsigned int nbox) {
	/*
	 * ===========================================================
	 * according to (6.3) simulation by Sima & Ozel 2010 -> p. 955
	 * ===========================================================
	 */

	// MODEL 3 from the paper
	// ----------------------------------------------------------
	// choose your desired material model as the following:
	// ----------------------------------------------------------
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	// ----------------------------------------------------------

	bool thermal_conduction = true;
	double hdx = 1.5;
	double rho0 = pc.rho0();
	double T0 = 300.0;
	double wp_T0 = T0;
	double tool_T0 = T0;
	try_read_env_double("MFREE_WP_T0", wp_T0);
	try_read_env_double("MFREE_TOOL_T0", tool_T0);
	if (!std::isfinite(wp_T0)) wp_T0 = T0;
	if (!std::isfinite(tool_T0)) tool_T0 = T0;
	double thermal_diffusivity = pc.tc().k()/(rho0*pc.tc().cp());

	double feed_per_rev_mm = 0.2;
	try_read_env_double("MFREE_FEED_PER_REV_MM", feed_per_rev_mm);
	if (!std::isfinite(feed_per_rev_mm) || feed_per_rev_mm < 0.) feed_per_rev_mm = 0.2;
	double depth_adjust = feed_per_rev_mm * 1e-3;
	double base_target_feed_mm = 0.1;
	try_read_env_double("MFREE_BASE_TARGET_FEED_MM", base_target_feed_mm);
	if (!std::isfinite(base_target_feed_mm) || base_target_feed_mm < 0.) base_target_feed_mm = 0.1;

	// workpiece dimensions SI
	double thickness_mm = 0.5;
	try_read_env_double("MFREE_WORKPIECE_THICKNESS_MM", thickness_mm);
	if (!std::isfinite(thickness_mm) || thickness_mm <= 0.) thickness_mm = 0.5;
	double thickness = thickness_mm * 1e-3;

	double lo_x = 0.00000;
	double hi_x = 0.00200;
	double base_hi_y = 0.00060;
	double base_lo_y = base_hi_y - thickness;
	double lx = hi_x - lo_x;
	double target_feed = base_target_feed_mm * 1e-3 + depth_adjust;
	unsigned int ny = nbox;
	double lo_y = 0., hi_y = 0., dy = 0.;
	adjust_workpiece_y_bounds_for_feed(base_lo_y, base_hi_y, nbox, target_feed, 5, lo_y, hi_y, ny, dy);
	double ly = hi_y - lo_y;

	double dx = dy;
	unsigned int nx = lx/dx + 1;
	double v_m_min = 500.;
	try_read_env_double("MFREE_CUTTING_SPEED_M_MIN", v_m_min);
	if (!std::isfinite(v_m_min) || v_m_min <= 0.) v_m_min = 500.;
	double vc = v_m_min / 60.;		// m/min -> m/s
	double nudge = -dx;
	double ratio = read_coupled_motion_ratio();
	double tool_vx = ratio * vc;
	double wp_vx = -(1.0 - ratio) * vc;
	bool apply_fixtures = !(std::abs(wp_vx) > 0.);

	// multi-resolution setup
	double resol_ratio = 2.0;
	double py_split = 0.5*ly + lo_y;
	double dxh = dx;
	double dxl = dxh * resol_ratio;
	unsigned int nxh = nx;
	unsigned int nyh = ny;
	unsigned int nxl = lx/dxl + 1;
	unsigned int nyl = ly/dxl + 1;
	double dVl = dxl *dxl;
	double dVh = dxh *dxh;
	double h0l = hdx * dxl;
	double h0h = hdx * dxh;

	// time settings
	double lc = 1e-3; // 1mm of cut
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
	time->set_t_final(t_final);
	time->set_dt(dt);

	particle *particles  = new particle[nxh*nyh];

	srand(0);
	unsigned int part_iter = 0;

	// Keep Model 3's seeded refined block on the same lattice as the high-resolution particles.
	// The old literal limit (0.000117 m) landed between columns for the default 61-layer setup and
	// left a visible clearance strip at the refinement front. Snap to an integer dxh column instead.
	double initial_refined_x_max = dxh * std::ceil((0.000117 - 1e-12) / dxh);

	// 1. create the high resolution region
	for (unsigned int i = 0; i < nxh; i++) {
		for (unsigned int j = 0; j < nyh; j++) {
			double pxh = i*dxh;
			double pyh = j*dxh;

			if ((pyh+lo_y)<(py_split-1.9*dxh) || pxh>initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxh + lo_x;
			particles[part_iter].y = pyh + lo_y;
			particles[part_iter].X = pxh + lo_x;
			particles[part_iter].Y = pyh + lo_y;

			particles[part_iter].refine_step = 1;

			/*
			 * high res
			 *
			   +---------------+
			   + HR |          +
			   +-----    LR    +
			   +               +
			   +---------------+

			 */

			part_iter++;
		}
	}

	// 2. create the low resolution region
	for (unsigned int i = 0; i < nxl; i++) {
		for (unsigned int j = 0; j < nyl; j++) {
			double pxl = i*dxl;
			double pyl = j*dxl;

			if ((pyl+lo_y)>=(py_split-1.9*dxh) && pxl<=initial_refined_x_max) continue;

			particles[part_iter] = particle(part_iter);
			particles[part_iter].x = pxl + lo_x;
			particles[part_iter].y = pyl + lo_y;
			particles[part_iter].X = pxl + lo_x;
			particles[part_iter].Y = pyl + lo_y;

			particles[part_iter].refine_step = 0;

			part_iter++;
		}
	}

	// total #particles
	unsigned int n = part_iter;

	// slight modification for reserved CHILD particles!
	for (unsigned int i = n; i < nxh*nyh; i++) {
		particles[part_iter] = particle(part_iter);
	}

	printf("n_single_resolution=%d   n_current=%d  \n",nxh*nyh,n);

	for (unsigned int i = 0; i < n; i++) {
		particles[i].rho = rho0;
		particles[i].T = wp_T0;
		particles[i].T_init = wp_T0;
		particles[i].h = (particles[i].refine_step!=0) ? h0h : h0l;
		particles[i].m = (particles[i].refine_step!=0) ? dVh*rho0 : dVl*rho0;
		particles[i].split = false;
		particles[i].merge = false;
		particles[i].vx = wp_vx;
		particles[i].vy = 0.;

		// fixtures
		particles[i].fixed = false;
		if (apply_fixtures) {
			particles[i].fixed = (particles[i].y < lo_y + 0.5*dxl) ? true : false;
			particles[i].fixed = particles[i].fixed || (particles[i].x > hi_x - 0.5*dxh);
		}
	}

	// correction constants
	double alpha = 1.;
	double beta  = 1.;
	double eta   = 0.1;

	double art_stress_eps = 0.3;
	kernel_result w = cubic_spline(0, 0, dx, 0, hdx*dx);
	double wdeltap = w.w;
	double stress_exponent = 4.;

	double xsph_eps = 0.5;

	correction_constants cs(constants_monaghan(wdeltap, stress_exponent, art_stress_eps),
			constants_artificial_viscosity(alpha, beta, eta), xsph_eps);

	// set simulation data
	simulation_data sim_data(pc, cs);

	// plasticity model
	plasticity *plast = new plasticity(new johnson_cook_Sima_2010(pc)); // JC-tanh Sima / Özel 2010
	plast->set_tolerance(1e-6);
	plast->set_dissipation_considered(true);

	// create the body
	body *b = new body(particles, n, sim_data);

	// save thermal setting to body
	thermal *trml = new thermal(pc);
	trml->set_method(thermal::thermal_solver::thermal_pse);  // optional: thermal_brookshaw

	// adaptivity settings

	// default settings +-+-++-+-+-+-+-+-+-+-+-+-+-
	double alpha_dx = 0.50;
	double beta_h = 0.50;
	double v_cr = 0.40;
	double div_v_cr = 2e+5;
	double SvM_cr = 1e+7;
	double eps_cr = 110;
	double T_cr = 700.;
	glm::dvec2 xy_min = {0.25, 0.25};
	glm::dvec2 xy_max = {0.75, 0.75};
	double frame_width =  0.000350;
	double frame_height = 0.000060;
	unsigned int n_nbh = 10;
	double l_eff = lc + 0.1*lx;
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	adaptivity *adapt = new adaptivity(alpha_dx, beta_h, v_cr, div_v_cr, SvM_cr, eps_cr,
									   T_cr, xy_min, xy_max, frame_width, frame_height,
									   n_nbh, l_eff, true);

	adapt->set_refine_criterion(adaptivity::refine_criteria::moving_frame);
	adapt->set_refine_pattern(adaptivity::pattern::cubic_basic);

	// tool settings
	float_t rake = 0.00001;
	float_t clear = 11.;
	glm::dvec2 tl(-0.000410  + nudge, 0.000986074);
	float_t length_tool = -0.000086824 - -0.000500000;
	float_t height_tool =  0.000986074 -  0.000555074;

	double mu_friction = 0.35;
	double fillet_radius = 5e-6;
	double rake_deg = rake;
	double clear_deg = clear;
	const char *swap_env = getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE");
	if (swap_env && atoi(swap_env) != 0) {
		double tmp = rake_deg;
		rake_deg = clear_deg;
		clear_deg = tmp;
	}
	double y_offset = 0.;
	try_read_env_double("MFREE_TOOL_Y_OFFSET", y_offset);

	double x_offset = 0.;
	try_read_env_double("MFREE_TOOL_X_OFFSET", x_offset);

	bool no_rigid_tool = true;

	// save settings to body
	b->set_plasticity(plast);
	if (thermal_conduction) b->set_thermal(trml);
	glm::dvec2 desired_center = compute_nominal_tool_center(tl, length_tool, height_tool, rake_deg, clear_deg);
	glm::dvec2 desired_vel = glm::dvec2(tool_vx, 0.);
	double desired_edge_y = (hi_y - target_feed);
	glm::dvec2 wp_corner(lo_x, hi_y);
	double clearance_target = feed_per_rev_mm * 1e-3;
	fe_tool *ft = attach_fe_tool_from_env(tool_T0, desired_center, desired_vel, desired_edge_y, wp_corner, clearance_target);
	ft->set_mu(mu_friction);
	b->set_fe_tool(ft);
	b->set_adaptivity(adapt);

	if (ft) {
		auto low_y = [&]() -> double {
			std::vector<glm::dvec2> poly = ft->boundary_loop_world();
			double low = std::numeric_limits<double>::infinity();
			for (const auto &p : poly) low = std::min(low, p.y);
			if (!std::isfinite(low)) low = hi_y;
			return low;
		};

			double current_feed = hi_y - low_y();
			double dist_to_target_feed = fabs(current_feed - target_feed);
			double correction_time = dist_to_target_feed / vc;
			double sign = (current_feed > target_feed) ? 1 : -1.;

			ft->set_pose(ft->get_pos(), glm::dvec2(0., vc));
			ft->update_pose(correction_time * sign);

			if (std::isfinite(y_offset) && y_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(0., 1.));
				ft->update_pose(y_offset);
			}
			if (std::isfinite(x_offset) && x_offset != 0.) {
				ft->set_pose(ft->get_pos(), glm::dvec2(1., 0.));
				ft->update_pose(x_offset);
			}

			ft->set_pose(ft->get_pos(), glm::dvec2(tool_vx, 0.));
			enforce_fe_tool_corner_clearance(*ft, wp_corner, clearance_target, 5);
		}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

	global_logger = new logger("cutting");
	global_logger->set_fe_tool(ft);
	global_logger->set_log_vtk(true);

	double feed_report = 0.;
	if (b->get_fe_tool()) {
		std::vector<glm::dvec2> poly = b->get_fe_tool()->boundary_loop_world();
		double low = std::numeric_limits<double>::infinity();
		for (const auto &p : poly) low = std::min(low, p.y);
		if (!std::isfinite(low)) low = hi_y;
		feed_report = hi_y - low;
	}
	printf("feed: %f, dt %e, num_part %d\n", feed_report, dt, n);
	printf("<<< dynamic refinement model >>>\n");

	return b;
}

```


## Assistant

Need check all good 6 matches.

**Tool Call: Search files for regex `dt = estimate_dt_for_cutting` (case-sensitive)**
Status: Completed

Found 6 matches:

## Matches in mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp

### body *cutting_ref_single_resol( ) › double dt › L777-781
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
```

### body *cutting_ref_single_resol( ) › L919-923
```
	}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```

### body *cutting_ref_multi_resol_apriori( ) › double dt › L1021-1025
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
```

### body *cutting_ref_multi_resol_apriori( ) › L1212-1216
```
		}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```

### body *cutting_ref_multi_resol_dynamic( ) › double dt › L1314-1318
```
	double t_final =  lc/vc;
	double dt_empirical = (nbox < 35) ? 1.0e-9 : 5.0e-10;
	double dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, nullptr);

	simulation_time *time = &simulation_time::getInstance();
```

### body *cutting_ref_multi_resol_dynamic( ) › L1533-1537
```
		}

	dt = estimate_dt_for_cutting(pc, dx, hdx, vc, dt_empirical, ft);
	time->set_dt(dt);

```


**Tool Call: src\\benchmarks\\validate\_main.cpp**
Status: Completed

Diff: src\benchmarks\validate_main.cpp
```
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

#include "fe_tool.h"
#include "contact.h"
#include "simulation_time.h"
#include "body.h"
#include "timestep_estimator.h"

#include "benchmarks/material_library.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include "particle.h"
#include "adaptivity.h"

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

// Barycentric interpolation of temperature at point (x,y) in tool frame
static double interpolate_temperature_at(const fe_tool &ft, glm::dvec2 p) {
	const auto &nodes = ft.nodes_tool_frame();
	const auto &tris = ft.triangles();

	for (const auto &tri : tris) {
		unsigned int i0 = tri[0], i1 = tri[1], i2 = tri[2];
		if (i0 >= nodes.size() || i1 >= nodes.size() || i2 >= nodes.size()) continue;

		const glm::dvec2 &a = nodes[i0];
		const glm::dvec2 &b = nodes[i1];
		const glm::dvec2 &c = nodes[i2];

		// Compute barycentric coordinates
		double denom = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
		if (denom == 0.0) continue;

		double w0 = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / denom;
		double w1 = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / denom;
		double w2 = 1.0 - w0 - w1;

		// Check if point is inside or on triangle
		if (w0 >= -1e-12 && w1 >= -1e-12 && w2 >= -1e-12) {
			return w0 * ft.temperature_at_node(i0) +
			       w1 * ft.temperature_at_node(i1) +
			       w2 * ft.temperature_at_node(i2);
		}
	}
	// Fallback: return temperature of nearest node
	if (nodes.empty()) {
		return 0.0;
	}
	unsigned int best = 0;
	double best_d2 = 1e300;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		double d2 = glm::dot(nodes[i] - p, nodes[i] - p);
		if (d2 < best_d2) {
			best_d2 = d2;
			best = i;
		}
	}
	return ft.temperature_at_node(best);
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

	ft.set_dirichlet_on_physical(1, 100.0);

	const double alpha = mat.k / (mat.rho * mat.cp);
	const double t_final = 0.002;
	const double dt = 1.0e-7;
	double dt_crit = ft.thermal_dt_crit();
	assert(dt <= 0.9 * dt_crit && "Time step violates stability criterion");
	unsigned int nstep = static_cast<unsigned int>(t_final / dt);
	for (unsigned int s = 0; s < nstep; s++) ft.advance_explicit(dt);

	// Sample temperature at center point using barycentric interpolation
	glm::dvec2 target(0.005, 0.0005);
	double T_num = interpolate_temperature_at(ft, target);
	double T_ref = analytic_dirichlet_neumann(target.x, t_final, L, alpha, 100.0);
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
	particle *pp = &b.get_particles()[0];

	fe_tool ft = make_rect_tool_mesh(1.0, 1.0, 3, 3, 1, 2, 3);
	fe_tool::thermal_material mat;
	mat.rho = 7800.0;
	mat.cp = 500.0;
	mat.k = 1.0e6;
	ft.set_material(mat);
	ft.set_mu(0.5);
	ft.set_pose(glm::dvec2(0.), glm::dvec2(0.));
	ft.set_initial_temperature(p.T);
	b.set_fe_tool(&ft);

	simulation_time *time = &simulation_time::getInstance();
	time->set_dt(1.0e-3);
	time->set_t_final(1.0e-3);

	pp->T_t = 0.;
	b.apply_contact();
	std::printf("inside=%g\n", ft.inside(glm::dvec2(pp->x, pp->y)));

	glm::dvec2 F_t(pp->ftx, pp->fty);
	glm::dvec2 F_n(pp->fcx, pp->fcy);
	double Fn = glm::length(F_n);
	if (Fn <= 0.) {
		std::printf("friction Fn=%g\n", Fn);
		return false;
	}

	glm::dvec2 n = glm::normalize(F_n);
	glm::dvec2 v_rel(pp->vx, pp->vy);
	glm::dvec2 vt = v_rel - glm::dot(v_rel, n) * n;
	double slip = glm::length(vt);
	double P_fric = glm::length(F_t) * slip;
	if (P_fric <= 0.0) {
		std::printf("friction P_fric=%g (no slip or no friction force)\n", P_fric);
		return false;
	}

	double dE_p = pp->m * pc.tc().cp() * (time->get_dt() * pp->T_t);
	double frac_wp = ft.get_contact_energy_balance().frac_workpiece;
	if (frac_wp <= 0.0) {
		std::printf("friction frac_workpiece=%g (no heat partition to workpiece)\n", frac_wp);
		return false;
	}
	double ratio = dE_p / (frac_wp * P_fric * time->get_dt());
	std::printf("friction ratio=%g P_fric=%g dE_p=%g frac_wp=%g\n", ratio, P_fric, dE_p, frac_wp);
	return std::abs(ratio - 1.0) <= 0.1;
}

static bool test_coupled_timestep_estimator() {
	physical_constants pc = matlib_tial6v4_Sima_tanh2010_SI();
	fe_tool ft = make_rect_tool_mesh(0.001, 0.001, 5, 5, 1, 2, 3);

	fe_tool::thermal_material tmat;
	tmat.rho = 14500.0;
	tmat.cp = 200.0;
	tmat.k = 80.0;
	ft.set_material(tmat);

	fe_tool::mechanical_material mmat;
	mmat.E = 600e9;
	mmat.nu = 0.22;
	mmat.alpha = 4.5e-6;
	ft.set_mechanical_material(mmat);
	ft.set_initial_temperature(293.15);

	coupled_timestep_config cfg;
	cfg.particle_spacing = 2.0e-5;
	cfg.smoothing_length_ratio = 1.7;
	cfg.max_relative_speed = 10.0;
	cfg.empirical_dt_cap = 1.0e-3;
	cfg.interface_contact_area = cfg.particle_spacing * cfg.particle_spacing;
	cfg.contact_conductance_full = 1.0e5;
	coupled_timestep_limits limits = estimate_coupled_timestep(pc, cfg, &ft);

	bool ok = limits.maximum_dt > 0. && std::isfinite(limits.maximum_dt) &&
	          limits.workpiece_mechanical_dt > 0. && limits.workpiece_thermal_dt > 0. &&
	          limits.tool_mechanical_dt > 0. && limits.tool_thermal_dt > 0. && limits.interface_thermal_dt > 0.;
	std::printf("timestep_estimator dt=%e limiter=%s wp_mech=%e wp_therm=%e tool_mech=%e tool_therm=%e interface=%e\n",
	            limits.maximum_dt,
	            limits.limiting_reason.c_str(),
	            limits.workpiece_mechanical_dt,
	            limits.workpiece_thermal_dt,
	            limits.tool_mechanical_dt,
	            limits.tool_thermal_dt,
	            limits.interface_thermal_dt);
	return ok;
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
	double dt_crit = ft.thermal_dt_crit();
	assert(dt <= 0.9 * dt_crit && "Time step violates stability criterion");
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
#if defined(_WIN32)
	_putenv_s("MFREE_DEFORMABLE_FE_TOOL", "");
	_putenv_s("MFREE_USE_FE_TOOL_FOR_CONTACT", "");
#else
	unsetenv("MFREE_DEFORMABLE_FE_TOOL");
	unsetenv("MFREE_USE_FE_TOOL_FOR_CONTACT");
#endif
	bool ok = true;
	bool ok1 = test_tool_1d_conduction();
	bool ok2 = test_frictional_heating_partition();
	bool ok3 = test_convection_lumped();
	bool ok4 = test_coupled_timestep_estimator();
	std::printf("tool_1d_conduction %s\n", ok1 ? "ok" : "fail");
	std::printf("friction_partition %s\n", ok2 ? "ok" : "fail");
	std::printf("convection_lumped %s\n", ok3 ? "ok" : "fail");
	std::printf("coupled_timestep_estimator %s\n", ok4 ? "ok" : "fail");
	ok = ok1 && ok2 && ok3 && ok4;

	if (!ok) {
		std::printf("validation_failed\n");
		return 1;
	}

	std::printf("validation_ok\n");
	return 0;
}

```


## Assistant

<thinking>
**Evaluating code compilation**

I need to compile the code, but maybe the "pc" variable is used in a timestep after setting the material, which could be good. I should check if the workpiece thermal conductivity is in the material library. There's a potential code issue: the "test_cuttings" includes the "timestep_estimator" through the header, but does it properly handle the "pc" variable? It seems like “single” has "pc". Okay, I need to build this properly.
</thinking>

**Tool Call: cmake --build build --config Release**
Status: Completed

Terminal:
```
CMake is re-running because D:/mfree_iwf_ul_cut_refine_windows_ver/build/CMakeFiles/generate.stamp is out-of-date.
  the file 'D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt'
  is newer than 'D:/mfree_iwf_ul_cut_refine_windows_ver/build/CMakeFiles/generate.stamp.depend'
  result='-1'
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- Configuring done (0.3s)
-- Generating done (0.3s)
-- Build files have been written to: D:/mfree_iwf_ul_cut_refine_windows_ver/build
MSBuild version 17.14.40+3e7442088 for .NET Framework

  mfree_geom_validation_math_test.vcxproj -> D:\mfree_iwf_ul_cut_refine_windows_ver\build\Release\mfree_geom_v
  alidation_math_test.exe
  adaptivity.cpp
  body.cpp
  cont_mech.cpp
  contact.cpp
  correctors.cpp
  derivatives.cpp
  fe_tool.cpp
  grid.cpp
  leap_frog.cpp
  logger.cpp
  material.cpp
  plasticity.cpp
  refine_cut_main.cpp
  thermal.cpp
  timestep_estimator.cpp
  vtk_writer.cpp
  test_benches.cpp
  test_cuttings.cpp
  test_density.cpp
  Generating Code...
  mfree_iwf.vcxproj -> D:\mfree_iwf_ul_cut_refine_windows_ver\build\Release\mfree_iwf.exe
  adaptivity.cpp
  body.cpp
  cont_mech.cpp
  contact.cpp
  correctors.cpp
  derivatives.cpp
  fe_tool.cpp
  grid.cpp
  leap_frog.cpp
  logger.cpp
  material.cpp
  plasticity.cpp
  thermal.cpp
  timestep_estimator.cpp
  vtk_writer.cpp
  validate_main.cpp
  Generating Code...
  mfree_iwf_validate.vcxproj -> D:\mfree_iwf_ul_cut_refine_windows_ver\build\Release\mfree_iwf_validate.exe

```


## Assistant

Need run validation maybe ctest.

**Tool Call: ctest -C Release --test-dir build --output-on-failure**
Status: Completed

Terminal:
```
Test project D:/mfree_iwf_ul_cut_refine_windows_ver/build
    Start 1: mfree_iwf_validate
1/5 Test #1: mfree_iwf_validate .......................   Passed    0.22 sec
    Start 2: mfree_iwf_preprocess_model1_rigid
2/5 Test #2: mfree_iwf_preprocess_model1_rigid ........   Passed    0.25 sec
    Start 3: mfree_iwf_preprocess_model1_fe_only
3/5 Test #3: mfree_iwf_preprocess_model1_fe_only ......   Passed    0.18 sec
    Start 4: mfree_iwf_geom_validate_model1_fe_only
4/5 Test #4: mfree_iwf_geom_validate_model1_fe_only ...   Passed    0.15 sec
    Start 5: mfree_geom_validation_math_test
5/5 Test #5: mfree_geom_validation_math_test ..........   Passed    0.02 sec

100% tests passed, 0 tests failed out of 5

Total Test time (real) =   0.91 sec

```


## Assistant

<thinking>
**Gathering method documentation**

I need to find the necessary docs that might help me determine and create the method the user asked about. It feels like I'll have to document everything properly, so maybe I'll consider using an edit to create a new doc. That sounds like a plan! And hmm, I might also need to think about including any diagnostics that could be relevant. It seems I have a bit of work ahead of me!
</thinking>

**Tool Call: No matches**
Status: Completed

```json
{
  "offset": 0,
  "current_matches_page": [],
  "all_matches_len": 0
}
```

## Assistant

Create docs maybe "coupled_timestep_estimation.md". Need no read? Need list directory exists known. create file.

**Tool Call: docs\\coupled\_timestep\_estimation.md**
Status: Completed

Diff: docs\coupled_timestep_estimation.md
```
# Coupled thermal-structural timestep estimation

The cutting setups now use a central timestep estimator in `src/timestep_estimator.h` / `src/timestep_estimator.cpp` for the SI-unit FE-tool cases. The estimator returns the minimum stable timestep over the active workpiece, FE-tool, and contact-interface limits.

## Limits considered

For the SPH workpiece, the estimator computes:

- mechanical/acoustic limit using the workpiece elastic wave speed `physical_constants::c0()` and the largest expected relative cutting speed;
- thermal diffusion limit using the workpiece thermal diffusivity `k / (rho * cp)` from `physical_constants::tc()`.

For the FE tool, when a tool is available, it computes:

- explicit structural limit from `fe_tool::mechanics_dt_crit()`, which accounts for the current FE mesh size and temperature-dependent `rho`, `E`, and `nu` material tables;
- explicit thermal limit from `fe_tool::thermal_dt_crit()`, which accounts for the current FE conduction operator, lumped capacities, convection, and Dirichlet boundaries;
- interface thermal exchange limit for a two-capacity contact pair using the SPH particle heat capacity and the minimum FE tool nodal heat capacity.

The interface estimate uses the full-contact conductance as the conservative contact conductance and the effective contact area, defaulting to one SPH particle area.

## Runtime controls

The cutting setup helper `estimate_dt_for_cutting()` honors these environment variables:

- `MFREE_TIMESTEP_WP_MECH_SAFETY`
- `MFREE_TIMESTEP_WP_THERM_SAFETY`
- `MFREE_TIMESTEP_TOOL_MECH_SAFETY`
- `MFREE_TIMESTEP_TOOL_THERM_SAFETY`
- `MFREE_TIMESTEP_INTERFACE_SAFETY`
- `MFREE_TIMESTEP_INTERFACE_AREA_FACTOR`
- `MFREE_THERMAL_H_FULL`
- `MFREE_TIMESTEP_PRINT=0` to suppress printed timestep reports

The estimator is called once before the FE tool is attached, then again after the FE tool is loaded and positioned so the final timestep includes the actual FE material tables, FE mesh, and interface capacity scale.

## Output

When printing is enabled, the run emits a line like:

`timestep estimate: dt=... limiter=... wp_mech=... wp_therm=... tool_mech=... tool_therm=... interface_therm=... empirical=...`

The `limiter` field identifies which physical process selected the final maximum timestep.

```


## Assistant

<thinking>
**Considering configurations and diagnostics**

I think it might be a good idea to run an editor config to ensure everything is set up properly. It's important to check all the configurations to avoid potential issues down the line. After that, running diagnostics seems like a smart move to see if there are any errors or warnings that need attention. This way, I can be sure everything is functioning as it should. Alright, let's see how it goes!
</thinking>

**Tool Call: python scripts/check_editorconfig_basic.py**
Status: Completed

Terminal:
```






```


**Tool Call: Check project diagnostics**
Status: Completed

D:\mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.h: 5 error(s), 0 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\contact.cpp: 2 error(s), 0 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\thermal.h: 2 error(s), 0 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\validate_main.cpp: 21 error(s), 0 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\body.cpp: 2 error(s), 1 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\benchmarks\test_cuttings.cpp: 2 error(s), 1 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\body.h: 4 error(s), 0 warning(s)
D:\mfree_iwf_ul_cut_refine_windows_ver\src\material.h: 2 error(s), 0 warning(s)


**Tool Call: Check diagnostics for `mfree_iwf_ul_cut_refine_windows_ver/src/timestep_estimator.cpp`**
Status: Completed

File doesn't have errors or warnings!

**Tool Call: Check diagnostics for `mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp`**
Status: Completed

error at line 51: In included file: 'glm/glm.hpp' file not found with <angled> include; use "quotes" instead
warning at line 57: Included header filesystem is not used directly (fix available)
error at line 453: No member named 'filesystem' in namespace 'std'


## Assistant

<thinking>
**Finalizing diagnostics details**

I want to make sure my summary is succinct. I should mention any preexisting diagnostics. It seems like the issue might be related to the language server configuration not being built correctly. I also need to note that the build and tests passed. Additionally, I should list the files that were changed. I'll use markdown for clear formatting, but there’s no need for any code blocks. Just keeping it clear and concise!
</thinking>

Implemented a coupled thermal-structural maximum timestep estimator and integrated it into the FE-tool cutting setups.

### What changed

- Added new central estimator:
  - `src/timestep_estimator.h`
  - `src/timestep_estimator.cpp`

- The estimator considers:
  - SPH workpiece mechanical/acoustic CFL limit via `physical_constants::c0()`
  - SPH workpiece thermal diffusion limit via `k / (rho * cp)`
  - FE tool explicit structural limit via `fe_tool::mechanics_dt_crit()`
  - FE tool explicit thermal limit via `fe_tool::thermal_dt_crit()`
  - SPH/FE interface thermal exchange limit using contact conductance and heat capacities
  - Optional empirical cap as a final upper bound

- Added FE tool API:
  - `fe_tool::min_thermal_nodal_capacity()`

- Integrated the estimator into the SI-unit cutting setup paths in `src/benchmarks/test_cuttings.cpp`:
  - initial estimate before FE tool load
  - final estimate after FE tool mesh/materials/tables are loaded

- Added validation coverage in `src/benchmarks/validate_main.cpp`.

- Added documentation:
  - `docs/coupled_timestep_estimation.md`

### Runtime controls added

The timestep estimator supports these environment variables:

- `MFREE_TIMESTEP_WP_MECH_SAFETY`
- `MFREE_TIMESTEP_WP_THERM_SAFETY`
- `MFREE_TIMESTEP_TOOL_MECH_SAFETY`
- `MFREE_TIMESTEP_TOOL_THERM_SAFETY`
- `MFREE_TIMESTEP_INTERFACE_SAFETY`
- `MFREE_TIMESTEP_INTERFACE_AREA_FACTOR`
- `MFREE_TIMESTEP_PRINT=0`
- Uses existing `MFREE_THERMAL_H_FULL` as the conservative interface conductance value

When enabled, it prints a summary like:

`timestep estimate: dt=... limiter=... wp_mech=... wp_therm=... tool_mech=... tool_therm=... interface_therm=... empirical=...`

### Validation

Successfully ran:

- `cmake --build build --config Release`
- `ctest -C Release --test-dir build --output-on-failure`
- `python scripts/check_editorconfig_basic.py`

CTest result: **5/5 tests passed**.

Diagnostics still show some IDE/include-resolution issues around `glm` and `std::filesystem`, but the actual CMake/MSVC build and test suite pass successfully.

