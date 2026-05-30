#include "config/simulation_config.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>

namespace mfree::config {

static std::string read_text_file_or_throw(const std::string &path) {
	std::ifstream in(path, std::ios::in | std::ios::binary);
	if (!in) {
		throw std::runtime_error("Failed to open config file: " + path);
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	return ss.str();
}

static const json_value &require_key(const json_value::object &o, const std::string &key, const std::string &ctx, int line, int col) {
	auto it = o.find(key);
	if (it == o.end()) {
		throw json_error("Missing required field: " + ctx + "." + key, line, col, ctx);
	}
	return it->second;
}

static const json_value *optional_key(const json_value::object &o, const std::string &key) {
	auto it = o.find(key);
	if (it == o.end())
		return nullptr;
	return &it->second;
}

static int as_int(const json_value &v, const std::string &ctx, int line, int col) {
	if (!v.is_number())
		throw json_error("Expected number for " + ctx, line, col, ctx);
	double d = v.as_number();
	if (!std::isfinite(d))
		throw json_error("Non-finite number for " + ctx, line, col, ctx);
	double r = std::round(d);
	if (std::fabs(d - r) > 1e-9)
		throw json_error("Expected integer for " + ctx, line, col, ctx);
	if (r < (double)std::numeric_limits<int>::min() || r > (double)std::numeric_limits<int>::max())
		throw json_error("Integer out of range for " + ctx, line, col, ctx);
	return (int)r;
}

static double as_double(const json_value &v, const std::string &ctx, int line, int col) {
	if (!v.is_number())
		throw json_error("Expected number for " + ctx, line, col, ctx);
	double d = v.as_number();
	if (!std::isfinite(d))
		throw json_error("Non-finite number for " + ctx, line, col, ctx);
	return d;
}

static bool as_bool(const json_value &v, const std::string &ctx, int line, int col) {
	if (!v.is_bool())
		throw json_error("Expected boolean for " + ctx, line, col, ctx);
	return v.as_bool();
}

static std::string as_string(const json_value &v, const std::string &ctx, int line, int col) {
	if (!v.is_string())
		throw json_error("Expected string for " + ctx, line, col, ctx);
	return v.as_string();
}

static void validate_choice_or_throw(const std::string &value, const std::set<std::string> &choices, const std::string &ctx) {
	if (choices.find(value) != choices.end())
		return;
	std::string msg = "Invalid value for " + ctx + ": " + value + ". Allowed: ";
	bool first = true;
	for (const auto &c : choices) {
		if (!first)
			msg += ", ";
		first = false;
		msg += c;
	}
	throw std::runtime_error(msg);
}

simulation_config load_simulation_config_file(const std::string &file_path) {
	const std::string txt = read_text_file_or_throw(file_path);
	const json_value root = parse_json(txt);
	if (!root.is_object()) {
		throw std::runtime_error("Config root must be a JSON object");
	}

	const int line = 1;
	const int col = 1;
	const auto &o = root.as_object();
	simulation_config cfg;

	{
		const json_value *v = optional_key(o, "schema_version");
		if (v)
			cfg.schema_version = as_int(*v, "schema_version", line, col);
	}
	{
		const json_value *v = optional_key(o, "name");
		if (v)
			cfg.name = as_string(*v, "name", line, col);
	}

	auto parse_io = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("io must be an object", line, col, "io");
		const auto &io = j.as_object();
		if (const json_value *v = optional_key(io, "output_dir"))
			cfg.io.output_dir = as_string(*v, "io.output_dir", line, col);
		if (const json_value *v = optional_key(io, "clear_output_dir"))
			cfg.io.clear_output_dir = as_bool(*v, "io.clear_output_dir", line, col);
		if (const json_value *v = optional_key(io, "num_print"))
			cfg.io.num_print = as_int(*v, "io.num_print", line, col);
		if (const json_value *v = optional_key(io, "all_steps"))
			cfg.io.all_steps = as_bool(*v, "io.all_steps", line, col);
	};

	auto parse_model = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("model must be an object", line, col, "model");
		const auto &mo = j.as_object();
		if (const json_value *v = optional_key(mo, "type"))
			cfg.model.type = as_string(*v, "model.type", line, col);
		if (const json_value *v = optional_key(mo, "nbox"))
			cfg.model.nbox = as_int(*v, "model.nbox", line, col);
	};

	auto parse_workpiece = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("workpiece must be an object", line, col, "workpiece");
		const auto &wo = j.as_object();
		if (const json_value *v = optional_key(wo, "lo_x"))
			cfg.workpiece.lo_x = as_double(*v, "workpiece.lo_x", line, col);
		if (const json_value *v = optional_key(wo, "hi_x"))
			cfg.workpiece.hi_x = as_double(*v, "workpiece.hi_x", line, col);
		if (const json_value *v = optional_key(wo, "lo_y"))
			cfg.workpiece.lo_y = as_double(*v, "workpiece.lo_y", line, col);
		if (const json_value *v = optional_key(wo, "hi_y"))
			cfg.workpiece.hi_y = as_double(*v, "workpiece.hi_y", line, col);
		if (const json_value *v = optional_key(wo, "base_height_y"))
			cfg.workpiece.base_height_y = as_double(*v, "workpiece.base_height_y", line, col);
		if (const json_value *v = optional_key(wo, "keep_base_spacing"))
			cfg.workpiece.keep_base_spacing = as_bool(*v, "workpiece.keep_base_spacing", line, col);
	};

	auto parse_time = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("time must be an object", line, col, "time");
		const auto &to = j.as_object();
		if (const json_value *v = optional_key(to, "cut_length"))
			cfg.time.cut_length = as_double(*v, "time.cut_length", line, col);
		if (const json_value *v = optional_key(to, "dt_empirical"))
			cfg.time.dt_empirical = as_double(*v, "time.dt_empirical", line, col);
		if (const json_value *v = optional_key(to, "mech_cfl_factor"))
			cfg.time.mech_cfl_factor = as_double(*v, "time.mech_cfl_factor", line, col);
		if (const json_value *v = optional_key(to, "heat_cfl_factor"))
			cfg.time.heat_cfl_factor = as_double(*v, "time.heat_cfl_factor", line, col);
		if (const json_value *v = optional_key(to, "dt"))
			cfg.time.dt_override = as_double(*v, "time.dt", line, col);
		if (const json_value *v = optional_key(to, "t_final"))
			cfg.time.t_final_override = as_double(*v, "time.t_final", line, col);
	};

	auto parse_tool = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("tool must be an object", line, col, "tool");
		const auto &to = j.as_object();
		if (const json_value *v = optional_key(to, "cutting_speed"))
			cfg.tool.cutting_speed = as_double(*v, "tool.cutting_speed", line, col);
		if (const json_value *v = optional_key(to, "rake_deg"))
			cfg.tool.rake_deg = as_double(*v, "tool.rake_deg", line, col);
		if (const json_value *v = optional_key(to, "clearance_deg"))
			cfg.tool.clearance_deg = as_double(*v, "tool.clearance_deg", line, col);
		if (const json_value *v = optional_key(to, "length"))
			cfg.tool.length = as_double(*v, "tool.length", line, col);
		if (const json_value *v = optional_key(to, "height"))
			cfg.tool.height = as_double(*v, "tool.height", line, col);
		if (const json_value *v = optional_key(to, "tl_y"))
			cfg.tool.tl_y = as_double(*v, "tool.tl_y", line, col);
		if (const json_value *v = optional_key(to, "fillet_radius"))
			cfg.tool.fillet_radius = as_double(*v, "tool.fillet_radius", line, col);
		if (const json_value *v = optional_key(to, "mu_friction"))
			cfg.tool.mu_friction = as_double(*v, "tool.mu_friction", line, col);
		if (const json_value *v = optional_key(to, "target_feed"))
			cfg.tool.target_feed = as_double(*v, "tool.target_feed", line, col);
		if (const json_value *v = optional_key(to, "tool_right_clearance"))
			cfg.tool.tool_right_clearance = as_double(*v, "tool.tool_right_clearance", line, col);
		if (const json_value *v = optional_key(to, "tool_x_shift"))
			cfg.tool.tool_x_shift = as_double(*v, "tool.tool_x_shift", line, col);
	};

	auto parse_numerical = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("numerical must be an object", line, col, "numerical");
		const auto &no = j.as_object();
		if (const json_value *v = optional_key(no, "hdx"))
			cfg.numerical.hdx = as_double(*v, "numerical.hdx", line, col);
		if (const json_value *v = optional_key(no, "alpha"))
			cfg.numerical.alpha = as_double(*v, "numerical.alpha", line, col);
		if (const json_value *v = optional_key(no, "beta"))
			cfg.numerical.beta = as_double(*v, "numerical.beta", line, col);
		if (const json_value *v = optional_key(no, "eta"))
			cfg.numerical.eta = as_double(*v, "numerical.eta", line, col);
		if (const json_value *v = optional_key(no, "xsph_eps"))
			cfg.numerical.xsph_eps = as_double(*v, "numerical.xsph_eps", line, col);
		if (const json_value *v = optional_key(no, "art_stress_eps"))
			cfg.numerical.art_stress_eps = as_double(*v, "numerical.art_stress_eps", line, col);
		if (const json_value *v = optional_key(no, "stress_exponent"))
			cfg.numerical.stress_exponent = as_double(*v, "numerical.stress_exponent", line, col);
	};

	auto parse_thermal = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("thermal must be an object", line, col, "thermal");
		const auto &to = j.as_object();
		if (const json_value *v = optional_key(to, "enabled"))
			cfg.thermal.enabled = as_bool(*v, "thermal.enabled", line, col);
		if (const json_value *v = optional_key(to, "method"))
			cfg.thermal.method = as_string(*v, "thermal.method", line, col);
		if (const json_value *v = optional_key(to, "T0"))
			cfg.thermal.T0 = as_double(*v, "thermal.T0", line, col);
	};

	auto parse_plasticity = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("plasticity must be an object", line, col, "plasticity");
		const auto &po = j.as_object();
		if (const json_value *v = optional_key(po, "enabled"))
			cfg.plasticity.enabled = as_bool(*v, "plasticity.enabled", line, col);
		if (const json_value *v = optional_key(po, "model"))
			cfg.plasticity.model = as_string(*v, "plasticity.model", line, col);
		if (const json_value *v = optional_key(po, "tolerance"))
			cfg.plasticity.tolerance = as_double(*v, "plasticity.tolerance", line, col);
		if (const json_value *v = optional_key(po, "dissipation_considered"))
			cfg.plasticity.dissipation_considered = as_bool(*v, "plasticity.dissipation_considered", line, col);
	};

	auto parse_material = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("material must be an object", line, col, "material");
		const auto &mo = j.as_object();
		if (const json_value *v = optional_key(mo, "physical_constants"))
			cfg.material.physical_constants = as_string(*v, "material.physical_constants", line, col);
	};

	auto parse_multires = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("multiresolution must be an object", line, col, "multiresolution");
		const auto &mo = j.as_object();
		if (const json_value *v = optional_key(mo, "resol_ratio"))
			cfg.multires.resol_ratio = as_double(*v, "multiresolution.resol_ratio", line, col);
		if (const json_value *v = optional_key(mo, "py_split_fraction"))
			cfg.multires.py_split_fraction = as_double(*v, "multiresolution.py_split_fraction", line, col);
		if (const json_value *v = optional_key(mo, "x_high_res_limit"))
			cfg.multires.x_high_res_limit = as_double(*v, "multiresolution.x_high_res_limit", line, col);
		if (const json_value *v = optional_key(mo, "py_margin_factor"))
			cfg.multires.py_margin_factor = as_double(*v, "multiresolution.py_margin_factor", line, col);
		if (const json_value *v = optional_key(mo, "py_margin_factor_dynamic"))
			cfg.multires.py_margin_factor_dynamic = as_double(*v, "multiresolution.py_margin_factor_dynamic", line, col);
	};

	auto parse_adapt = [&](const json_value &j) {
		if (!j.is_object())
			throw json_error("adaptivity must be an object", line, col, "adaptivity");
		const auto &ao = j.as_object();
		if (const json_value *v = optional_key(ao, "enabled"))
			cfg.adaptivity.enabled = as_bool(*v, "adaptivity.enabled", line, col);
		if (const json_value *v = optional_key(ao, "criterion"))
			cfg.adaptivity.criterion = as_string(*v, "adaptivity.criterion", line, col);
		if (const json_value *v = optional_key(ao, "pattern"))
			cfg.adaptivity.pattern = as_string(*v, "adaptivity.pattern", line, col);
		if (const json_value *v = optional_key(ao, "alpha_dx"))
			cfg.adaptivity.alpha_dx = as_double(*v, "adaptivity.alpha_dx", line, col);
		if (const json_value *v = optional_key(ao, "beta_h"))
			cfg.adaptivity.beta_h = as_double(*v, "adaptivity.beta_h", line, col);
		if (const json_value *v = optional_key(ao, "v_cr"))
			cfg.adaptivity.v_cr = as_double(*v, "adaptivity.v_cr", line, col);
		if (const json_value *v = optional_key(ao, "div_v_cr"))
			cfg.adaptivity.div_v_cr = as_double(*v, "adaptivity.div_v_cr", line, col);
		if (const json_value *v = optional_key(ao, "SvM_cr"))
			cfg.adaptivity.SvM_cr = as_double(*v, "adaptivity.SvM_cr", line, col);
		if (const json_value *v = optional_key(ao, "eps_cr"))
			cfg.adaptivity.eps_cr = as_double(*v, "adaptivity.eps_cr", line, col);
		if (const json_value *v = optional_key(ao, "T_cr"))
			cfg.adaptivity.T_cr = as_double(*v, "adaptivity.T_cr", line, col);
		if (const json_value *v = optional_key(ao, "frame_width"))
			cfg.adaptivity.frame_width = as_double(*v, "adaptivity.frame_width", line, col);
		if (const json_value *v = optional_key(ao, "frame_height"))
			cfg.adaptivity.frame_height = as_double(*v, "adaptivity.frame_height", line, col);
		if (const json_value *v = optional_key(ao, "xy_min_x"))
			cfg.adaptivity.xy_min_x = as_double(*v, "adaptivity.xy_min_x", line, col);
		if (const json_value *v = optional_key(ao, "xy_min_y"))
			cfg.adaptivity.xy_min_y = as_double(*v, "adaptivity.xy_min_y", line, col);
		if (const json_value *v = optional_key(ao, "xy_max_x"))
			cfg.adaptivity.xy_max_x = as_double(*v, "adaptivity.xy_max_x", line, col);
		if (const json_value *v = optional_key(ao, "xy_max_y"))
			cfg.adaptivity.xy_max_y = as_double(*v, "adaptivity.xy_max_y", line, col);
		if (const json_value *v = optional_key(ao, "n_nbh"))
			cfg.adaptivity.n_nbh = as_int(*v, "adaptivity.n_nbh", line, col);
		if (const json_value *v = optional_key(ao, "l_eff_extra_fraction"))
			cfg.adaptivity.l_eff_extra_fraction = as_double(*v, "adaptivity.l_eff_extra_fraction", line, col);
		if (const json_value *v = optional_key(ao, "allow_refine"))
			cfg.adaptivity.allow_refine = as_bool(*v, "adaptivity.allow_refine", line, col);
	};

	if (const json_value *v = optional_key(o, "io"))
		parse_io(*v);
	if (const json_value *v = optional_key(o, "model"))
		parse_model(*v);
	if (const json_value *v = optional_key(o, "workpiece"))
		parse_workpiece(*v);
	if (const json_value *v = optional_key(o, "time"))
		parse_time(*v);
	if (const json_value *v = optional_key(o, "tool"))
		parse_tool(*v);
	if (const json_value *v = optional_key(o, "numerical"))
		parse_numerical(*v);
	if (const json_value *v = optional_key(o, "thermal"))
		parse_thermal(*v);
	if (const json_value *v = optional_key(o, "plasticity"))
		parse_plasticity(*v);
	if (const json_value *v = optional_key(o, "material"))
		parse_material(*v);
	if (const json_value *v = optional_key(o, "multiresolution"))
		parse_multires(*v);
	if (const json_value *v = optional_key(o, "adaptivity"))
		parse_adapt(*v);

	validate_simulation_config_or_throw(cfg);
	return cfg;
}

void validate_simulation_config_or_throw(const simulation_config &cfg) {
	if (cfg.schema_version != 1) {
		throw std::runtime_error("Unsupported schema_version: " + std::to_string(cfg.schema_version));
	}

	validate_choice_or_throw(cfg.model.type, {"single_resolution", "apriori_refinement", "dynamic_refinement"}, "model.type");
	validate_choice_or_throw(cfg.material.physical_constants, {"tial6v4_sima_tanh2010_si"}, "material.physical_constants");
	validate_choice_or_throw(cfg.plasticity.model, {"johnson_cook_sima_2010"}, "plasticity.model");
	validate_choice_or_throw(cfg.thermal.method, {"thermal_pse", "thermal_brookshaw"}, "thermal.method");
	validate_choice_or_throw(cfg.adaptivity.criterion, {"moving_frame"}, "adaptivity.criterion");
	validate_choice_or_throw(cfg.adaptivity.pattern, {"cubic_basic"}, "adaptivity.pattern");

	if (!(cfg.io.num_print >= 1))
		throw std::runtime_error("io.num_print must be >= 1");
	if (!(cfg.model.nbox >= 3))
		throw std::runtime_error("model.nbox must be >= 3");

	if (!(cfg.workpiece.hi_x > cfg.workpiece.lo_x))
		throw std::runtime_error("workpiece.hi_x must be > workpiece.lo_x");
	if (!(cfg.workpiece.hi_y > cfg.workpiece.lo_y))
		throw std::runtime_error("workpiece.hi_y must be > workpiece.lo_y");
	if (!(cfg.workpiece.base_height_y > 0.0))
		throw std::runtime_error("workpiece.base_height_y must be > 0");

	if (!(cfg.tool.length > 0.0))
		throw std::runtime_error("tool.length must be > 0");
	if (!(cfg.tool.height > 0.0))
		throw std::runtime_error("tool.height must be > 0");
	if (!(cfg.tool.fillet_radius >= 0.0))
		throw std::runtime_error("tool.fillet_radius must be >= 0");
	if (!(cfg.tool.mu_friction >= 0.0))
		throw std::runtime_error("tool.mu_friction must be >= 0");
	if (!(cfg.tool.cutting_speed > 0.0))
		throw std::runtime_error("tool.cutting_speed must be > 0");
	if (!(cfg.tool.target_feed > 0.0))
		throw std::runtime_error("tool.target_feed must be > 0");

	if (!(cfg.numerical.hdx > 0.0))
		throw std::runtime_error("numerical.hdx must be > 0");
	if (!(cfg.numerical.eta >= 0.0))
		throw std::runtime_error("numerical.eta must be >= 0");
	if (!(cfg.numerical.xsph_eps >= 0.0 && cfg.numerical.xsph_eps <= 1.0))
		throw std::runtime_error("numerical.xsph_eps must be in [0,1]");

	if (!(cfg.time.cut_length > 0.0))
		throw std::runtime_error("time.cut_length must be > 0");
	if (!(cfg.time.dt_empirical > 0.0))
		throw std::runtime_error("time.dt_empirical must be > 0");
	if (!(cfg.time.mech_cfl_factor > 0.0))
		throw std::runtime_error("time.mech_cfl_factor must be > 0");
	if (!(cfg.time.heat_cfl_factor > 0.0))
		throw std::runtime_error("time.heat_cfl_factor must be > 0");

	if (cfg.time.dt_override && !(*cfg.time.dt_override > 0.0))
		throw std::runtime_error("time.dt must be > 0 when provided");
	if (cfg.time.t_final_override && !(*cfg.time.t_final_override > 0.0))
		throw std::runtime_error("time.t_final must be > 0 when provided");

	if (!(cfg.multires.resol_ratio >= 1.0))
		throw std::runtime_error("multiresolution.resol_ratio must be >= 1");
	if (!(cfg.multires.py_split_fraction >= 0.0 && cfg.multires.py_split_fraction <= 1.0))
		throw std::runtime_error("multiresolution.py_split_fraction must be in [0,1]");
	if (!(cfg.multires.py_margin_factor >= 0.0))
		throw std::runtime_error("multiresolution.py_margin_factor must be >= 0");
	if (!(cfg.multires.py_margin_factor_dynamic >= 0.0))
		throw std::runtime_error("multiresolution.py_margin_factor_dynamic must be >= 0");

	if (cfg.model.type == "dynamic_refinement" && !cfg.adaptivity.enabled) {
		throw std::runtime_error("adaptivity.enabled must be true for model.type=dynamic_refinement");
	}
}

static json_value::object dump_obj(std::initializer_list<std::pair<std::string, json_value>> items) {
	json_value::object o;
	for (const auto &it : items)
		o.emplace(it.first, it.second);
	return o;
}

json_value dump_default_simulation_config_json() {
	simulation_config cfg;
	json_value root(json_value::object{});
	auto &o = root.as_object();

	o.emplace("schema_version", (double)cfg.schema_version);
	o.emplace("name", "default_model_1_like");

	o.emplace("io", dump_obj({
						{"output_dir", cfg.io.output_dir},
						{"clear_output_dir", cfg.io.clear_output_dir},
						{"num_print", (double)cfg.io.num_print},
						{"all_steps", cfg.io.all_steps},
					}));

	o.emplace("model", dump_obj({
						   {"type", cfg.model.type},
						   {"nbox", (double)cfg.model.nbox},
					   }));

	o.emplace("workpiece", dump_obj({
							   {"lo_x", cfg.workpiece.lo_x},
							   {"hi_x", cfg.workpiece.hi_x},
							   {"lo_y", cfg.workpiece.lo_y},
							   {"hi_y", cfg.workpiece.hi_y},
							   {"base_height_y", cfg.workpiece.base_height_y},
							   {"keep_base_spacing", cfg.workpiece.keep_base_spacing},
						   }));

	o.emplace("time", dump_obj({
						  {"cut_length", cfg.time.cut_length},
						  {"dt_empirical", cfg.time.dt_empirical},
						  {"mech_cfl_factor", cfg.time.mech_cfl_factor},
						  {"heat_cfl_factor", cfg.time.heat_cfl_factor},
					  }));

	o.emplace("tool", dump_obj({
						  {"cutting_speed", cfg.tool.cutting_speed},
						  {"rake_deg", cfg.tool.rake_deg},
						  {"clearance_deg", cfg.tool.clearance_deg},
						  {"length", cfg.tool.length},
						  {"height", cfg.tool.height},
						  {"tl_y", cfg.tool.tl_y},
						  {"fillet_radius", cfg.tool.fillet_radius},
						  {"mu_friction", cfg.tool.mu_friction},
						  {"target_feed", cfg.tool.target_feed},
						  {"tool_right_clearance", cfg.tool.tool_right_clearance},
						  {"tool_x_shift", cfg.tool.tool_x_shift},
					  }));

	o.emplace("numerical", dump_obj({
							   {"hdx", cfg.numerical.hdx},
							   {"alpha", cfg.numerical.alpha},
							   {"beta", cfg.numerical.beta},
							   {"eta", cfg.numerical.eta},
							   {"xsph_eps", cfg.numerical.xsph_eps},
							   {"art_stress_eps", cfg.numerical.art_stress_eps},
							   {"stress_exponent", cfg.numerical.stress_exponent},
						   }));

	o.emplace("thermal", dump_obj({
							 {"enabled", cfg.thermal.enabled},
							 {"method", cfg.thermal.method},
							 {"T0", cfg.thermal.T0},
						 }));

	o.emplace("plasticity", dump_obj({
								{"enabled", cfg.plasticity.enabled},
								{"model", cfg.plasticity.model},
								{"tolerance", cfg.plasticity.tolerance},
								{"dissipation_considered", cfg.plasticity.dissipation_considered},
							}));

	o.emplace("material", dump_obj({
							  {"physical_constants", cfg.material.physical_constants},
						  }));

	o.emplace("multiresolution", dump_obj({
									 {"resol_ratio", cfg.multires.resol_ratio},
									 {"py_split_fraction", cfg.multires.py_split_fraction},
									 {"x_high_res_limit", cfg.multires.x_high_res_limit},
									 {"py_margin_factor", cfg.multires.py_margin_factor},
									 {"py_margin_factor_dynamic", cfg.multires.py_margin_factor_dynamic},
								 }));

	o.emplace("adaptivity", dump_obj({
								{"enabled", cfg.adaptivity.enabled},
								{"criterion", cfg.adaptivity.criterion},
								{"pattern", cfg.adaptivity.pattern},
								{"alpha_dx", cfg.adaptivity.alpha_dx},
								{"beta_h", cfg.adaptivity.beta_h},
								{"v_cr", cfg.adaptivity.v_cr},
								{"div_v_cr", cfg.adaptivity.div_v_cr},
								{"SvM_cr", cfg.adaptivity.SvM_cr},
								{"eps_cr", cfg.adaptivity.eps_cr},
								{"T_cr", cfg.adaptivity.T_cr},
								{"frame_width", cfg.adaptivity.frame_width},
								{"frame_height", cfg.adaptivity.frame_height},
								{"xy_min_x", cfg.adaptivity.xy_min_x},
								{"xy_min_y", cfg.adaptivity.xy_min_y},
								{"xy_max_x", cfg.adaptivity.xy_max_x},
								{"xy_max_y", cfg.adaptivity.xy_max_y},
								{"n_nbh", (double)cfg.adaptivity.n_nbh},
								{"l_eff_extra_fraction", cfg.adaptivity.l_eff_extra_fraction},
								{"allow_refine", cfg.adaptivity.allow_refine},
							}));

	return root;
}

} // namespace mfree::config
