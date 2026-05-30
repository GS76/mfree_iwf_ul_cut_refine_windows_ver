#ifndef MFREE_CONFIG_SIMULATION_CONFIG_H_
#define MFREE_CONFIG_SIMULATION_CONFIG_H_

#include "config/json.h"

#include <optional>
#include <string>

namespace mfree::config {

struct simulation_config {
	int schema_version = 1;
	std::string name;

	struct io_cfg {
		std::string output_dir = "results";
		bool clear_output_dir = true;
		int num_print = 150;
		bool all_steps = false;
	};

	struct model_cfg {
		std::string type = "single_resolution";
		int nbox = 31;
	};

	struct workpiece_cfg {
		double lo_x = 0.0;
		double hi_x = 0.00200;
		double lo_y = 0.00030;
		double hi_y = 0.00080;
		double base_height_y = 0.00030;
		bool keep_base_spacing = true;
	};

	struct time_cfg {
		double cut_length = 1e-3;
		double dt_empirical = 1.0e-9;
		double mech_cfl_factor = 0.50;
		double heat_cfl_factor = 0.50;
		std::optional<double> dt_override;
		std::optional<double> t_final_override;
	};

	struct tool_cfg {
		double cutting_speed = 100.0 / 60.0;
		double rake_deg = -5.0;
		double clearance_deg = 5.0;
		double length = 0.0020;
		double height = 0.0015;
		double tl_y = 0.000986074;
		double fillet_radius = 5e-5;
		double mu_friction = 0.35;
		double target_feed = 2e-4;
		double tool_right_clearance = 0.0;
		double tool_x_shift = 1.25e-4;
	};

	struct numerical_cfg {
		double hdx = 1.5;
		double alpha = 1.0;
		double beta = 1.0;
		double eta = 0.1;
		double xsph_eps = 0.5;
		double art_stress_eps = 0.3;
		double stress_exponent = 4.0;
	};

	struct thermal_cfg {
		bool enabled = true;
		std::string method = "thermal_pse";
		double T0 = 300.0;
	};

	struct plasticity_cfg {
		bool enabled = true;
		std::string model = "johnson_cook_sima_2010";
		double tolerance = 1e-6;
		bool dissipation_considered = true;
	};

	struct material_cfg {
		std::string physical_constants = "tial6v4_sima_tanh2010_si";
	};

	struct multires_cfg {
		double resol_ratio = 2.0;
		double py_split_fraction = 0.5;
		double x_high_res_limit = 0.000117;
		double py_margin_factor = 1.1;
		double py_margin_factor_dynamic = 1.9;
	};

	struct adaptivity_cfg {
		bool enabled = false;
		std::string criterion = "moving_frame";
		std::string pattern = "cubic_basic";
		double alpha_dx = 0.50;
		double beta_h = 0.50;
		double v_cr = 0.40;
		double div_v_cr = 2e5;
		double SvM_cr = 1e7;
		double eps_cr = 110.0;
		double T_cr = 700.0;
		double frame_width = 0.000350;
		double frame_height = 0.000060;
		double xy_min_x = 0.25;
		double xy_min_y = 0.25;
		double xy_max_x = 0.75;
		double xy_max_y = 0.75;
		int n_nbh = 10;
		double l_eff_extra_fraction = 0.1;
		bool allow_refine = true;
	};

	io_cfg io;
	model_cfg model;
	workpiece_cfg workpiece;
	time_cfg time;
	tool_cfg tool;
	numerical_cfg numerical;
	thermal_cfg thermal;
	plasticity_cfg plasticity;
	material_cfg material;
	multires_cfg multires;
	adaptivity_cfg adaptivity;
};

simulation_config load_simulation_config_file(const std::string &file_path);
json_value dump_default_simulation_config_json();
void validate_simulation_config_or_throw(const simulation_config &cfg);

} // namespace mfree::config

#endif
