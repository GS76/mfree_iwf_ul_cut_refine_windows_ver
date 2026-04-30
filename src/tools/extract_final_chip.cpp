// extract_final_chip.cpp
//
// Post-processor: reads the final (or any specified) particle VTK frame from a
// simulation results folder, classifies particles into chip / retained workpiece,
// computes per-region thermal energy deltas, and writes:
//   chip_final.vtk
//   retained_workpiece_final.vtk
//   classified_particles_final.vtk
//   chip_final_summary.csv
//
// Chip classification criterion
// --------------------------------
// A particle is "chip" when its CURRENT y-position exceeds the original top
// edge of the workpiece plus a tolerance:
//   chip_i  iff  y_i > max(initial_y) + tol
//
// Since Phase 3, every particle VTK contains scalar fields:
//   initial_x, initial_y        -- reference-frame position (particle.X / .Y)
//   initial_temperature         -- temperature at particle creation (particle.T_init)
//
// When these fields are present (new VTKs) the tool is fully self-contained:
//   * top_y   = max(initial_y) over all particles in the final frame
//   * spacing = min inter-particle spacing estimated from initial positions
//   * dE_i    = m_i * cp * (T_i - T_init_i)   (per-particle reference)
//
// When the fields are absent (pre-Phase-3 VTKs), the tool falls back to:
//   * reading out_000000.vtk for top_y / spacing
//   * using MFREE_CHIP_T_REF (default 300 K) as global T_ref
//
// Energy CSV integration
// --------------------------------
// The tool reads <results_dir>/*_energy.csv (or cutting_energy.csv) and
// extracts the final-row values. Since Phase 2 the CSV contains:
//   delta_tool_internal_E   -- tool thermal energy gain above T_ref baseline
//   cum_tool_E_convection   -- cumulative convection loss from tool
// Older CSVs fall back to computing the tool delta from
// tool_internal_E_above_ref (or tool_internal_E) first/last row difference.
//
// Environment variable overrides
// --------------------------------
//   MFREE_CHIP_RESULTS_DIR   override results directory
//   MFREE_CHIP_TOP_Y         override workpiece top edge (m)
//   MFREE_CHIP_Y_TOL         override chip-threshold tolerance (m)
//   MFREE_CHIP_CP            override workpiece cp (J/kg/K)
//   MFREE_CHIP_T_REF         fallback global T_ref when initial_temperature absent (K)

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

// ---------------------------------------------------------------------------
// VTK data container
// ---------------------------------------------------------------------------
struct vtk_data {
	std::vector<std::array<double, 3>> points;
	std::map<std::string, std::vector<double>> scalars;
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::vector<std::string> split_csv_line(const std::string &line) {
	std::vector<std::string> out;
	std::string cur;
	bool quoted = false;
	for (char c : line) {
		if (c == '"') { quoted = !quoted; continue; }
		if (c == ',' && !quoted) { out.push_back(cur); cur.clear(); }
		else cur.push_back(c);
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

static double scalar_at(const vtk_data &data, const std::string &name,
                        std::size_t i, double fallback) {
	auto it = data.scalars.find(name);
	if (it == data.scalars.end() || i >= it->second.size()) return fallback;
	return it->second[i];
}

static bool has_scalar(const vtk_data &data, const std::string &name) {
	return data.scalars.count(name) > 0;
}

// ---------------------------------------------------------------------------
// VTK reader (legacy ASCII unstructured particle VTK)
// ---------------------------------------------------------------------------
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
			std::string name, type;
			int comps = 1;
			iss >> name >> type >> comps;
			std::getline(in, line); // LOOKUP_TABLE default
			std::vector<double> values(point_count, 0.0);
			for (std::size_t i = 0; i < point_count; i++) {
				std::getline(in, line);
				std::istringstream vss(line);
				vss >> values[i];
				for (int c = 1; c < comps; c++) { double ign = 0.0; vss >> ign; }
			}
			data.scalars[name] = std::move(values);
		} else if (key == "VECTORS") {
			for (std::size_t i = 0; i < point_count; i++) std::getline(in, line);
		}
	}
	return !data.points.empty();
}

// ---------------------------------------------------------------------------
// Particle spacing estimator (min inter-particle spacing in x or y)
// ---------------------------------------------------------------------------
static double estimate_spacing(const vtk_data &data) {
	std::vector<double> xs, ys;
	xs.reserve(data.points.size());
	ys.reserve(data.points.size());
	for (const auto &p : data.points) { xs.push_back(p[0]); ys.push_back(p[1]); }

	auto min_spacing = [](std::vector<double> v) {
		std::sort(v.begin(), v.end());
		v.erase(std::unique(v.begin(), v.end(),
		                    [](double a, double b) { return std::abs(a - b) < 1.0e-14; }),
		        v.end());
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

// ---------------------------------------------------------------------------
// Find the latest out_NNNNNN.vtk in a directory
// ---------------------------------------------------------------------------
static fs::path find_latest_particle_vtk(const fs::path &dir) {
	fs::path best;
	int best_id = -1;
	for (const auto &entry : fs::directory_iterator(dir)) {
		if (!entry.is_regular_file()) continue;
		std::string name = entry.path().filename().string();
		if (name.rfind("out_", 0) != 0 || entry.path().extension() != ".vtk") continue;
		std::string id_s = name.substr(4, 6);
		int id = -1;
		try { id = std::stoi(id_s); } catch (...) { continue; }
		if (id > best_id) { best_id = id; best = entry.path(); }
	}
	return best;
}

// ---------------------------------------------------------------------------
// Find the energy CSV in a results directory.
// Accepts *_energy.csv (any prefix) or the legacy cutting_energy.csv.
// ---------------------------------------------------------------------------
static fs::path find_energy_csv(const fs::path &dir) {
	fs::path legacy = dir / "cutting_energy.csv";
	fs::path best;
	for (const auto &entry : fs::directory_iterator(dir)) {
		if (!entry.is_regular_file()) continue;
		std::string name = entry.path().filename().string();
		if (name.size() > 11 &&
		    name.substr(name.size() - 11) == "_energy.csv")
			best = entry.path();
	}
	if (!best.empty()) return best;
	if (fs::exists(legacy)) return legacy;
	return {};
}

// ---------------------------------------------------------------------------
// Read the last row of the energy CSV into a column-name → value map.
// Also optionally reads the first data row (for backward-compat fallback).
// ---------------------------------------------------------------------------
static bool read_energy_csv(const fs::path &path,
                            std::map<std::string, double> &last_row,
                            std::map<std::string, double> *first_row = nullptr) {
	std::ifstream in(path);
	if (!in) return false;
	std::string header;
	std::getline(in, header);
	auto cols = split_csv_line(header);

	auto parse_row = [&](const std::string &line) {
		std::map<std::string, double> row;
		auto vals = split_csv_line(line);
		for (std::size_t i = 0; i < cols.size() && i < vals.size(); i++) {
			char *end = nullptr;
			double v = std::strtod(vals[i].c_str(), &end);
			if (end != vals[i].c_str()) row[cols[i]] = v;
		}
		return row;
	};

	std::string line, first_line, last_line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		if (first_line.empty()) first_line = line;
		last_line = line;
	}
	if (last_line.empty()) return false;

	last_row = parse_row(last_line);
	if (first_row && !first_line.empty())
		*first_row = parse_row(first_line);
	return true;
}

// ---------------------------------------------------------------------------
// VTK subset writer
// Writes chip or retained particles (or all, when all_points=true) from data.
// Automatically includes all scalar fields present in data (including the new
// initial_x, initial_y, initial_temperature fields added in Phase 3).
// The thermal_energy_delta field uses initial_temperature per particle when
// available, otherwise falls back to the global tref.
// ---------------------------------------------------------------------------
static void write_vtk_subset(const fs::path &path, const vtk_data &data,
                             const std::vector<char> &include,
                             double cp, double tref, bool all_points) {
	std::vector<std::size_t> ids;
	ids.reserve(data.points.size());
	for (std::size_t i = 0; i < data.points.size(); i++) {
		if (all_points || include[i]) ids.push_back(i);
	}

	const bool has_T_init = has_scalar(data, "initial_temperature");

	std::ofstream out(path);
	out << "# vtk DataFile Version 2.0\n";
	out << "final chip extraction\n";
	out << "ASCII\n\n";
	out << "DATASET UNSTRUCTURED_GRID\n";
	out << "POINTS " << ids.size() << " float\n";
	for (std::size_t id : ids)
		out << data.points[id][0] << " " << data.points[id][1] << " "
		    << data.points[id][2] << "\n";
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

	// Per-particle thermal energy delta above the particle's own initial
	// temperature (when available) or the global tref fallback.
	out << "SCALARS thermal_energy_delta double 1\nLOOKUP_TABLE default\n";
	for (std::size_t id : ids) {
		double m  = scalar_at(data, "mass",                id, 0.0);
		double T  = scalar_at(data, "temperature",         id, tref);
		double T0 = has_T_init
		                ? scalar_at(data, "initial_temperature", id, tref)
		                : tref;
		out << m * cp * (T - T0) << "\n";
	}
	out << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char **argv) {
	fs::path results_dir = argc > 1
	                           ? fs::path(argv[1])
	                           : fs::path("D:/mfree_results/"
	                                      "tool_plane_strain_explicit_coupled_"
	                                      "100000steps_threads_logical_minus_1");
	if (const char *s = std::getenv("MFREE_CHIP_RESULTS_DIR"); s && s[0] != '\0')
		results_dir = fs::path(s);

	// ---- Read final VTK -----------------------------------------------
	fs::path final_path = find_latest_particle_vtk(results_dir);
	if (final_path.empty()) {
		std::cerr << "No out_*.vtk particle files found in " << results_dir << "\n";
		return 1;
	}
	vtk_data final_data;
	if (!read_legacy_particle_vtk(final_path, final_data)) {
		std::cerr << "Failed to read final VTK: " << final_path << "\n";
		return 1;
	}

	// ---- Determine whether the new per-particle fields are present ----
	const bool has_initial_pos  = has_scalar(final_data, "initial_x") &&
	                               has_scalar(final_data, "initial_y");
	const bool has_initial_temp = has_scalar(final_data, "initial_temperature");

	if (has_initial_pos)
		std::cout << "Phase-3 fields detected: using initial_x/initial_y "
		             "from final VTK for chip threshold.\n";
	else
		std::cout << "Phase-3 fields absent: falling back to out_000000.vtk "
		             "for top_y / spacing.\n";

	if (has_initial_temp)
		std::cout << "Phase-3 fields detected: using per-particle "
		             "initial_temperature for dE.\n";
	else
		std::cout << "Phase-3 fields absent: using global MFREE_CHIP_T_REF "
		             "as energy reference.\n";

	// ---- Derive top_y and inter-particle spacing -----------------------
	double top_y   = -std::numeric_limits<double>::infinity();
	double spacing = 0.0;

	if (has_initial_pos) {
		// Preferred path (Phase 3+): extract initial positions from final VTK.
		vtk_data init_approx;
		init_approx.points.resize(final_data.points.size());
		for (std::size_t i = 0; i < final_data.points.size(); i++) {
			double ix = scalar_at(final_data, "initial_x", i, final_data.points[i][0]);
			double iy = scalar_at(final_data, "initial_y", i, final_data.points[i][1]);
			init_approx.points[i] = {ix, iy, 0.};
			top_y = std::max(top_y, iy);
		}
		spacing = estimate_spacing(init_approx);
	} else {
		// Fallback: read the initial frame VTK.
		fs::path initial_path = results_dir / "out_000000.vtk";
		vtk_data initial;
		if (read_legacy_particle_vtk(initial_path, initial)) {
			for (const auto &p : initial.points) top_y = std::max(top_y, p[1]);
			spacing = estimate_spacing(initial);
		} else {
			std::cerr << "Warning: out_000000.vtk not found and initial_y "
			             "absent. Estimating top_y from final particle "
			             "positions.\n";
			for (const auto &p : final_data.points) top_y = std::max(top_y, p[1]);
			spacing = estimate_spacing(final_data);
		}
	}

	// Allow env-var overrides regardless of which path was taken.
	top_y   = env_double("MFREE_CHIP_TOP_Y",  top_y);
	spacing = std::isfinite(spacing) && spacing > 0.0 ? spacing : 0.0;
	double tol       = env_double("MFREE_CHIP_Y_TOL", 0.5 * spacing);
	double threshold = top_y + tol;
	double cp        = env_double("MFREE_CHIP_CP",    580.0);
	double tref      = env_double("MFREE_CHIP_T_REF", 300.0); // global fallback only

	// ---- Classify particles and accumulate per-region energies ---------
	std::vector<char> is_chip(final_data.points.size(), 0);
	double chip_mass     = 0.0, retained_mass     = 0.0;
	double chip_E_delta  = 0.0, retained_E_delta  = 0.0;
	double chip_Tmax     = -std::numeric_limits<double>::infinity();
	double retained_Tmax = -std::numeric_limits<double>::infinity();
	std::size_t chip_count = 0, retained_count = 0;

	for (std::size_t i = 0; i < final_data.points.size(); i++) {
		const bool chip = final_data.points[i][1] > threshold;
		is_chip[i] = chip ? 1 : 0;

		double m  = scalar_at(final_data, "mass",                i, 0.0);
		double T  = scalar_at(final_data, "temperature",         i, tref);
		// Use per-particle T_init when available (Phase 3+), else global tref.
		double T0 = has_initial_temp
		                ? scalar_at(final_data, "initial_temperature", i, tref)
		                : tref;
		double dE = m * cp * (T - T0);

		if (chip) {
			chip_count++;
			chip_mass    += m;
			chip_E_delta += dE;
			chip_Tmax     = std::max(chip_Tmax,     T);
		} else {
			retained_count++;
			retained_mass    += m;
			retained_E_delta += dE;
			retained_Tmax     = std::max(retained_Tmax, T);
		}
	}

	// ---- Read tool energy from the energy CSV --------------------------
	double tool_delta         = 0.0;
	double tool_convection_loss = 0.0;

	fs::path energy_csv = find_energy_csv(results_dir);
	if (!energy_csv.empty()) {
		std::map<std::string, double> last_row, first_row;
		if (read_energy_csv(energy_csv, last_row, &first_row)) {
			tool_convection_loss = std::abs(
			    last_row.count("cum_tool_E_convection")
			        ? last_row["cum_tool_E_convection"]
			        : 0.0);

			// Phase 2+: delta_tool_internal_E is a direct column.
			if (last_row.count("delta_tool_internal_E")) {
				tool_delta = last_row["delta_tool_internal_E"];
			} else if (last_row.count("tool_internal_E_above_ref") &&
			           first_row.count("tool_internal_E_above_ref")) {
				tool_delta = last_row["tool_internal_E_above_ref"] -
				             first_row["tool_internal_E_above_ref"];
			} else if (last_row.count("tool_internal_E") &&
			           first_row.count("tool_internal_E")) {
				// Legacy: absolute-temperature column (pre-Phase-2).
				tool_delta = last_row["tool_internal_E"] -
				             first_row["tool_internal_E"];
			}
		}
	} else {
		std::cerr << "Warning: no energy CSV found in " << results_dir
		          << "; tool energy delta set to 0.\n";
	}

	// ---- Compute summary fractions -------------------------------------
	double tool_with_loss  = tool_delta + tool_convection_loss;
	double total           = chip_E_delta + retained_E_delta + tool_with_loss;
	double chip_fraction     = total > 0.0 ? 100.0 * chip_E_delta   / total : 0.0;
	double retained_fraction = total > 0.0 ? 100.0 * retained_E_delta / total : 0.0;
	double tool_fraction     = total > 0.0 ? 100.0 * tool_with_loss  / total : 0.0;

	// ---- Write output VTKs --------------------------------------------
	std::vector<char> retained_mask(final_data.points.size(), 0);
	for (std::size_t i = 0; i < is_chip.size(); i++)
		retained_mask[i] = is_chip[i] ? 0 : 1;

	write_vtk_subset(results_dir / "chip_final.vtk",
	                 final_data, is_chip, cp, tref, false);
	write_vtk_subset(results_dir / "retained_workpiece_final.vtk",
	                 final_data, retained_mask, cp, tref, false);
	write_vtk_subset(results_dir / "classified_particles_final.vtk",
	                 final_data, is_chip, cp, tref, true);

	// ---- Write summary CSV --------------------------------------------
	std::ofstream csv(results_dir / "chip_final_summary.csv");
	csv << std::setprecision(17);
	csv << "results_dir,final_vtk,"
	       "has_initial_pos,has_initial_temp,"
	       "top_y,spacing,y_tol,chip_threshold,cp,t_ref_fallback,"
	       "final_particle_count,chip_count,retained_count,"
	       "chip_mass,retained_mass,"
	       "chip_E_delta,retained_E_delta,"
	       "tool_E_delta,tool_convection_loss,tool_E_with_loss,"
	       "total_heat_inventory,"
	       "chip_fraction_percent,retained_fraction_percent,tool_fraction_percent,"
	       "chip_Tmax,retained_Tmax\n";
	csv << results_dir.string() << ","
	    << final_path.filename().string() << ","
	    << (has_initial_pos  ? "yes" : "no") << ","
	    << (has_initial_temp ? "yes" : "no") << ","
	    << top_y << "," << spacing << "," << tol << "," << threshold << ","
	    << cp << "," << tref << ","
	    << final_data.points.size() << ","
	    << chip_count << "," << retained_count << ","
	    << chip_mass << "," << retained_mass << ","
	    << chip_E_delta << "," << retained_E_delta << ","
	    << tool_delta << "," << tool_convection_loss << "," << tool_with_loss << ","
	    << total << ","
	    << chip_fraction << "," << retained_fraction << "," << tool_fraction << ","
	    << chip_Tmax << "," << retained_Tmax << "\n";

	// ---- Console summary ----------------------------------------------
	std::cout << "\n";
	std::cout << "Final particle VTK : " << final_path << "\n";
	std::cout << "Chip threshold y   : " << threshold
	          << "  (top_y=" << top_y << " + tol=" << tol << ")\n";
	std::cout << "Energy reference   : "
	          << (has_initial_temp ? "per-particle initial_temperature"
	                               : "global T_ref = " + std::to_string(tref) + " K")
	          << "\n";
	std::cout << "Chip particles     : " << chip_count << " / "
	          << final_data.points.size() << "\n";
	std::cout << "Chip E_delta       : " << chip_E_delta    << " J  ("
	          << chip_fraction    << "%)\n";
	std::cout << "Retained E_delta   : " << retained_E_delta << " J  ("
	          << retained_fraction << "%)\n";
	std::cout << "Tool E_delta       : " << tool_with_loss   << " J  ("
	          << tool_fraction     << "%)  [delta=" << tool_delta
	          << " + convection=" << tool_convection_loss << "]\n";
	std::cout << "Total inventory    : " << total << " J\n";
	std::cout << "\n";
	std::cout << "Wrote: " << (results_dir / "chip_final_summary.csv")        << "\n";
	std::cout << "Wrote: " << (results_dir / "chip_final.vtk")                << "\n";
	std::cout << "Wrote: " << (results_dir / "retained_workpiece_final.vtk")  << "\n";
	std::cout << "Wrote: " << (results_dir / "classified_particles_final.vtk") << "\n";
	return 0;
}
