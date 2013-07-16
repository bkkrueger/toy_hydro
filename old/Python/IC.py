###############################################################################
#                                                                             #
#  Initial conditions                                                         #
#                                                                             #
#  This component implements the initial conditions for the simulation.       #
#                                                                             #
###############################################################################

# Standard Python imports
import numpy as np

# Imports unique to this code
import Grid

# =============================================================================

def set_initial_conditions():
   """
   Set the initial conditions
   """

   x0 = 0.0
   dx = 1.5
   y0 = 10.0
   dy = 1.25
   Grid.data[:] = y0 + dy * np.exp(-1.0 * ((Grid.x - x0)/dx)**2)

   #Grid.data[:] = 10.0
   #Grid.data[(x0-dx < Grid.x) & (Grid.x < x0+dx)] = 8.0

