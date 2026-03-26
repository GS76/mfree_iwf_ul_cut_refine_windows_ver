# Technical Documentation: Artificial Stress, Damping, and Pressure Terms in the Meshfree Cutting Solver

## Introduction

This solver is a 2D, particle-based, Updated Lagrangian meshfree method (SPH-like) for large-deformation cutting. The three documented components are:

- **Pressure** $p$: computed per particle from a linear elastic equation of state (EOS) and coupled into the stress divergence as an isotropic term.
- **Artificial stresses** $\bm{R}$: a Monaghan-style tensile-instability correction, added inside the stress divergence operator to introduce repulsion under tension.
- **Damping / dissipation**: primarily (i) Monaghan artificial viscosity (compression-activated numerical damping) added directly to acceleration, and (ii) penalty contact + Coulomb friction forces from the tool, added as external forces. A further stabilizer, XSPH, smooths advection velocities.

The time integration is explicit and second-order (leapfrog predictor/corrector) where all right-hand-side (RHS) terms are assembled at the half step.

## Methodology

### Spatial discretization

The domain is discretized by particles indexed by $i=1,\dots,N$ with state variables

$$
(x_i,y_i,\rho_i,h_i,\bm{v}_i,\bm{S}_i,T_i), \qquad \bm{v}_i=(v_{x,i},v_{y,i}).
\label{eq:state}
$$

Here $\rho_i$ is density, $h_i$ is smoothing length, $\bm{S}_i$ is deviatoric stress (components $S_{xx},S_{xy},S_{yy},S_{zz}$), and $T_i$ is temperature.

#### Neighbor sets (Verlet lists)

Each particle $i$ maintains a neighbor list $\mathcal{N}(i)$ built by spatial hashing into a uniform grid each step. Neighbors are included when

$$
\|\bm{x}_i-\bm{x}_j\|^2 \le (2h_i)^2.
\label{eq:neighbor}
$$

#### Kernel and gradients

A 2D cubic spline kernel $W_{ij}=W(\bm{x}_i-\bm{x}_j,h_i)$ and its gradient $\nabla W_{ij}$ are used (support radius $2h$). Define

$$
\bm{x}_{ij}=\bm{x}_i-\bm{x}_j,\quad r_{ij}=\|\bm{x}_{ij}\|,\quad q=\frac{r_{ij}}{h_i}.
\label{eq:qdef}
$$

Then $W_{ij}$ and $\nabla W_{ij}=(\partial_x W_{ij},\partial_y W_{ij})$ are evaluated by the cubic spline formulas.

#### CSPM gradient correction (optional)

An optional corrective SPH procedure (CSPM) replaces $\nabla W_{ij}$ with a corrected gradient to improve consistency. Define particle ``volume'' $V_j=m_j/\rho_j$ and moment matrix

$$
\bm{B}_i=\sum_{j\in\mathcal{N}(i)} (\bm{x}_j-\bm{x}_i)\otimes \nabla W_{ij}\,V_j.
\label{eq:Bi}
$$

Then

$$
\nabla W_{ij}^{\,corr} = \nabla W_{ij}\,\bm{B}_i^{-1}.
\label{eq:cspm}
$$

### Temporal discretization (leapfrog predictor/corrector)

Let $u$ denote any particle state component (e.g., $x$, $\rho$, $v_x$, $S_{xx}$). With timestep $\Delta t$, the scheme is:

**Predict to half step**

$$
u_i^{n+\frac12}=u_i^{n}+\frac{\Delta t}{2}\,\dot{u}_i^{n}.
\label{eq:predict}
$$

**Compute RHS at half step**

All $\dot{u}^{n+\frac12}$ terms are assembled using the half-step state.

**Correct to full step**

$$
u_i^{n+1}=u_i^{n}+\Delta t\,\dot{u}_i^{n+\frac12}.
\label{eq:correct}
$$

### Per-step coupling procedure

At each time step, the solver performs the following operator-split procedure:

1. Build $\mathcal{N}(i)$ and precompute $W_{ij},\nabla W_{ij}$ (SPH or CSPM).
2. Predictor step $\eqref{eq:predict}$.
3. Reset RHS accumulators $\dot{u}_i\leftarrow 0$.
4. Apply contact + friction forces from tool (external forces).
5. Assemble continuum RHS:
   1. EOS pressure $p$.
   2. Artificial stress $\bm{R}$.
   3. Stress divergence (includes $p$ and $\bm{R}$).
   4. Velocity gradient.
   5. Artificial viscosity (adds to acceleration).
   6. XSPH (adds to kinematic advection).
   7. Jaumann deviatoric stress rate.
   8. Continuity $\dot{\rho}$.
   9. Momentum $\dot{\bm{v}}$.
   10. Advection $\dot{\bm{x}}$.
6. Corrector step $\eqref{eq:correct}$.
7. Plasticity update and boundary-condition enforcement.

## Derivations

### Pressure term

#### EOS formulation

Pressure is computed per particle by a linear elastic EOS with bulk modulus $K(T)$ and reference density $\rho_0(T)$:

$$
c_0(T)=\sqrt{\frac{K(T)}{\rho_0(T)}}.
\label{eq:c0}
$$

$$
p = c_0(T)^2\left(\rho-\rho_0(T)\right).
\label{eq:eos}
$$

**Definitions**

- $p$: pressure (scalar).
- $K(T)$: bulk modulus as a function of temperature.
- $\rho_0(T)$: reference density as a function of temperature.
- $c_0(T)$: reference wave speed used in the EOS.

#### Coupling into stress used for divergence

The solver stores deviatoric stress $\bm{S}$ and forms in-plane ``total'' stress components as

$$
\sigma_{xx}=S_{xx}-p,\quad \sigma_{yy}=S_{yy}-p,\quad \sigma_{xy}=S_{xy}.
\label{eq:sigma_from_Sp}
$$

### Artificial stress (Monaghan tensile-instability correction)

#### Local rotation to a principal-like frame

Define stress components $s_{xx},s_{xy},s_{yy}$ as in $\eqref{eq:sigma_from_Sp}$. A rotation angle is computed:

$$
\theta = \frac12 \operatorname{atan2}\!\left(2s_{xy},\,s_{xx}-s_{yy}+\varepsilon\right).
\label{eq:theta}
$$

Let $c=\cos\theta$ and $s=\sin\theta$. The rotated normal stresses are

$$
\tilde{s}_{xx}=c^2 s_{xx}+2cs\,s_{xy}+s^2 s_{yy},
\label{eq:rot_sxx}
$$

$$
\tilde{s}_{yy}=s^2 s_{xx}-2cs\,s_{xy}+c^2 s_{yy}.
\label{eq:rot_syy}
$$

#### Tensile-only repulsive correction

Let $\epsilon_{AS}>0$ be the artificial stress coefficient and $\rho$ the density. The correction acts only in tension:

$$
\tilde{R}_{xx}=
\begin{cases}
-\epsilon_{AS}\,\tilde{s}_{xx}\,\rho^{-2}, & \tilde{s}_{xx}>0,\\
0, & \tilde{s}_{xx}\le 0,
\end{cases}
\label{eq:Rxx_tilde}
$$

$$
\tilde{R}_{yy}=
\begin{cases}
-\epsilon_{AS}\,\tilde{s}_{yy}\,\rho^{-2}, & \tilde{s}_{yy}>0,\\
0, & \tilde{s}_{yy}\le 0.
\end{cases}
\label{eq:Ryy_tilde}
$$

The tensor is rotated back to yield $(R_{xx},R_{xy},R_{yy})$ in the global frame.

#### Injection into stress divergence

A kernel-ratio weight is computed:

$$
f_{ab}=\left(\frac{W_{ij}}{W_{\Delta p}}\right)^{n},
\qquad n\in\mathbb{N},\quad W_{\Delta p}>0.
\label{eq:fab}
$$

Then the pairwise artificial stress is

$$
\bm{R}_{ij}=f_{ab}\left(\bm{R}_i+\bm{R}_j\right).
\label{eq:Rij}
$$

A representative component form of the discrete divergence consistent with the implementation is:

$$
\begin{align}
(\nabla\cdot \bm{\sigma})_{x,i}\approx\;&
\sum_{j\in\mathcal{N}(i)} m_j\left(
\frac{\sigma_{xx,i}}{\rho_i^2}+\frac{\sigma_{xx,j}}{\rho_j^2}+R_{xx,ij}
\right)\frac{\partial W_{ij}}{\partial x}
\nonumber\\
&+
\sum_{j\in\mathcal{N}(i)} m_j\left(
\frac{\sigma_{xy,i}}{\rho_i^2}+\frac{\sigma_{xy,j}}{\rho_j^2}+R_{xy,ij}
\right)\frac{\partial W_{ij}}{\partial y}.
\label{eq:stress_div_x}
\end{align}
$$

### Damping / dissipation mechanisms

#### Artificial viscosity (Monaghan-type)

Define relative kinematics

$$
\bm{v}_{ij}=\bm{v}_i-\bm{v}_j,\quad \bm{x}_{ij}=\bm{x}_i-\bm{x}_j.
\label{eq:rel_kin}
$$

Viscosity activates only under approach:

$$
\bm{v}_{ij}\cdot\bm{x}_{ij}<0.
\label{eq:approach}
$$

Define averaged quantities

$$
h_{ij}=\frac12(h_i+h_j),\quad \rho_{ij}=\frac12(\rho_i+\rho_j),\quad c_{ij}=\frac12(c_i+c_j),
\label{eq:avg}
$$

with wave speed (as used in the code)

$$
c_i=\sqrt{\frac{K(T_i)}{\rho_i}}.
\label{eq:ci}
$$

Then

$$
\mu_{ij}=\frac{h_{ij}\,(\bm{v}_{ij}\cdot\bm{x}_{ij})}{\|\bm{x}_{ij}\|^2+\eta_{AV}^2 h_{ij}^2},
\label{eq:muij}
$$

$$
\Pi_{ij}=\frac{-\alpha_{AV}c_{ij}\mu_{ij}+\beta_{AV}\mu_{ij}^2}{\rho_{ij}}.
\label{eq:piij}
$$

The acceleration contribution is

$$
\dot{\bm{v}}_i \mathrel{+}= -\sum_{j\in\mathcal{N}(i)} m_j\,\Pi_{ij}\,\nabla W_{ij}.
\label{eq:av_accel}
$$

#### Tool contact penalty (normal) and friction (tangential)

Let $g_N$ be penetration depth and $\bm{n}$ the surface normal returned by the tool contact query. With particle mass $m_s$ and timestep $\Delta t$:

$$
\bm{f}_N = -\alpha_C\,m_s\,\frac{g_N}{\Delta t^2}\,\bm{n}.
\label{eq:contact_penalty}
$$

Friction uses a clamped update with coefficient $\mu$:

$$
\|\bm{f}_T\| \le \mu \|\bm{f}_N\|.
\label{eq:coulomb}
$$

The resulting forces enter the momentum equation as external forces:

$$
\dot{\bm{v}}_i \mathrel{+}= \frac{\bm{f}_{c,i}+\bm{f}_{t,i}}{m_i}.
\label{eq:contact_mom}
$$

#### XSPH kinematic smoothing (stabilization)

XSPH modifies the position RHS with parameter $\epsilon_{XSPH}$ and $\rho_{ij}=\frac12(\rho_i+\rho_j)$:

$$
\dot{\bm{x}}_i \mathrel{+}= -\epsilon_{XSPH}\sum_{j\in\mathcal{N}(i)} \frac{m_i}{\rho_{ij}}(\bm{v}_i-\bm{v}_j)\,W_{ij}.
\label{eq:xsph}
$$

## Results

At each timestep, after assembling RHS terms at the half-step state, the solver produces:

- Pressure field $p_i$ via $\eqref{eq:eos}$.
- Artificial stress correction $(R_{xx,i},R_{xy,i},R_{yy,i})$ via $\eqref{eq:theta}$--$\eqref{eq:Ryy_tilde}$ and coupling $\eqref{eq:fab}$--$\eqref{eq:Rij}$.
- Internal acceleration contributions from stress divergence (e.g., $\eqref{eq:stress_div_x}$) and artificial viscosity $\eqref{eq:av_accel}$.
- External contact/friction forces and their contribution to $\dot{\bm{v}}_i$ via $\eqref{eq:contact_mom}$.
- Updated state $u^{n+1}$ via $\eqref{eq:correct}$.

### Boundary conditions

Fixed particles are enforced by resetting to reference position $(X_i,Y_i)$ and zeroing velocities and kinematic RHS terms:

$$
\bm{x}_i \leftarrow \bm{X}_i,\quad \bm{v}_i\leftarrow \bm{0},\quad \dot{\bm{x}}_i\leftarrow \bm{0},\quad \dot{\bm{v}}_i\leftarrow \bm{0}.
\label{eq:fixed_bc}
$$

## Conclusions

- Pressure is computed by a temperature-dependent linear EOS $\eqref{eq:eos}$ and enters the momentum balance through $\bm{\sigma}=\bm{S}-p\bm{I}$ $\eqref{eq:sigma_from_Sp}$ inside a symmetric SPH stress divergence.
- Artificial stress is a tensile-instability stabilization: it detects tensile principal-like stresses via rotation $\eqref{eq:theta}$--$\eqref{eq:rot_syy}$, applies a repulsive correction only in tension $\eqref{eq:Rxx_tilde}$--$\eqref{eq:Ryy_tilde}$, and injects it into the stress divergence with a kernel-ratio weighting $\eqref{eq:fab}$--$\eqref{eq:Rij}$.
- Damping/dissipation is dominated by Monaghan artificial viscosity active only for approaching particles $\eqref{eq:approach}$--$\eqref{eq:av_accel}$, and penalty contact plus friction forces from the tool $\eqref{eq:contact_penalty}$--$\eqref{eq:contact_mom}$. XSPH provides additional stabilization by smoothing advection velocities $\eqref{eq:xsph}$.
- Coupling is explicit and operator-split, evaluated at the half step, and advanced by leapfrog $\eqref{eq:predict}$--$\eqref{eq:correct}$.

