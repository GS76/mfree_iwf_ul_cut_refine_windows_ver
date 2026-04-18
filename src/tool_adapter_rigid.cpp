#include "tool_adapter_rigid.h"

#include "tool.h"

bool rigid_tool_contact_adapter::contact(glm::dvec2 x_slave, tool_contact_hit_2d &out) const {
	if (!m_tool) {
		out.inside = false;
		out.x_contact = glm::dvec2(0.);
		out.normal = glm::dvec2(0.);
		return false;
	}
	glm::dvec2 x_contact(0.);
	glm::dvec2 n(0.);
	bool inside = m_tool->contact(x_slave, x_contact, n);
	out.inside = inside;
	out.x_contact = x_contact;
	out.normal = n;
	return inside;
}

glm::dvec2 rigid_tool_contact_adapter::velocity_world() const {
	if (!m_tool) return glm::dvec2(0.);
	return m_tool->get_vel();
}

double rigid_tool_contact_adapter::mu() const {
	if (!m_tool) return 0.;
	return m_tool->mu();
}
