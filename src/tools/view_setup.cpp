#include <filesystem>
#include <string>
#include <iostream>
#include <cfloat>
#include <cstdio>
#include <cstdlib>

#include "vtk_writer.h"
#include "test_cuttings.h"
#include "simulation_time.h"
#include "logger.h"

logger *global_logger = nullptr;

static void write_workpiece_outline_vtk(const std::string& path, const body& b) {
	const auto& particles = b.get_particles();
	double xmin = DBL_MAX, xmax = -DBL_MAX;
	double ymin = DBL_MAX, ymax = -DBL_MAX;

	for (const auto& p : particles) {
		xmin = std::min(xmin, p.x);
		xmax = std::max(xmax, p.x);
		ymin = std::min(ymin, p.y);
		ymax = std::max(ymax, p.y);
	}

	FILE* fp = std::fopen(path.c_str(), "w");
	if (!fp) {
		std::cerr << "Failed to open " << path << std::endl;
		std::abort();
	}

	std::fprintf(fp, "# vtk DataFile Version 2.0\n");
	std::fprintf(fp, "workpiece_outline\n");
	std::fprintf(fp, "ASCII\n");
	std::fprintf(fp, "DATASET POLYDATA\n");
	std::fprintf(fp, "POINTS 4 double\n");
	std::fprintf(fp, "%e %e 0\n", xmin, ymin);
	std::fprintf(fp, "%e %e 0\n", xmax, ymin);
	std::fprintf(fp, "%e %e 0\n", xmax, ymax);
	std::fprintf(fp, "%e %e 0\n", xmin, ymax);
	std::fprintf(fp, "POLYGONS 1 5\n");
	std::fprintf(fp, "4 0 1 2 3\n");
	std::fclose(fp);
}

int main(int argc, char** argv) {
	int model = 1;
	std::string out_dir = "results/setup";

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-m" && i + 1 < argc) {
			model = std::atoi(argv[++i]);
		} else if (arg == "--out" && i + 1 < argc) {
			out_dir = argv[++i];
		}
	}

	std::filesystem::create_directories(out_dir);

	int nx = 31;
	body* b = nullptr;
	switch (model) {
	case 1:
		b = cutting_ref_single_resol(nx);
		break;
	case 2:
		nx = 61;
		b = cutting_ref_multi_resol_dynamic(nx);
		break;
	case 3:
		nx = 61;
		b = cutting_ref_multi_resol_apriori(nx);
		break;
	case 4:
		nx = 61;
		b = cutting_ref_single_resol(nx);
		break;
	default:
		std::cerr << "Unsupported model: " << model << std::endl;
		return 2;
	}

	const auto& particles = b->get_particles();
	const tool* t = b->get_tool();

	simulation_time* time = &simulation_time::getInstance();
	time->set_dt(time->get_dt());
	time->set_t_final(time->get_t_final());

	vtk_writer_write(particles, 0, out_dir.c_str(), "setup", "setup");
	if (t) vtk_writer_write(t, 0, out_dir.c_str(), "setup", "setup");
	write_workpiece_outline_vtk(out_dir + "/workpiece_outline.vtk", *b);

	if (global_logger) {
		delete global_logger;
		global_logger = nullptr;
	}

	std::cout << "Wrote:" << std::endl;
	std::cout << "  " << out_dir << "/setup_000000.vtk" << std::endl;
	std::cout << "  " << out_dir << "/setup_tool_000000.vtk" << std::endl;
	std::cout << "  " << out_dir << "/workpiece_outline.vtk" << std::endl;
	return 0;
}

