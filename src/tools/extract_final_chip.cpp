#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct vtk_data {
	std::vector<std::array<double, 3>> points;
	std::map<std::string, std::vector<double>> scalars;
};

static std::vector<std::string> split_csv_line(const std::string &line) {
	std::vector<std::string> out;
	std::string cur;
	bool quoted = false;
	for (char c : line) {
		if (c == '"') {
			quoted = !quoted;
			continue;
		}
		if (c == ',' && !quoted) {
			out.push_back(cur);
			cur.clear();
		} else {
			cur.push_back(c);
		}
	}
	out.push_back(cur);
	return out;
}

static double env_double(const char *name, double fallback) {
	const char *s = std::getenv(name);
	if (!s || s[0] == '\0') return fallback;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	if (end == s || !std::isfinite(v)) return fallback;
	return v;
}

static bool read_legacy_particle_vtk(const fs::path &path, vtk_data &data) {
	std::ifstream in(path);
	if (!in) return false;

	std::string line;
	std::size_t point_count = 0;
	while (std::getline(in, line)) {
		std::istringstream iss(line);
		std::string key;
		iss >> key;
		if (key == "POINTS") {
			std::string type;
			iss >> point_count >> type;
			data.points.resize(point_count);
			for (std::size_t i = 0; i < point_count; i++) {
				std::getline(in, line);
				std::istringstream pss(line);
				pss >> data.points[i][0] >> data.points[i][1] >> data.points[i][2];
			}
		} else if (key == "SCALARS") {
			std::string name;
			std::string type;
			int comps = 1;
			iss >> name >> type >> comps;
			std::getline(in, line); // LOOKUP_TABLE default
			std::vector<double> values(point_count, 0.0);
			for (std::size_t i = 0; i < point_count; i++) {
				std::getline(in, line);
				std::istringstream vss(line);
				vss >> values[i];
				if (comps > 1) {
					for (int c = 1; c < comps; c++) {
						double ignored = 0.0;
						vss >> ignored;
					}
				}
			}
			data.scalars[name] = std::move(values);
		} else if (key == "VECTORS") {
			for (std::size_t i = 0; i < point_count; i++) std::getline(in, line);
		}
	}
	return !data.points.empty();
}

static double estimate_spacing(const vtk_data &data) {
	std::vector<double> xs;
	std::vector<double> ys;
	xs.reserve(data.points.size());
	ys.reserve(data.points.size());
	for (const auto &p : data.points) {
		xs.push_back(p[0]);
		ys.push_back(p[1]);
	}
	auto min_spacing = [](std::vector<double> v) {
		std::sort(v.begin(), v.end());
		v.erase(std::unique(v.begin(), v.end(), [](double a, double b) { return std::abs(a - b) < 1.0e-14; }), v.end());
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t i = 1; i < v.size(); i++) {
			double d = v[i] - v[i - 1];
			if (std::isfinite(d) && d > 1.0e-14) best = std::min(best, d);
		}
		return best;
	};
	double dx = min_spacing(xs);
	double dy = min_spacing(ys);
	double h = std::min(dx, dy);
	return std::isfinite(h) ? h : 0.0;
}

static fs::path find_latest_particle_vtk(const fs::path &dir) {
	fs::path best;
	int best_id = -1;
	for (const auto &entry : fs::directory_iterator(dir)) {
		if (!entry.is_regular_file()) continue;
		std::string name = entry.path().filename().string();
		if (name.rfind("out_", 0) != 0 || entry.path().extension() != ".vtk") continue;
		std::string id_s = name.substr(4, 6);
		int id = -1;
		try {
			id = std::stoi(id_s);
		} catch (...) {
			continue;
		}
		if (id > best_id) {
			best_id = id;
			best = entry.path();
		}
	}
	return best;
}

static double scalar_at(const vtk_data &data, const std::string &name, std::size_t i, double fallback) {
	auto it = data.scalars.find(name);
	if (it == data.scalars.end() || i >= it->second.size()) return fallback;
	return it->second[i];
}

static void write_vtk_subset(const fs::path &path, const vtk_data &data, const std::vector<char> &include, double cp, double tref, bool all_points) {
	std::vector<std::size_t> ids;
	ids.reserve(data.points.size());
	for (std::size_t i = 0; i < data.points.size(); i++) {
		if (all_points || include[i]) ids.push_back(i);
	}

	std::ofstream out(path);
	out << "# vtk DataFile Version 2.0\n";
	out << "final chip extraction\n";
	out << "ASCII\n\n";
	out << "DATASET UNSTRUCTURED_GRID\n";
	out << "POINTS " << ids.size() << " float\n";
	for (std::size_t id : ids) out << data.points[id][0] << " " << data.points[id][1] << " " << data.points[id][2] << "\n";
	out << "\nCELLS " << ids.size() << " " << 2 * ids.size() << "\n";
	for (std::size_t i = 0; i < ids.size(); i++) out << "1 " << i << "\n";
	out << "\nCELL_TYPES " << ids.size() << "\n";
	for (std::size_t i = 0; i < ids.size(); i++) out << "1\n";
	out << "\nPOINT_DATA " << ids.size() << "\n";

	for (const auto &kv : data.scalars) {
		out << "SCALARS " << kv.first << " double 1\n";
		out << "LOOKUP_TABLE default\n";
		for (std::size_t id : ids) out << kv.second[id] << "\n";
		out << "\n";
	}

	out << "SCALARS chip_flag int 1\nLOOKUP_TABLE default\n";
	for (std::size_t id : ids) out << (include[id] ? 1 : 0) << "\n";
	out << "\n";

	out << "SCALARS thermal_energy_delta double 1\nLOOKUP_TABLE default\n";
	for (std::size_t id : ids) {
		double m = scalar_at(data, "mass", id, 0.0);
		double T = scalar_at(data, "temperature", id, tref);
		out << m * cp * (T - tref) << "\n";
	}
	out << "\n";
}

static std::map<std::string, double> parse_energy_row(const std::vector<std::string> &cols, const std::string &line) {
	std::map<std::string, double> values;
	auto vals = split_csv_line(line);
	for (std::size_t i = 0; i < cols.size() && i < vals.size(); i++) {
		char *end = nullptr;
		double v = std::strtod(vals[i].c_str(), &end);
		if (end != vals[i].c_str()) values[cols[i]] = v;
	}
	return values;
}

static bool read_energy_first_final(const fs::path &path, std::map<std::string, double> &first, std::map<std::string, double> &last) {
	std::ifstream in(path);
	if (!in) return false;
	std::string header;
	std::getline(in, header);
	auto cols = split_csv_line(header);
	std::string line;
	std::string first_line;
	std::string last_line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		if (first_line.empty()) first_line = line;
		last_line = line;
	}
	if (first_line.empty() || last_line.empty()) return false;
	first = parse_energy_row(cols, first_line);
	last = parse_energy_row(cols, last_line);
	return true;
}

int main(int argc, char **argv) {
	fs::path results_dir = argc > 1 ? fs::path(argv[1]) : fs::path("D:/mfree_results/tool_plane_strain_explicit_coupled_100000steps_threads_logical_minus_1");
	if (const char *s = std::getenv("MFREE_CHIP_RESULTS_DIR"); s && s[0] != '\0') results_dir = fs::path(s);

	fs::path initial_path = results_dir / "out_000000.vtk";
	fs::path final_path = find_latest_particle_vtk(results_dir);
	if (final_path.empty()) {
		std::cerr << "No out_*.vtk particle files found in " << results_dir << "\n";
		return 1;
	}

	vtk_data initial;
	vtk_data final_data;
	if (!read_legacy_particle_vtk(initial_path, initial)) {
		std::cerr << "Failed to read initial VTK: " << initial_path << "\n";
		return 1;
	}
	if (!read_legacy_particle_vtk(final_path, final_data)) {
		std::cerr << "Failed to read final VTK: " << final_path << "\n";
		return 1;
	}

	double top_y = -std::numeric_limits<double>::infinity();
	for (const auto &p : initial.points) top_y = std::max(top_y, p[1]);
	top_y = env_double("MFREE_CHIP_TOP_Y", top_y);

	double spacing = estimate_spacing(initial);
	double tol = env_double("MFREE_CHIP_Y_TOL", 0.5 * spacing);
	double threshold = top_y + tol;
	double cp = env_double("MFREE_CHIP_CP", 580.0);
	double tref = env_double("MFREE_CHIP_T_REF", 300.0);

	std::vector<char> is_chip(final_data.points.size(), 0);
	double chip_mass = 0.0;
	double retained_mass = 0.0;
	double chip_E_delta = 0.0;
	double retained_E_delta = 0.0;
	double chip_Tmax = -std::numeric_limits<double>::infinity();
	double retained_Tmax = -std::numeric_limits<double>::infinity();
	std::size_t chip_count = 0;
	std::size_t retained_count = 0;

	for (std::size_t i = 0; i < final_data.points.size(); i++) {
		bool chip = final_data.points[i][1] > threshold;
		is_chip[i] = chip ? 1 : 0;
		double m = scalar_at(final_data, "mass", i, 0.0);
		double T = scalar_at(final_data, "temperature", i, tref);
		double dE = m * cp * (T - tref);
		if (chip) {
			chip_count++;
			chip_mass += m;
			chip_E_delta += dE;
			chip_Tmax = std::max(chip_Tmax, T);
		} else {
			retained_count++;
			retained_mass += m;
			retained_E_delta += dE;
			retained_Tmax = std::max(retained_Tmax, T);
		}
	}

	std::map<std::string, double> first_energy;
	std::map<std::string, double> final_energy;
	double tool_delta = 0.0;
	double tool_convection_loss = 0.0;
	if (read_energy_first_final(results_dir / "cutting_energy.csv", first_energy, final_energy)) {
		tool_convection_loss = std::abs(final_energy["cum_tool_E_convection"]);
		tool_delta = final_energy["tool_internal_E"] - first_energy["tool_internal_E"];
	}

	double tool_with_loss = tool_delta + tool_convection_loss;
	double total = chip_E_delta + retained_E_delta + tool_with_loss;
	double tool_fraction = total > 0.0 ? 100.0 * tool_with_loss / total : 0.0;
	double chip_fraction = total > 0.0 ? 100.0 * chip_E_delta / total : 0.0;
	double retained_fraction = total > 0.0 ? 100.0 * retained_E_delta / total : 0.0;

	write_vtk_subset(results_dir / "chip_final.vtk", final_data, is_chip, cp, tref, false);
	std::vector<char> retained(final_data.points.size(), 0);
	for (std::size_t i = 0; i < is_chip.size(); i++) retained[i] = is_chip[i] ? 0 : 1;
	write_vtk_subset(results_dir / "retained_workpiece_final.vtk", final_data, retained, cp, tref, false);
	write_vtk_subset(results_dir / "classified_particles_final.vtk", final_data, is_chip, cp, tref, true);

	std::ofstream csv(results_dir / "chip_final_summary.csv");
	csv << std::setprecision(17);
	csv << "results_dir,initial_vtk,final_vtk,top_y,spacing,y_tol,chip_threshold,cp,t_ref,final_particle_count,chip_count,retained_count,";
	csv << "chip_mass,retained_mass,chip_E_delta,retained_E_delta,tool_E_delta,tool_convection_loss,tool_E_with_loss,total_heat_inventory,";
	csv << "tool_fraction_percent,chip_fraction_percent,retained_fraction_percent,chip_Tmax,retained_Tmax\n";
	csv << results_dir.string() << "," << initial_path.filename().string() << "," << final_path.filename().string() << ",";
	csv << top_y << "," << spacing << "," << tol << "," << threshold << "," << cp << "," << tref << "," << final_data.points.size() << ",";
	csv << chip_count << "," << retained_count << "," << chip_mass << "," << retained_mass << "," << chip_E_delta << "," << retained_E_delta << ",";
	csv << tool_delta << "," << tool_convection_loss << "," << tool_with_loss << "," << total << ",";
	csv << tool_fraction << "," << chip_fraction << "," << retained_fraction << "," << chip_Tmax << "," << retained_Tmax << "\n";

	std::cout << "Final particle VTK: " << final_path << "\n";
	std::cout << "Chip threshold y: " << threshold << "\n";
	std::cout << "Chip particles: " << chip_count << " / " << final_data.points.size() << "\n";
	std::cout << "Wrote: " << (results_dir / "chip_final_summary.csv") << "\n";
	std::cout << "Wrote: " << (results_dir / "chip_final.vtk") << "\n";
	std::cout << "Wrote: " << (results_dir / "retained_workpiece_final.vtk") << "\n";
	std::cout << "Wrote: " << (results_dir / "classified_particles_final.vtk") << "\n";
	return 0;
}
