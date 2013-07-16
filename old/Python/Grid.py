###############################################################################
#                                                                             #
#  Grid storage                                                               #
#                                                                             #
#  This component maintains the grid information.  This includes the size of  #
#  the grid, the geometry of the grid, the arrays holding the variables, and  #
#  so on.  This first implementation is a highly-simplified grid: Cartesian,  #
#  1D, single-variable.                                                       #
#                                                                             #
###############################################################################

# Standard Python imports
import math
import numpy as np
import os
import shutil

# Imports unique to this code
import Driver

# =============================================================================

Ng   = None # the number of guard (aka ghost) cells around the borders
Ng2  = None # twice the number of guard cells (as it comes up a lot)
Nx   = None # the number of internal (non-guard) cells in the x direction

xmin = None
xmax = None
dx   = None
x    = None

data = None # the data grid

# =============================================================================

def init(parameters):
   """
   Initialize the grid.
   """

   global Ng
   global Ng2
   global Nx
   global xmin
   global xmax
   global dx
   global x
   global data

   # Number of guard cells
   Ng  = 1
   Ng2 = 2 * Ng

   # Number of internal cells
   if 'nx' in parameters:
      Nx = int(parameters['nx'])
   else:
      raise Exception("Nx required") #TODO
   parameters['nx'] = Nx

   # Limits
   if 'xmin' in parameters:
      xmin = float(parameters['xmin'])
   else:
      raise Exception("xmin required") #TODO
   parameters['xmin'] = xmin
   if 'xmax' in parameters:
      xmax = float(parameters['xmax'])
   else:
      raise Exception("xmax required") #TODO
   parameters['xmax'] = xmax

   # Set up grid coordinates
   dx   = (xmax - xmin) / Nx
   x    = xmin + dx*(0.5+np.arange(Nx+Ng2)-Ng)

   # Set up the grid
   data = np.zeros(Nx+Ng2)

   return parameters

# =============================================================================

def finalize():
   pass

# =============================================================================

def fill_boundary_conditions():
   """
   Fill the guard cells based on the boundary conditions.  This is a simple
   implementation, so it assumes periodic boundaries."
   """

   global data

   data[    0:     Ng] = data[Nx:Ng+Nx]
   data[Nx+Ng:Nx+2*Ng] = data[Ng:2*Ng]

# =============================================================================

def write_data(step_num):
   """
   Write the data to a file.
   """

   global x
   global data

   # Create the subdirectory for the current output
   w = str(int(math.floor(math.log10(Driver.MAX_ITER))) + 1)
   directory = "output/step_{n:0" + w + "d}"
   directory = directory.format(n=step_num)
   if os.path.exists(directory):
      shutil.rmtree(directory)
   os.makedirs(directory)

   # Write the important header information
   filename = directory + "/header.txt"
   f = open(filename, 'w')
   f.write("time = {t:20.13e}\n".format(t=Driver.time))
   f.write("step = " + str(step_num) + "\n")
   f.close()

   # Write the data
   filename = directory + "/grid.dat"
   outdata = np.empty((Nx,2))
   outdata[:,0] = x[Ng:Ng+Nx]
   outdata[:,1] = data[Ng:Ng+Nx]
   np.savetxt(filename, outdata)

