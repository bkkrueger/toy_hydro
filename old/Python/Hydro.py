###############################################################################
#                                                                             #
#  Hydrodynamics core                                                         #
#                                                                             #
#  This component performs the hydrodynamics computations.  This first        #
#  implementation is a highly-simplified hydrodynamics method: linear         #
#  advection of a single variable in one dimension.                           #
#                                                                             #
###############################################################################

# standard Python imports
import numpy as np

# imports specific to this code
import Grid

# =============================================================================

v_adv = None   # the advection speed
f_cfl = None   # the maximum fraction of a CFL time step

# =============================================================================

def init(parameters):
   """
   TODO
   """   # TODO

   global v_adv
   global f_cfl

   if 'v_adv' in parameters:
      v_adv = float(parameters['advection_speed'])
   else:
      v_adv = 1.0
   parameters['v_adv'] = v_adv

   if 'f_cfl' in parameters:
      f_cfl = float(parameters['f_cfl'])
      if ((f_cfl <= 0.0) or (1.0 < f_cfl)):
         raise #TODO
   else:
      f_cfl = 0.75
   parameters['f_cfl'] = f_cfl

   return parameters

# =============================================================================

def finalize():
   pass

# =============================================================================

def compute_time_step():
   """
   Compute the maximum time step acceptible to the Hydro component.

   Inputs:
   - none

   Outputs:
   - dt: the maximum allowed time step

   Side effects:
   - none
   """

   global v_adv
   global f_cfl

   # dt <= f_cfl * v_max * dx
   dt = f_cfl * v_adv * Grid.dx

   return dt

# =============================================================================

def one_step(dt):
   """
   Perform a single step of the hydrodynamics algorithm.

   Inputs:
   - dt: the current time step

   Outputs:
   - none

   Side effects:
   - Modifies the contents of the grid based on a single step of the
     hydrodynamics equations
   """

   # Compute the fluxes
   fluxes = compute_fluxes()

   # Update the variables
   update(fluxes, dt)

# =============================================================================

def compute_fluxes():
   """
   Compute the fluxes between the cells.

   Inputs:
   - none

   Outputs:
   - fluxes: the fluxes between the cells

   Side effects:
   - none
   """

   # Reconstruction
   lower, upper = reconstruction()

   # Riemann solve
   fluxes = riemann(lower, upper)

   return fluxes

# =============================================================================

def reconstruction():
   """
   Reconstruct the lower/upper states for the Riemann problem at each
   cell interface.  The lower state is immediately below the interface
   and the upper state is immediately above the interface.

   Inputs:
   - none

   Outputs:
   - lower: the states just below each interface
   - upper: the states just above each interface

   Side effects:
   - none

   Notes:
   - This version performs the simplest reconstruction, piecwise
     constant (Godunov), giving 1st-order convergence in space.
     Therefore, the lower value for interface i+0.5 is the same as the
     cell-center value of cell i, and the upper value for interface
     i+0.5 is the same as the cell-center value of cell i+1.
   - For Nx+2*Ng cells, there are Nx+2*Ng-1 interfaces to compute.  The
     interface between cells i and i+1 is called interface i+0.5, but in
     the arrays it is indexed as interface i; thus lower(i) and upper(i)
     are the inputs states to the Riemann solve for the flux between
     cells i and i+1.
   """
   
   lower = Grid.data[:-1]
   upper = Grid.data[1:]

   return lower, upper

# =============================================================================

def riemann(lower, upper):
   """
   Solve the Riemann problem at each interface.

   Inputs:
   - lower: the array of states below each interface
   - upper: the array of states above each interface

   Outputs:
   - fluxes: the fluxes across each interface

   Side effects:
   - none

   Notes:
   - This version performs a highly-simplified problem: linear advection
     in a single dimension with a constant speed.
   """

   # No advection velocity --- the fluxes are zero
   if (v_adv == 0):

      fluxes = np.zeros_like(lower)

   # The non-trivial cases
   else:

      if (v_adv > 0):
         interface = lower
      else:
         interface = upper

      fluxes = v_adv * interface

   return fluxes

# =============================================================================

def update(fluxes, dt):
   """
   Update the quantities on the grid based on the fluxes and time step.

   Inputs:
   - fluxes: the fluxes between the cells
   - dt    : the current time step

   Output:
   - none

   Side effects:
   - The grid variables change based on the hydrodynamics step.

   Notes:
   - As with the lower/upper states (discussed above in the comments on
     the reconstruction), the fluxes are indexes such that flux i is
     between cells i and i+1.  The sign convention is such that a
     positive flux i lowers cell i and increases cell i+1.
   """

   # The grid stores q, the density of Q.  Thus we can say that dQ = dq*dx*dA,
   # where dA is the area of the interface between the cells.  The flux is
   # F = dQ/dt/dA = dq*dx/dt.  Of course, we could also look at the advection
   # equation to see that dq/dt + dF/dx = 0.  From here we can show that q*dx
   # changes by F*dt: q*dx --> q*dx + F*dt.  This is essentially updating Q/A,
   # because A is constant in our 1D Cartesian model and can safely be ignored.
   # If we divide through by dx, we see that q --> q + F*dt/dx.
   F = fluxes * dt / Grid.dx

   # Matter flowing out to the right
   Grid.data[:-1] -= F

   # Matter flowing in from the left
   Grid.data[1:]  += F

