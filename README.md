# CMSE 822: Parallel Computing Final Project

This will serve as the repo for your final project for the course. Complete all code development work in this repo. Commit early and often! For a detailed description of the project requirements, see [here](https://cmse822.github.io/projects).

First, give a brief description of your project topic idea and detail how you will address each of the specific project requierements given in the link above. 

I would really like to do a very simple and primitive turbulence simulation (using Navier Stokes, not MHD) with a very simple and primitive finite difference method, but I'm worried this is too involved. I would like to show the dependency of turbulence behavior as it relates to grid size (I don't think I will be able to implement different orders of solvers because that can become really involved).

If that doesn't work, I'm happy to do an MD implementation, but I'm not sure what simulations I would compare to...


# Actual Implementation of Navier Stokes

The [Navier Stokes Equation](https://www.britannica.com/science/Navier-Stokes-equation) in modern notation is given by:$$\frac{\partial \textbf{u}}{\partial t} + \textbf{u} \cdot \nabla \textbf{u} = -\frac{\nabla P}{\rho} + \nu \nabla^2 \textbf{u}$$ $$\nabla\cdot \textbf{u}=0 $$
with velocity vector $\textbf{u}$, scalar fluid pressure $P$, fluid density $\rho$ and viscocity $\nu$. This equation in component form becomes $$\frac{\partial u_x}{\partial t} + ux$$

Splitting these into component form, with $u_x=u$ and $u_y=v$ (I know my notation is sloppy but it is okay)

$$ \frac{\partial u}{\partial t} + \frac{\partial p}{\partial x} = \frac{1}{\text{Re}} \left( \frac{\partial^2 u}{\partial x^2} + \frac{\partial^2 u}{\partial y^2} \right) - \frac{\partial(u^2)}{\partial x} - \frac{\partial(uv)}{\partial y} + g_x$$
$$\frac{\partial v}{\partial t} + \frac{\partial p}{\partial y} = \frac{1}{\text{Re}} \left( \frac{\partial^2 v}{\partial x^2} + \frac{\partial^2 v}{\partial y^2} \right) - \frac{\partial(uv)}{\partial x} - \frac{\partial(v^2)}{\partial y} + g_y$$
$$\frac{\partial u}{\partial x} + \frac{\partial v}{\partial y} = 0$$

With Reynold's number $Re$ defined as $Re=\rho\sqrt{u^2+v^2}L/\mu$ and kinematic viscosity $\mu=\rho\nu$.

After initial conditions of $u$, $v$, and $p$ are set, I will use a 3 point forward/backward difference stencil depending on the sign of the current $u$ or $v$ to replace each of the above differential terms, to discretize my derivatives. Consider quantity $w(z)$, with discretized $z_k$ and spacing $\Delta z$. Now:

$$\bigl[\frac{\partial w}{\partial z}\bigr]_k^+=\frac{1}{2*\Delta z}(3w_k-4w_{k-1}+w_{k-2})$$
$$\bigl[\frac{\partial w}{\partial z}\bigr]_k^-=-\frac{1}{2*\Delta z}(3w_k-4w_{k+1}+w_{k+2})$$

$$\bigl[\frac{\partial^2w}{\partial z^2}\bigr]_k^+=\frac{1}{\Delta z^2}(w_k-2w_{k-1}+w_{k-2})$$
$$\bigl[\frac{\partial^2w}{\partial z^2}\bigr]_k^-=\frac{1}{\Delta z^2}(w_k-2w_{k+1}+w_{k+2})$$

Rather than a bunch of slow if/else statments for the upwinding, we can define: $s=sign(w_k)$ and our full stencil becomes:

$$\bigl[\frac{\partial w}{\partial z}\bigr]_k=\frac{s}{2*\Delta z}(3w_k-4w_{k-s}+w_{k-2s})$$
$$\bigl[\frac{\partial^2w}{\partial z^2}\bigr]_k=\frac{1}{\Delta z^2}(w_k-2w_{k-s}+w_{k-2s})$$

For each iteration, the Reynolds number $Re$ will also be calculated, and set to `invRe`=$\frac{\mu}{\rho L\sqrt{u^2+v^2}}$. Sometimes, when the velocity is very low, and `invRe` can get very large. When this happens, invRe is set to 5000.0 instead. This prevents the propagation of `inf` or `nan` values.