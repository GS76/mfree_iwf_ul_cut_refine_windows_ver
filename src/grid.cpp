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

#include "grid.h"
#include "simulation_time.h"

#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <limits>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {
double parse_env_double_nonnegative(const char *name, double fallback) {
	const char *s = std::getenv(name);
	if (!s || s[0] == '\0') return fallback;
	char *end = nullptr;
	double v = std::strtod(s, &end);
	if (end == s || !std::isfinite(v) || v < 0.) return fallback;
	return v;
}

double runaway_bounds_factor() {
	static const double v = parse_env_double_nonnegative("MFREE_PARTICLE_BOUNDS_FACTOR", 200.);
	return v;
}

double runaway_bounds_margin_m() {
	static const double v = parse_env_double_nonnegative("MFREE_PARTICLE_BOUNDS_MARGIN_M", 0.01);
	return v;
}
[[noreturn]] void fail_invalid_hash(const particle &p,
                                    unsigned int particle_index,
                                    unsigned int num_particles,
                                    std::uint64_t hash,
                                    std::uint64_t num_cells,
                                    std::uint64_t nx,
                                    std::uint64_t ny,
                                    double bbmin_x,
                                    double bbmin_y,
                                    double dx) {
	const unsigned int step = simulation_time::getInstance().get_step();
	const double fx = (p.x - bbmin_x) / dx;
	const double fy = (p.y - bbmin_y) / dx;
	fprintf(stderr, "[GRID_HASH_OOB] step=%u i=%u n=%u hash=%llu num_cells=%llu nx=%llu ny=%llu\n",
	        step, particle_index, num_particles,
	        static_cast<unsigned long long>(hash),
	        static_cast<unsigned long long>(num_cells),
	        static_cast<unsigned long long>(nx),
	        static_cast<unsigned long long>(ny));
	fprintf(stderr, "  x=%e y=%e h=%e rho=%e m=%e refine_step=%u last_refine_at=%u\n",
	        p.x, p.y, p.h, p.rho, p.m, p.refine_step, p.last_refine_at);
	fprintf(stderr, "  hash_coords fx=%e fy=%e dx=%e bbmin=(%e,%e)\n",
	        fx, fy, dx, bbmin_x, bbmin_y);
	fprintf(stderr, "  finite_flags x=%d y=%d h=%d rho=%d m=%d dx=%d\n",
	        std::isfinite(p.x) ? 1 : 0,
	        std::isfinite(p.y) ? 1 : 0,
	        std::isfinite(p.h) ? 1 : 0,
	        std::isfinite(p.rho) ? 1 : 0,
	        std::isfinite(p.m) ? 1 : 0,
	        std::isfinite(dx) ? 1 : 0);
	fflush(stderr);
	std::exit(EXIT_FAILURE);
}

[[noreturn]] void fail_runaway_particle(const particle &p,
                                        unsigned int particle_index,
                                        unsigned int num_particles,
                                        double min_x,
                                        double max_x,
                                        double min_y,
                                        double max_y,
                                        double ref_min_x,
                                        double ref_max_x,
                                        double ref_min_y,
                                        double ref_max_y,
                                        double factor,
                                        double margin_m) {
	const unsigned int step = simulation_time::getInstance().get_step();
	fprintf(stderr, "[PARTICLE_BOUNDS_OOB] step=%u i=%u n=%u\n", step, particle_index, num_particles);
	fprintf(stderr, "  x=%e y=%e vx=%e vy=%e h=%e rho=%e m=%e\n",
	        p.x, p.y, p.vx, p.vy, p.h, p.rho, p.m);
	fprintf(stderr, "  refine_step=%u last_refine_at=%u fixed=%d\n",
	        p.refine_step, p.last_refine_at, p.fixed ? 1 : 0);
	fprintf(stderr, "  allowed_x=[%e,%e] allowed_y=[%e,%e]\n", min_x, max_x, min_y, max_y);
	fprintf(stderr, "  ref_bbox_x=[%e,%e] ref_bbox_y=[%e,%e] factor=%e margin_m=%e\n",
	        ref_min_x, ref_max_x, ref_min_y, ref_max_y, factor, margin_m);
	fprintf(stderr, "  finite_flags x=%d y=%d vx=%d vy=%d h=%d rho=%d m=%d\n",
	        std::isfinite(p.x) ? 1 : 0,
	        std::isfinite(p.y) ? 1 : 0,
	        std::isfinite(p.vx) ? 1 : 0,
	        std::isfinite(p.vy) ? 1 : 0,
	        std::isfinite(p.h) ? 1 : 0,
	        std::isfinite(p.rho) ? 1 : 0,
	        std::isfinite(p.m) ? 1 : 0);
	fflush(stderr);
	std::exit(EXIT_FAILURE);
}

[[noreturn]] void fail_invalid_grid_geometry(const char *reason,
                                             unsigned int num_particles,
                                             double minx,
                                             double maxx,
                                             double miny,
                                             double maxy,
                                             double h_max,
                                             double dx) {
	const unsigned int step = simulation_time::getInstance().get_step();
	fprintf(stderr, "[GRID_GEOMETRY_INVALID] step=%u n=%u reason=%s\n", step, num_particles, reason);
	fprintf(stderr, "  minx=%e maxx=%e miny=%e maxy=%e h_max=%e dx=%e\n",
	        minx, maxx, miny, maxy, h_max, dx);
	fflush(stderr);
	std::exit(EXIT_FAILURE);
}

[[noreturn]] void fail_neighbor_overflow(const particle &p,
                                         unsigned int particle_index,
                                         unsigned int num_particles,
                                         unsigned int attempted_neighbor_count,
                                         unsigned int max_neighbors,
                                         std::uint64_t cell_index,
                                         std::uint64_t nx,
                                         std::uint64_t ny,
                                         double dx) {
	const unsigned int step = simulation_time::getInstance().get_step();
	fprintf(stderr, "[GRID_NEIGHBOR_OVERFLOW] step=%u i=%u n=%u attempted=%u max=%u cell=%llu nx=%llu ny=%llu dx=%e\n",
	        step, particle_index, num_particles, attempted_neighbor_count, max_neighbors,
	        static_cast<unsigned long long>(cell_index),
	        static_cast<unsigned long long>(nx),
	        static_cast<unsigned long long>(ny),
	        dx);
	fprintf(stderr, "  x=%e y=%e h=%e rho=%e m=%e refine_step=%u last_refine_at=%u\n",
	        p.x, p.y, p.h, p.rho, p.m, p.refine_step, p.last_refine_at);
	fprintf(stderr, "  finite_flags x=%d y=%d h=%d rho=%d m=%d\n",
	        std::isfinite(p.x) ? 1 : 0,
	        std::isfinite(p.y) ? 1 : 0,
	        std::isfinite(p.h) ? 1 : 0,
	        std::isfinite(p.rho) ? 1 : 0,
	        std::isfinite(p.m) ? 1 : 0);
	fflush(stderr);
	std::exit(EXIT_FAILURE);
}
} // namespace
void grid::assign_hashes(std::vector<particle> &particles , unsigned int n) const {
	if (m_nx == 0 || m_ny == 0 || !(m_dx > 0.) || !std::isfinite(m_dx)) {
		fail_invalid_grid_geometry("invalid_grid_spacing_or_shape", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}
	if (m_num_cell >= std::numeric_limits<std::uint64_t>::max()) {
		fail_invalid_grid_geometry("num_cells_overflow_before_table_alloc", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}
	const std::uint64_t required64 = m_num_cell + 1;
	if (required64 > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
		fail_invalid_grid_geometry("num_cells_exceeds_addressable_table_size", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
	for (int ii = 0; ii < static_cast<int>(n); ii++) {
		const unsigned int i = static_cast<unsigned int>(ii);
		const double fx = (particles[i].x - m_bbmin_x)/m_dx;
		const double fy = (particles[i].y - m_bbmin_y)/m_dx;
		if (!std::isfinite(fx) || !std::isfinite(fy) || fx < 0. || fy < 0.) {
			fail_invalid_hash(particles[i], i, n, std::numeric_limits<std::uint64_t>::max(), m_num_cell, m_nx, m_ny, m_bbmin_x, m_bbmin_y, m_dx);
		}

		std::uint64_t ix = static_cast<std::uint64_t>(fx);
		std::uint64_t iy = static_cast<std::uint64_t>(fy);
		if (ix >= m_nx) ix = m_nx - 1;
		if (iy >= m_ny) iy = m_ny - 1;

		if (ix > std::numeric_limits<std::uint64_t>::max() / m_ny) {
			fail_invalid_hash(particles[i], i, n, std::numeric_limits<std::uint64_t>::max(), m_num_cell, m_nx, m_ny, m_bbmin_x, m_bbmin_y, m_dx);
		}
		particles[i].hash = ix*m_ny + iy;
	}
}

bool grid::in_bbox(glm::dvec3 qp) const {
	bool in_x = m_bbmin_x <= qp.x && qp.x <= m_bbmax_x;
	bool in_y = m_bbmin_y <= qp.y && qp.y <= m_bbmax_y;
	bool in_z = m_bbmin_z <= qp.z && qp.z <= m_bbmax_z;
	return in_x && in_y && in_z;
}

void grid::get_bbox(glm::dvec3 &bbmin, glm::dvec3 &bbmax) const {
	bbmin.x = m_bbmin_x;
	bbmin.y = m_bbmin_y;
	bbmin.z = m_bbmin_z;

	bbmax.x = m_bbmax_x;
	bbmax.y = m_bbmax_y;
	bbmax.z = m_bbmax_z;
}

void grid::unhash(std::uint64_t idx, std::uint64_t &i, std::uint64_t &j) const {
	i = idx/(m_ny);
	j = idx-(i)*(m_ny);
}

const std::vector<int> &grid::get_cells(const std::vector<particle> &particles, unsigned int n)  {
	if (n == 0) {
		m_cells.clear();
		return m_cells;
	}

	//needs to be sorted for this to work
	for (unsigned int i = 0; i < n-1; i++) {
		assert(particles[i].hash <= particles[i+1].hash);
	}

	for (unsigned int i = 0; i < n; i++) {
		if (particles[i].hash >= m_num_cell) {
			fail_invalid_hash(particles[i], i, n, particles[i].hash, m_num_cell, m_nx, m_ny, m_bbmin_x, m_bbmin_y, m_dx);
		}
	}
	if (n > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
		fail_invalid_grid_geometry("particle_count_exceeds_cell_index_type", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}
	if (m_num_cell >= std::numeric_limits<std::uint64_t>::max()) {
		fail_invalid_grid_geometry("num_cells_overflow_before_table_alloc", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}
	const std::uint64_t required64 = m_num_cell + 1;
	if (required64 > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
		fail_invalid_grid_geometry("num_cells_exceeds_addressable_table_size", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}
	const std::size_t required = static_cast<std::size_t>(required64);
	if (m_cells.size() != required) {
		m_cells.assign(required, -1);
	} else {
		std::fill(m_cells.begin(), m_cells.end(), -1);
	}

	m_cells[static_cast<std::size_t>(particles[0].hash)] = 0;

	for (unsigned int i = 0; i < n-1; i++) {
		if (particles[i].hash != particles[i+1].hash) {
			m_cells[static_cast<std::size_t>(particles[i+1].hash)] = static_cast<int>(i+1);
		}
	}
	m_cells[static_cast<std::size_t>(particles[n-1].hash+1)] = static_cast<int>(n);

	//empty boxes are now set to -1
	//in order to iterate through a cell by [cells(cell_index),...,cells(cell_index+1)[
	//those need to be fixed by propagating a "fix" value from the right
	//(such that the above range will just be empty)

	int fix = static_cast<int>(n);
	for (auto it = m_cells.rbegin(); it != m_cells.rend(); ++it) {
		if (*it == -1) {
			*it = fix;
		} else {
			fix = *it;
		}
	}

	return m_cells;
}

void grid::update_geometry(const std::vector<particle> &particles, unsigned int n, double kernel_width) {
	if (n == 0) {
		fail_invalid_grid_geometry("empty_particle_set", n, 0., 0., 0., 0., 0., 0.);
	}
	if (!std::isfinite(kernel_width) || !(kernel_width > 0.)) {
		fail_invalid_grid_geometry("invalid_kernel_width", n, 0., 0., 0., 0., 0., kernel_width);
	}
	double h_max = -DBL_MAX;

	double minx = +DBL_MAX;
	double maxx = -DBL_MAX;
	double miny = +DBL_MAX;
	double maxy = -DBL_MAX;

	for (unsigned int i = 0; i < n; i++) {
		if (!std::isfinite(particles[i].x) || !std::isfinite(particles[i].y) || !std::isfinite(particles[i].h)) {
			fail_invalid_grid_geometry("particle_state_non_finite_for_geometry", n, minx, maxx, miny, maxy, h_max, 0.);
		}
		h_max = fmax(particles[i].h, h_max);

		minx = fmin(particles[i].x, minx);
		miny = fmin(particles[i].y, miny);
		maxx = fmax(particles[i].x, maxx);
		maxy = fmax(particles[i].y, maxy);

	}
	if (!std::isfinite(h_max) || !(h_max > 0.) || !std::isfinite(minx) || !std::isfinite(maxx) || !std::isfinite(miny) || !std::isfinite(maxy)) {
		fail_invalid_grid_geometry("invalid_particle_extents_or_hmax", n, minx, maxx, miny, maxy, h_max, 0.);
	}
	if (maxx < minx || maxy < miny) {
		fail_invalid_grid_geometry("invalid_particle_bbox_order", n, minx, maxx, miny, maxy, h_max, 0.);
	}

	const double factor = runaway_bounds_factor();
	const double margin_m = runaway_bounds_margin_m();
	if (!m_bounds_ref_initialized) {
		m_bounds_ref_min_x = minx;
		m_bounds_ref_max_x = maxx;
		m_bounds_ref_min_y = miny;
		m_bounds_ref_max_y = maxy;
		m_bounds_ref_initialized = true;
	}
	if (factor > 0.) {
		const double ref_lx = std::max(m_bounds_ref_max_x - m_bounds_ref_min_x, 1e-12);
		const double ref_ly = std::max(m_bounds_ref_max_y - m_bounds_ref_min_y, 1e-12);
		const double ref_cx = 0.5*(m_bounds_ref_min_x + m_bounds_ref_max_x);
		const double ref_cy = 0.5*(m_bounds_ref_min_y + m_bounds_ref_max_y);
		const double half_x = 0.5*factor*ref_lx + margin_m;
		const double half_y = 0.5*factor*ref_ly + margin_m;
		const double allowed_min_x = ref_cx - half_x;
		const double allowed_max_x = ref_cx + half_x;
		const double allowed_min_y = ref_cy - half_y;
		const double allowed_max_y = ref_cy + half_y;
		for (unsigned int i = 0; i < n; i++) {
			const bool oob = !std::isfinite(particles[i].x) || !std::isfinite(particles[i].y) ||
			                 particles[i].x < allowed_min_x || particles[i].x > allowed_max_x ||
			                 particles[i].y < allowed_min_y || particles[i].y > allowed_max_y;
			if (oob) {
				fail_runaway_particle(particles[i], i, n,
				                      allowed_min_x, allowed_max_x,
				                      allowed_min_y, allowed_max_y,
				                      m_bounds_ref_min_x, m_bounds_ref_max_x,
				                      m_bounds_ref_min_y, m_bounds_ref_max_y,
				                      factor, margin_m);
			}
		}
	}

	//some nudging to prevent round off errors
	m_bbmin_x = minx - 1e-6;
	m_bbmax_x = maxx + 1e-6;
	m_bbmin_y = miny - 1e-6;
	m_bbmax_y = maxy + 1e-6;


	m_dx = h_max*kernel_width;
	if (!(m_dx > 0.) || !std::isfinite(m_dx)) {
		fail_invalid_grid_geometry("invalid_grid_spacing", n, minx, maxx, miny, maxy, h_max, m_dx);
	}

	m_lx = m_bbmax_x - m_bbmin_x;
	m_ly = m_bbmax_y - m_bbmin_y;
	if (m_lx < 0. || m_ly < 0. || !std::isfinite(m_lx) || !std::isfinite(m_ly)) {
		fail_invalid_grid_geometry("invalid_grid_lengths", n, minx, maxx, miny, maxy, h_max, m_dx);
	}

	m_nx = static_cast<std::uint64_t>(std::ceil(m_lx/m_dx));
	m_ny = static_cast<std::uint64_t>(std::ceil(m_ly/m_dx));
	if (m_nx == 0) m_nx = 1;
	if (m_ny == 0) m_ny = 1;
	if (m_nx > std::numeric_limits<std::uint64_t>::max()/m_ny) {
		fail_invalid_grid_geometry("num_cells_overflow", n, minx, maxx, miny, maxy, h_max, m_dx);
	}
	m_num_cell = m_nx*m_ny;
	if (m_num_cell == 0) {
		fail_invalid_grid_geometry("zero_cells_after_geometry_update", n, minx, maxx, miny, maxy, h_max, m_dx);
	}
}

void grid::debug_print() const {
	FILE *fp = fopen("grid.txt", "w+");
	for (std::uint64_t i = 0; i < m_nx; i++) {
		for (std::uint64_t j = 0; j < m_ny; j++) {
			double x_lo = m_bbmin_x + i*m_dx;
			double x_hi = m_bbmin_x + (i+1)*m_dx;

			double y_lo = m_bbmin_y + j*m_dx;
			double y_hi = m_bbmin_y + (j+1)*m_dx;

			fprintf(fp, "%f %f %f %f\n", x_lo, x_hi, y_lo, y_hi);
		}
	}
	fclose(fp);
}

// constructs verlet lists (particles[i]->nbh). indices are such that they point
// into array sorted by hash
void grid::construct_verlet_lists(std::vector<particle> &particles, unsigned int n, double kernel_width) {
	(void)kernel_width;

	std::vector<int> cells = get_cells(particles, n);
	if (m_num_cell > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
		fail_invalid_grid_geometry("num_cells_exceeds_parallel_loop_index_type", n, m_bbmin_x, m_bbmax_x, m_bbmin_y, m_bbmax_y, 0., m_dx);
	}

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 8)
#endif
	for (int bb = 0; bb < static_cast<int>(m_num_cell); bb++) {
		const std::uint64_t b = static_cast<std::uint64_t>(bb);
		std::uint64_t gi = 0;
		std::uint64_t gj = 0;

		unhash(b, gi, gj);

		const std::uint64_t low_i = (gi > 0) ? gi - 1 : 0;
		const std::uint64_t low_j = (gj > 0) ? gj - 1 : 0;
		const std::uint64_t high_i = std::min<std::uint64_t>(gi + 2, m_nx);
		const std::uint64_t high_j = std::min<std::uint64_t>(gj + 2, m_ny);

		for (int i = cells[static_cast<std::size_t>(b)]; i < cells[static_cast<std::size_t>(b + 1)]; i++) {

			unsigned int nbh_iter = 0;

			const double hi = particles[i].h;
			const double xi = particles[i].x;
			const double yi = particles[i].y;

			double radius2 = hi*hi*2*2;
			for (std::uint64_t ni = low_i; ni < high_i; ni++) {
				for (std::uint64_t nj = low_j; nj < high_j; nj++) {
					const std::size_t cell_idx = static_cast<std::size_t>(ni*m_ny + nj);
					const std::size_t next_cell_idx = cell_idx + 1;
					for (int j = cells[cell_idx]; j < cells[next_cell_idx]; j++) {

						const double xj = particles[j].x;
						const double yj = particles[j].y;

						const double xij = xi-xj;
						const double yij = yi-yj;

						const double r2 = xij*xij + yij*yij;

						if (r2 <= radius2) {
							if (nbh_iter >= MAX_NBH) {
								fail_neighbor_overflow(particles[i], static_cast<unsigned int>(i), n, nbh_iter + 1, MAX_NBH, b, m_nx, m_ny, m_dx);
							}
							particles[i].nbh[nbh_iter] = j;
							nbh_iter++;
						}

					}

				}
			}

			assert(nbh_iter <= MAX_NBH);

			if (nbh_iter == 0) {
				printf("alarm, particle with no neighbors found!\n");
			}

			particles[i].num_nbh = nbh_iter;
		}
	}
}

std::uint64_t grid::nx() const {
	return m_nx;
}
std::uint64_t grid::ny() const {
	return m_ny;
}

double grid::bbmin_x() const {
	return m_bbmin_x;
}

double grid::bbmin_y() const {
	return m_bbmin_y;
}

double grid::dx() const {
	return m_dx;
}

void grid::dbg_print_bbox() const {
	printf("%f %f %f\n", m_bbmin_x, m_bbmin_y, m_bbmin_z);
	printf("%f %f %f\n", m_bbmax_x, m_bbmax_y, m_bbmax_z);
}
