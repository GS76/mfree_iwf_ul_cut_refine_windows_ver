#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>

struct TableProperty {
    enum class Mode {
        Constant,
        Table,
        Linear
    };

    std::vector<double> temps;
    std::vector<double> values;
    double default_value = 0.0;
    bool active = false;
    double m = 0.0;
    double b = 0.0;
    bool clamp_min_active = false;
    bool clamp_max_active = false;
    double clamp_min_value = 0.0;
    double clamp_max_value = 0.0;
    Mode mode = Mode::Constant;

    TableProperty() : default_value(0.0), active(false), mode(Mode::Constant) {}
    TableProperty(double val) : default_value(val), active(false), mode(Mode::Constant) {}

    void set(double val) { 
        default_value = val; 
        active = false; 
        temps.clear();
        values.clear();
        clamp_min_active = false;
        clamp_max_active = false;
        clamp_min_value = 0.0;
        clamp_max_value = 0.0;
        mode = Mode::Constant;
    }

    void set_table(const std::vector<double>& t, const std::vector<double>& v) {
        if (t.size() != v.size() || t.empty()) {
            std::cerr << "Error: Invalid table data for property." << std::endl;
            return;
        }
        temps = t; 
        values = v; 
        active = true;
        clamp_min_active = false;
        clamp_max_active = false;
        clamp_min_value = 0.0;
        clamp_max_value = 0.0;
        mode = Mode::Table;
    }

    void set_linear(double slope_m, double intercept_b) {
        m = slope_m;
        b = intercept_b;
        temps.clear();
        values.clear();
        active = false;
        clamp_min_active = false;
        clamp_max_active = false;
        clamp_min_value = 0.0;
        clamp_max_value = 0.0;
        mode = Mode::Linear;
    }

    void set_linear(double slope_m, double intercept_b, bool use_min, double min_val, bool use_max, double max_val) {
        set_linear(slope_m, intercept_b);
        clamp_min_active = use_min;
        clamp_max_active = use_max;
        clamp_min_value = min_val;
        clamp_max_value = max_val;
    }

    void set_clamp_min(double min_val) {
        clamp_min_active = true;
        clamp_min_value = min_val;
    }

    void set_clamp_max(double max_val) {
        clamp_max_active = true;
        clamp_max_value = max_val;
    }

    void clear_clamps() {
        clamp_min_active = false;
        clamp_max_active = false;
        clamp_min_value = 0.0;
        clamp_max_value = 0.0;
    }

    double get(double T) const {
        if (mode == Mode::Constant) return default_value;

        if (mode == Mode::Table) {
            if (!active) return default_value;
        
            if (T <= temps.front()) return values.front();
            if (T >= temps.back()) return values.back();

            auto it = std::lower_bound(temps.begin(), temps.end(), T);
            if (it == temps.begin()) return values.front();

            size_t idx = std::distance(temps.begin(), it);
            double T1 = temps[idx - 1];
            double T2 = temps[idx];
            double V1 = values[idx - 1];
            double V2 = values[idx];

            return V1 + (T - T1) * (V2 - V1) / (T2 - T1);
        }

        double y = m * T + b;
        if (clamp_min_active && y < clamp_min_value) y = clamp_min_value;
        if (clamp_max_active && y > clamp_max_value) y = clamp_max_value;
        return y;
    }
};

#endif /* PROPERTY_H_ */
