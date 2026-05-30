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
#include "simulation_time.h"
#include <cstring>

void vtk_writer_write(const std::vector<particle> &particles, unsigned int step, const char *folder) {
	vtk_writer_write(particles, step, folder, nullptr, nullptr);
}

static int vtk_stage_id(const char *stage_label) {
	if (!stage_label) return 0;
	if (std::strcmp(stage_label, "cooldown") == 0) return 1;
	if (std::strcmp(stage_label, "residual-stress-ready") == 0) return 2;
	return 0;
}

void vtk_writer_write(const std::vector<particle> &particles, unsigned int step, const char *folder, const char *stage_label, const char *frame_label) {
	char buf[256];
	if (frame_label && frame_label[0] != '\0') {
		sprintf(buf, "%s/%s_%06d.vtk", folder, frame_label, step);
	} else if (stage_label && stage_label[0] != '\0') {
		sprintf(buf, "%s/%s_%06d.vtk", folder, stage_label, step);
	} else {
		sprintf(buf, "%s/out_%06d.vtk", folder, step);
	}
	FILE *fp = fopen(buf, "w+");

	unsigned int np = particles.size();

	simulation_time *time = &simulation_time::getInstance();
	const double t = time->get_time();
	const int stage = vtk_stage_id(stage_label ? stage_label : frame_label);

	fprintf(fp, "# vtk DataFile Version 2.0\n");
	if (frame_label && frame_label[0] != '\0') {
		fprintf(fp, "mfree iwf stage=%s\n", frame_label);
	} else if (stage_label && stage_label[0] != '\0') {
		fprintf(fp, "mfree iwf stage=%s\n", stage_label);
	} else {
		fprintf(fp, "mfree iwf\n");
	}
	fprintf(fp, "ASCII\n");
	fprintf(fp, "\n");

	fprintf(fp, "DATASET UNSTRUCTURED_GRID\n");		// Particle positions
	fprintf(fp, "POINTS %d float\n", np);
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f %f %f\n", particles[i].x, particles[i].y, 0.);
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

	fprintf(fp, "SCALARS time double 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%e\n", t);
	}
	fprintf(fp, "\n");

	fprintf(fp, "SCALARS stage int 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%d\n", stage);
	}
	fprintf(fp, "\n");

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

	fprintf(fp, "SCALARS pressure float 1\n");
	fprintf(fp, "LOOKUP_TABLE default\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f\n", particles[i].p);
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

	fprintf(fp, "TENSORS stress float\n");
	for (unsigned int i = 0; i < np; i++) {
		const double sxx = particles[i].Sxx - particles[i].p;
		const double sxy = particles[i].Sxy;
		const double syy = particles[i].Syy - particles[i].p;
		const double szz = particles[i].Szz - particles[i].p;
		fprintf(fp, "%f %f %f\n", sxx, sxy, 0.0);
		fprintf(fp, "%f %f %f\n", sxy, syy, 0.0);
		fprintf(fp, "%f %f %f\n", 0.0, 0.0, szz);
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

	fprintf(fp, "VECTORS displacement float\n");
	for (unsigned int i = 0; i < np; i++) {
		fprintf(fp, "%f %f %f\n", particles[i].x - particles[i].X, particles[i].y - particles[i].Y, 0.0);
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

	fclose(fp);
}

void vtk_writer_write(const tool* tool, unsigned int step, const char *folder) {
	vtk_writer_write(tool, step, folder, nullptr, nullptr);
}

void vtk_writer_write(const tool* tool, unsigned int step, const char *folder, const char *stage_label, const char *frame_label) {
	auto segments = tool->get_segments();
	if (segments.size() == 0) return;

	assert(segments.size() == 4 || segments.size() == 5);

	std::vector<glm::dvec2> outline;
	if (segments.size() == 4) {
		outline.push_back(segments[0].left);
		outline.push_back(segments[0].right);
		outline.push_back(segments[1].right);
		outline.push_back(segments[2].right);
	} else if (segments.size() == 5) {
		outline.push_back(segments[0].left);
		outline.push_back(segments[0].right);
		outline.push_back(segments[1].right);

		if (tool->get_fillet() != 0) {
			const int num_discr = 20;
			auto fillet = tool->get_fillet();
			const double r = fillet->r;
			const glm::dvec2 c = glm::dvec2(fillet->p.x, fillet->p.y);

			const glm::dvec2 br = segments[1].l.intersect(segments[3].l);
			const bool br_valid = std::isfinite(br.x) && std::isfinite(br.y);

			double start = fillet->t1;
			double end = fillet->t2;
			if (end < start) end += 2.0 * M_PI;

			if (br_valid) {
				double start_alt = fillet->t2;
				double end_alt = fillet->t1;
				if (end_alt < start_alt) end_alt += 2.0 * M_PI;

				const double mid = start + 0.5 * (end - start);
				const double mid_alt = start_alt + 0.5 * (end_alt - start_alt);

				const glm::dvec2 pm = glm::dvec2(c.x - r * cos(mid), c.y - r * sin(mid));
				const glm::dvec2 pm_alt = glm::dvec2(c.x - r * cos(mid_alt), c.y - r * sin(mid_alt));

				const double d = glm::length(pm - br);
				const double d_alt = glm::length(pm_alt - br);
				if (d_alt < d) {
					start = start_alt;
					end = end_alt;
				}
			}

			const double d_angle = (end - start) / (num_discr - 1);
			for (int i = 1; i < num_discr - 1; i++) {
				const double a = start + i * d_angle;
				outline.push_back(glm::dvec2(c.x - r * cos(a), c.y - r * sin(a)));
			}
		}

		outline.push_back(segments[2].right);
		outline.push_back(segments[3].right);
	}
	if (outline.size() < 3) return;

	char buf[256];
	if (frame_label && frame_label[0] != '\0') {
		sprintf(buf, "%s/%s_tool_%06d.vtk", folder, frame_label, step);
	} else if (stage_label && stage_label[0] != '\0') {
		sprintf(buf, "%s/%s_tool_%06d.vtk", folder, stage_label, step);
	} else {
		sprintf(buf, "%s/tool_%06d.vtk", folder, step);
	}
	FILE *fp = fopen(buf, "w+");

	fprintf(fp, "# vtk DataFile Version 2.0\n");
	if (frame_label && frame_label[0] != '\0') {
		fprintf(fp, "mfree iwf stage=%s\n", frame_label);
	} else if (stage_label && stage_label[0] != '\0') {
		fprintf(fp, "mfree iwf stage=%s\n", stage_label);
	} else {
		fprintf(fp, "mfree iwf\n");
	}
	fprintf(fp, "ASCII\n");
	fprintf(fp, "\n");
	fprintf(fp, "DATASET POLYDATA\n");
	fprintf(fp, "POINTS %d float\n", (int) outline.size());
	for (const auto& p : outline) {
		fprintf(fp, "%f %f %f\n", p.x, p.y, 0.);
	}
	fprintf(fp, "\n");

	fprintf(fp, "POLYGONS 1 %d\n", (int) outline.size() + 1);
	fprintf(fp, "%d", (int) outline.size());
	for (int i = 0; i < (int) outline.size(); i++) {
		fprintf(fp, " %d", i);
	}
	fprintf(fp, "\n");

	fclose(fp);

}
