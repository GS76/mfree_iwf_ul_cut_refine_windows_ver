#ifndef TOOL_ADAPTER_POLY_H_
#define TOOL_ADAPTER_POLY_H_

#include "tool_iface.h"

#include <vector>

class poly_tool_contact_adapter final : public tool_contact_2d {
  public:
	poly_tool_contact_adapter(const std::vector<glm::dvec2> &poly, double mu, glm::dvec2 vel);

	bool contact(glm::dvec2 x_slave, tool_contact_hit_2d &out) const override;
	glm::dvec2 velocity_world() const override;
	double mu() const override;

  private:
	const std::vector<glm::dvec2> *m_poly = nullptr;
	double m_mu = 0.;
	glm::dvec2 m_vel = glm::dvec2(0.);
	glm::dvec2 m_bbox_min = glm::dvec2(0.);
	glm::dvec2 m_bbox_max = glm::dvec2(0.);
};

#endif
