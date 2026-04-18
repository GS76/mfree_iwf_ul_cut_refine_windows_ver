#ifndef TOOL_IFACE_H_
#define TOOL_IFACE_H_

#include <glm/glm.hpp>

struct tool_contact_hit_2d {
	bool inside = false;
	glm::dvec2 x_contact = glm::dvec2(0.);
	glm::dvec2 normal = glm::dvec2(0.);
};

class tool_contact_2d {
public:
	virtual ~tool_contact_2d() = default;
	virtual bool contact(glm::dvec2 x_slave, tool_contact_hit_2d &out) const = 0;
	virtual glm::dvec2 velocity_world() const = 0;
	virtual double mu() const = 0;
};

#endif
