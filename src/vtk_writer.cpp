/*
 * ================================================
 * 					COPYRIGHT:
 * Institute of Machine Tools & Manufacturing (IWF)
 * Department of Mechanical & Process Engineering
 * 					ETH ZURICH
 * ================================================
 *
 *  This file is part of "mfree_iwf-ul-cut-refine".
 *
 * 	mfree_iwf is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	mfree_iwf-ul-cut-refine is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *  along with mfree_iwf-ul-cut-refine.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This is the source code used to produce the results
 *  of the metal cutting simulation presented in:
 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  "Meshfree Simulation of Metal Cutting:
 *  An Updated Lagrangian Approach with Dynamic Refinement"
 *
 * 	Authored by:
 * 	Mohamadreza Afrasiabi
 * 	Dr. Matthias Roethlin
 * 	Hagen Klippel
 * 	Prof. Dr. Konrad Wegener
 *
 * 	Published by:
 * 	International Journal of Mechanical Sciences
 * 	28 June 2019
 * 	https://doi.org/10.1016/j.ijmecsci.2019.06.045
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * 	For further descriptions, you may refer to the manuscript
 * 	or the previous works of the same research group
 * 	at IWF, ETH Zurich.
 *
 */

#include "vtk_writer.h"

#include "fe_tool.h"

#include <cmath>
#include <cstdio>

void vtk_writer_write(const std::vector<particle> &particles, unsigned int step, const char *folder) {
	char buf[1024];
	std::snprintf(buf, sizeof(buf), "%s/out_%06d.vtk", folder, step);
	FILE *fp = fopen(buf, "w+");
	if (!fp) return;

	unsigned int np = particles.size();

	fprintf(fp, "# vtk DataFile Version 2.0\n");
	fprintf(fp, "mfree iwf\n");
	fprintf(fp, "ASCII\n");
	fprintf(fp, "\n");

	fprintf(fp, "DATASET UNSTRUCTURED_GRID\n");		// Particle positions
	fprintf(fp, "POINTS %d float\n", np);
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%e %e %e\n", particles[i].x, particles[i].y, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "CELLS %d %d\n", np, 2*np);
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%d %d\n", 1, i);
	}
	fprintf(fp, "\n");

	fprintf(fp, "CELL_TYPES %d\n", np);
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%d\n", 1);
	}
	fprintf(fp, "\n");

	fprintf(fp, "POINT_DATA %d\n", np);

	fprintf(fp, "SCALARS density float 1\n");		// Current particle density
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f\n", particles[i].rho);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS temperature float 1\n");    // Particle temperature
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f\n", particles[i].T);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS Svm float 1\n");        // Particle Von Mises stress
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		double sxx = particles[i].Sxx - particles[i].p;
		double sxy = particles[i].Sxy;
		double syy = particles[i].Syy - particles[i].p;
		double szz = particles[i].Szz - particles[i].p;

		double svm = sqrt(fabs((sxx*sxx + syy*syy + szz*szz) - sxx * syy - sxx * szz - syy * szz + 3.0 * (sxy*sxy)));
		fprintf(fp, "%f\n", svm);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS equiv_plastic_strain float 1\n");		// Current particle's equivalent plastic strain
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f\n", particles[i].eps_pl_equiv);
	}
	fprintf(fp, "\n");

	fprintf(fp, "VECTORS velocity float\n");		// Particle velocities
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f %f %f\n", particles[i].vx, particles[i].vy, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "VECTORS contact_force_n float\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%e %e %e\n", particles[i].fcx, particles[i].fcy, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "VECTORS contact_force_t float\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%e %e %e\n", particles[i].ftx, particles[i].fty, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS contact_pressure float 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		double Fn = std::sqrt(particles[i].fcx * particles[i].fcx + particles[i].fcy * particles[i].fcy);
		double p = 0.0;
		if (Fn > 0.0 && particles[i].m > 0.0 && particles[i].rho > 0.0) {
			p = Fn * particles[i].rho / particles[i].m;
		}
		fprintf(fp, "%e\n", p);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS displacement float 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		double dx = particles[i].x - particles[i].X;
		double dy = particles[i].y - particles[i].Y;
		double u = std::sqrt(dx * dx + dy * dy);
		fprintf(fp, "%e\n", u);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS glob_density_err double 1\n");  // global density error acc. to Feldman 2006
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%e\n", (particles[i].rho - particles[i].rho_init)*(particles[i].rho - particles[i].rho_init));
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS mass double 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%e\n", particles[i].m);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS fixed int 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%d\n", particles[i].fixed ? 1 : 0);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS num_neighbors int 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%u\n", particles[i].num_nbh);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS refine_step int 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%u\n", particles[i].refine_step);
	}
	fprintf(fp, "\n");

	fclose(fp);
}

void vtk_writer_write(const fe_tool* tool, unsigned int step, const char *folder) {
	vtk_writer_write(tool, step, folder, "fe_tool");
}

void vtk_writer_write(const fe_tool* tool, unsigned int step, const char *folder, const char *filename_prefix) {
	if (!tool) return;
	const auto &nodes_tool = tool->nodes_tool_frame();
	const auto &tris = tool->triangles();
	if (nodes_tool.empty() || tris.empty()) return;

	char buf[1024];
	const char *effective_prefix = filename_prefix;
	if (!effective_prefix || effective_prefix[0] == '\0') effective_prefix = "fe_tool";
	std::snprintf(buf, sizeof(buf), "%s/%s_%06d.vtk", folder, effective_prefix, step);
	FILE *fp = fopen(buf, "w+");
	if (!fp) return;

	fprintf(fp, "# vtk DataFile Version 2.0\n");
	fprintf(fp, "mfree iwf\n");
	fprintf(fp, "ASCII\n");
	fprintf(fp, "\n");
	fprintf(fp, "DATASET UNSTRUCTURED_GRID\n");

	fprintf(fp, "POINTS %d float\n", static_cast<int>(nodes_tool.size()));
	for (std::size_t i = 0; i < nodes_tool.size(); i++) {
		glm::dvec2 pw = tool->node_world(static_cast<unsigned int>(i));
		if (!std::isfinite(pw.x) || !std::isfinite(pw.y)) pw = glm::dvec2(0.);
		fprintf(fp, "%e %e %e\n", pw.x, pw.y, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "CELLS %d %d\n", static_cast<int>(tris.size()), static_cast<int>(4 * tris.size()));
	for (std::size_t i = 0; i < tris.size(); i++) {
		fprintf(fp, "3 %u %u %u\n", tris[i][0], tris[i][1], tris[i][2]);
	}
	fprintf(fp, "\n");

	fprintf(fp, "CELL_TYPES %d\n", static_cast<int>(tris.size()));
	for (std::size_t i = 0; i < tris.size(); i++) fprintf(fp, "5\n");
	fprintf(fp, "\n");

	fprintf(fp, "POINT_DATA %d\n", static_cast<int>(nodes_tool.size()));
	fprintf(fp, "SCALARS temperature double 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (std::size_t i = 0; i < nodes_tool.size(); i++) {
		fprintf(fp, "%e\n", tool->temperature_at_node(static_cast<unsigned int>(i)));
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS power double 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (std::size_t i = 0; i < nodes_tool.size(); i++) {
		fprintf(fp, "%e\n", tool->nodal_power(static_cast<unsigned int>(i)));
	}
	fprintf(fp, "\n");

	fprintf(fp, "VECTORS nodal_force double\n");
	for (std::size_t i = 0; i < nodes_tool.size(); i++) {
		glm::dvec2 f = tool->nodal_force(static_cast<unsigned int>(i));
		fprintf(fp, "%e %e %e\n", f.x, f.y, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "VECTORS pose_velocity double\n");
	{
		glm::dvec2 v = tool->get_vel();
		for (std::size_t i = 0; i < nodes_tool.size(); i++) {
			fprintf(fp, "%e %e %e\n", v.x, v.y, 0.);
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS fixed_ux int 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (std::size_t i = 0; i < nodes_tool.size(); i++) {
		fprintf(fp, "%d\n", tool->is_mechanics_fixed_x(static_cast<unsigned int>(i)) ? 1 : 0);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS fixed_uy int 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (std::size_t i = 0; i < nodes_tool.size(); i++) {
		fprintf(fp, "%d\n", tool->is_mechanics_fixed_y(static_cast<unsigned int>(i)) ? 1 : 0);
	}
	fprintf(fp, "\n");

	fclose(fp);
}
