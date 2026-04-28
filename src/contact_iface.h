#ifndef CONTACT_IFACE_H_
#define CONTACT_IFACE_H_

class body;
class fe_tool;
class tool_contact_2d;

void contact_apply_master_to_body_2d(const tool_contact_2d &master, body &slave, fe_tool *thermal_master, double accounting_dt);

#endif
