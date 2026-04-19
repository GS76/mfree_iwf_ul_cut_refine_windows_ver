#include "geom_validation_math.h"

#include <cmath>
#include <limits>
#include <vector>

static int require_true(bool v) { return v ? 0 : 1; }

int main() {
	int fails = 0;
	{
		std::vector<glm::dvec2> p;
		double a = geom_validation_math::polygon_signed_area(p);
		fails += require_true(std::isnan(a));
	}
	{
		std::vector<glm::dvec2> p = {{0, 0}, {1, 0}};
		double a = geom_validation_math::polygon_signed_area(p);
		fails += require_true(std::isnan(a));
	}
	{
		std::vector<glm::dvec2> p = {{0, 0}, {1, 0}, {0, 1}};
		double a = geom_validation_math::polygon_signed_area(p);
		fails += require_true(std::isfinite(a) && std::abs(a - 0.5) < 1e-12);
	}
	{
		std::vector<glm::dvec2> p = {{0, 0}, {1, 1}, {2, 2}};
		double a = geom_validation_math::polygon_signed_area(p);
		fails += require_true(std::isnan(a));
	}
	{
		double nan = std::numeric_limits<double>::quiet_NaN();
		std::vector<glm::dvec2> p = {{0, 0}, {1, 0}, {nan, 1}};
		double a = geom_validation_math::polygon_signed_area(p);
		fails += require_true(std::isnan(a));
	}
	{
		double big = 1e200;
		std::vector<glm::dvec2> p = {{0, 0}, {big, 0}, {0, big}};
		double a = geom_validation_math::polygon_signed_area(p);
		fails += require_true(std::isnan(a));
	}
	{
		std::vector<glm::dvec2> poly = {{0, 0}, {0, 0}, {1, 0}, {1, 1}};
		std::size_t ei = 0;
		double et = 0.0;
		glm::dvec2 cp = geom_validation_math::polygon_closest_point(glm::dvec2(0.2, 0.3), poly, &ei, &et);
		fails += require_true(std::isfinite(cp.x) && std::isfinite(cp.y));
		fails += require_true(ei < poly.size());
		fails += require_true(et >= 0.0 && et <= 1.0);
	}
	return fails == 0 ? 0 : 1;
}

