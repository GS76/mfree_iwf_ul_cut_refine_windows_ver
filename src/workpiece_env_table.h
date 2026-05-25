#ifndef WORKPIECE_ENV_TABLE_H_
#define WORKPIECE_ENV_TABLE_H_

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <utility>
#include <vector>

namespace workpiece_env_table {

struct table {
	std::vector<double> T;
	std::vector<double> v;
	bool enabled = false;
};

inline bool parse_pairs(const char *s, table &out) {
	if (!s || s[0] == '\0')
		return false;

	std::vector<std::pair<double, double>> pairs;
	const char *p = s;
	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r')
			++p;
		if (!*p)
			break;

		char *end = nullptr;
		double T = std::strtod(p, &end);
		if (end == p || !std::isfinite(T))
			return false;
		p = end;

		while (*p == ' ' || *p == '\t')
			++p;
		if (*p != ':' && *p != '=')
			return false;
		++p;
		while (*p == ' ' || *p == '\t')
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
	out.T.clear();
	out.v.clear();
	for (const auto &kv : pairs) {
		if (!out.T.empty() && kv.first == out.T.back()) {
			out.v.back() = kv.second;
			continue;
		}
		out.T.push_back(kv.first);
		out.v.push_back(kv.second);
	}
	out.enabled = out.T.size() >= 2;
	return out.enabled;
}

inline const char *pick_env(const char *primary, const char *alias) {
	const char *s = std::getenv(primary);
	if (s && s[0] != '\0')
		return s;
	s = std::getenv(alias);
	if (s && s[0] != '\0')
		return s;
	return nullptr;
}

inline double parse_const_or(const char *primary, const char *alias, double fallback) {
	const char *s = pick_env(primary, alias);
	if (!s)
		return fallback;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	if (end == s || !std::isfinite(v))
		return fallback;
	while (*end && std::isspace(static_cast<unsigned char>(*end)))
		++end;
	if (*end != '\0')
		return fallback;
	return v;
}

inline table load_table(const char *primary, const char *alias) {
	table t;
	const char *s = pick_env(primary, alias);
	if (!s)
		return t;
	parse_pairs(s, t);
	return t;
}

inline double eval(double T, const table &t, double fallback) {
	if (!t.enabled || t.T.size() != t.v.size() || t.T.size() < 2 || !std::isfinite(T))
		return fallback;
	if (T <= t.T.front())
		return t.v.front();
	if (T >= t.T.back())
		return t.v.back();
	auto it = std::upper_bound(t.T.begin(), t.T.end(), T);
	std::size_t i1 = static_cast<std::size_t>(it - t.T.begin());
	if (i1 == 0 || i1 >= t.T.size())
		return fallback;
	std::size_t i0 = i1 - 1;
	double T0 = t.T[i0];
	double T1 = t.T[i1];
	double v0 = t.v[i0];
	double v1 = t.v[i1];
	double dT = T1 - T0;
	if (!(dT > 0.))
		return fallback;
	double a = (T - T0) / dT;
	return (1.0 - a) * v0 + a * v1;
}

inline double nu_at(double T, double fallback) {
	static const table nu_table = load_table("MFREE_WP_NU_TABLE", "MFREE_WORKPIECE_NU_TABLE");
	static const double nu_const = parse_const_or("MFREE_WP_NU", "MFREE_WORKPIECE_NU", fallback);
	return eval(T, nu_table, nu_const);
}

inline double alpha_at(double T, double fallback) {
	static const table alpha_table = load_table("MFREE_WP_ALPHA_TABLE", "MFREE_WORKPIECE_ALPHA_TABLE");
	static const double alpha_const = parse_const_or("MFREE_WP_ALPHA", "MFREE_WORKPIECE_ALPHA", fallback);
	return eval(T, alpha_table, alpha_const);
}

} // namespace workpiece_env_table

#endif
