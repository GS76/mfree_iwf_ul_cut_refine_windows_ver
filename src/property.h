#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>

struct TableProperty {
    std::vector<double> temps;
    std::vector<double> values;
    double default_value = 0.0;
    bool active = false;

    TableProperty() : default_value(0.0), active(false) {}
    TableProperty(double val) : default_value(val), active(false) {}

    void set(double val) { 
        default_value = val; 
        active = false; 
        temps.clear();
        values.clear();
    }

    void set_table(const std::vector<double>& t, const std::vector<double>& v) {
        if (t.size() != v.size() || t.empty()) {
            std::cerr << "Error: Invalid table data for property." << std::endl;
            return;
        }
        temps = t; 
        values = v; 
        active = true;
    }

    double get(double T) const {
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
};

#endif /* PROPERTY_H_ */
