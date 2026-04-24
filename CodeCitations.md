# Code Citations

## License: GPL-3.0
https://github.com/iwf-inspire/mfree_iwf/blob/f9d3e76ce082279a6fcdc5bc91b5bfcb47595071/src/benchmarks/rings.cpp

```
*test_bench_setup_rings(unsigned int nbox) {
	// material constants (rubber like)
	double E    = 1e7;
	double nu   = 0.4;
	double rho0 = 1;

	physical_constants physical_constants(nu, E, rho0);

	//problem dimensions (monaghan & gray)
	double ri = 0.03;
	double ro = 0.04;
	double spacing = ro + 0.001;

	double dx = 2*ro/(nbox-1);
	double hdx = 1.7;

	double vel_rings =  180.;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

	printf("using timestep %e\n", dt);

	particle *particles = new
```


## License: GPL-3.0
https://github.com/iwf-inspire/mfree_iwf-ul-cut-refine/blob/e8dd8f8483414118369616767ad25192bc839b89/src/benchmarks/test_benches.cpp

```
*test_bench_setup_rings(unsigned int nbox) {
	// material constants (rubber like)
	double E    = 1e7;
	double nu   = 0.4;
	double rho0 = 1;

	physical_constants physical_constants(nu, E, rho0);

	//problem dimensions (monaghan & gray)
	double ri = 0.03;
	double ro = 0.04;
	double spacing = ro + 0.001;

	double dx = 2*ro/(nbox-1);
	double hdx = 1.7;

	double vel_rings =  180.;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

	printf("using timestep %e\n", dt);

	particle *particles = new
```


## License: GPL-3.0
https://github.com/iwf-inspire/mfree_iwf-ul-cut-refine/blob/e8dd8f8483414118369616767ad25192bc839b89/src/benchmarks/test_benches.cpp

```
test_bench_setup_disk_impact(unsigned int nbox) {
	physical_constants physical_constants = matlib_tial6v4_Sima_tanh2010_SI();
	double rho0 = physical_constants.rho0();

	//problem dimensions (monaghan & gray)
	double ro = 0.04;
	double spacing = ro + ro/40;

	double dx = 2*ro/(nbox-1);
	double hdx = 1.7;

	double vel_rings = 180.;

	double c0 = physical_constants.c0();
	double dt = 0.1*dx*hdx/(vel_rings + c0);

	printf("using timestep %e\n", dt);

	particle *particles = new particle[nbox*nbox];

	unsigned int part_iter = 0
```


## License: GPL-3.0
https://github.com/iwf-inspire/mfree_iwf-ul-cut-refine/blob/e8dd8f8483414118369616767ad25192bc839b89/README.md

```
frames presented above can be viewed using [ParaView](https://www.paraview.org/) using the legacy VTK format.

**mfree_iwf-ul_cut-refine** was tested on various versions of Ubuntu Linux. The only dependency is [GLM](https://glm.g-truc.net/0.9.9/index.html). Make files for both a Release version and a Debug build are provided. **mfree_iwf-ul_cut-refine** was developed at _IWF_ [ETHZ](www.ethz.ch) by the
```

