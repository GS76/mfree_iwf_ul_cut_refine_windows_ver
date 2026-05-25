#ifndef ENV_TABLE_H_
#define ENV_TABLE_H_

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <utility>
#include <vector>

namespace env_table {
inline bool parse_string(const char *s, std::vector<double> &Tout, std::vector<double> &Vout) {
	if (!s || s[0] == '\0')
		return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p && (std::isspace(static_cast<unsigned char>(*p)) || *p == ',' || *p == ';'))
			++p;
		if (!*p)
			break;

		char *end = nullptr;
		double T = std::strtod(p, &end);
		if (end == p || !std::isfinite(T))
			return false;
		p = end;
		while (*p && std::isspace(static_cast<unsigned char>(*p)))
			++p;
		if (*p != ':' && *p != '=')
			return false;
		++p;
		while (*p && std::isspace(static_cast<unsigned char>(*p)))
			++p;

		end = nullptr;
		double v = std::strtod(p, &end);
		if (end == p || !std::isfinite(v))
			return false;
		p = end;
		pairs.push_back({T, v});
	}

	if (pairs.size() < 2)
		return false;

	std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
	Tout.clear();
	Vout.clear();
	for (const auto &kv : pairs) {
		if (!Tout.empty() && kv.first == Tout.back()) {
			Vout.back() = kv.second;
			continue;
		}
		Tout.push_back(kv.first);
		Vout.push_back(kv.second);
	}
	return Tout.size() >= 2;
}

inline bool parse_from_env(const char *key, std::vector<double> &Tout, std::vector<double> &Vout) {
	return parse_string(std::getenv(key), Tout, Vout);
}

inline bool parse_from_env_any(std::initializer_list<const char *> keys, std::vector<double> &Tout, std::vector<double> &Vout) {
	for (const char *k : keys) {
		if (parse_from_env(k, Tout, Vout))
			return true;
	}
	return false;
}

inline double eval_linear_clamped(double T, const std::vector<double> &T_tab, const std::vector<double> &v_tab, double fallback) {
	if (T_tab.size() < 2 || T_tab.size() != v_tab.size() || !std::isfinite(T))
		return fallback;
	if (T <= T_tab.front())
		return v_tab.front();
	if (T >= T_tab.back())
		return v_tab.back();
	auto it = std::upper_bound(T_tab.begin(), T_tab.end(), T);
	std::size_t i1 = static_cast<std::size_t>(it - T_tab.begin());
	if (i1 == 0 || i1 >= T_tab.size())
		return fallback;
	std::size_t i0 = i1 - 1;
	double T0 = T_tab[i0];
	double T1 = T_tab[i1];
	double v0 = v_tab[i0];
	double v1 = v_tab[i1];
	double dT = T1 - T0;
	if (!(dT > 0.))
		return fallback;
	double a = (T - T0) / dT;
	return (1.0 - a) * v0 + a * v1;
}
} // namespace env_table

#endif // ENV_TABLE_H_
