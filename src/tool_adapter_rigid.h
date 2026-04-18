#ifndef TOOL_ADAPTER_RIGID_H_
#define TOOL_ADAPTER_RIGID_H_

#include "tool_iface.h"

class tool;

class rigid_tool_contact_adapter final : public tool_contact_2d {
public:
	explicit rigid_tool_contact_adapter(const tool *t) : m_tool(t) {}

	bool contact(glm::dvec2 x_slave, tool_contact_hit_2d &out) const override;
	glm::dvec2 velocity_world() const override;
	double mu() const override;

private:
	const tool *m_tool = nullptr;
};

#endif
