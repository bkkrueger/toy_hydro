###############################################################################
#                                                                             #
#  Driver                                                                     #
#                                                                             #
#  This component is the entry point for the rest of the code.                #
#                                                                             #
###############################################################################

# General Python stuff
import argparse
import ConfigParser
import math
import os

# Imports specific to this code
# - I use "import x as x" for future development, when there may be more than
#   one implementation of the various components, so that you can do, for
#   example, "import GammaLaw as EoS" and the rest of the code will be written
#   generically in terms of EoS.
import Grid as Grid
import Hydro as Hydro
import IC as IC

# =============================================================================

MAX_ITER  = None
output_dt = None
time      = None
tmax      = None

# =============================================================================

def parse_command_line():
   """
   Parse the command-line arguments.
   """

   desc = "Perform fluid dynamics simulations."
   parser = argparse.ArgumentParser(description=desc)

   # Parameter file
   help_txt = "name of the configuration file (default is 'config.ini.')"
   parser.add_argument("-f", "--file", metavar="FILE", default="config.ini",
                       required=False, dest="config_file", help=help_txt)

   return parser.parse_args()

# =============================================================================

def parse_config_file(config_file):
   """
   Parse the configuration file.
   """

   config = ConfigParser.SafeConfigParser()
   config.read(config_file)

   config_dict = {}
   for section in config.sections():
      # TODO : Should I force all section names to lowercase?
      config_dict[section.strip()] = dict(config.items(section))


   return config_dict

# =============================================================================

def init():
   """
   Handle the initialization for the Driver component.
   """

   global MAX_ITER
   global output_dt
   global time
   global tmax

   # ==========================================================================
   # Arguments and parameters

   # Read the command-line arguments
   cline_args = parse_command_line()

   # Read the parameter file
   cfile_args = parse_config_file(cline_args.config_file)

   # Add the command-line arguments as section 'CommandLine'
   # TODO --- test this
   cfile_args['CommandLine'] = vars(cline_args)

   # ==========================================================================
   # Initialize the Driver component

   if 'Driver' not in cfile_args:
      cfile_args['Driver'] = {}

   # Maximum number of iterations
   if 'max_iter' in cfile_args['Driver']:
      MAX_ITER = int(cfile_args['Driver']['max_iter'])
   else:
      MAX_ITER = 10
   # Store either the default value or the value from the config file converted
   # from string to integer
   cfile_args['Driver']['max_iter'] = MAX_ITER

   # Time step between outputs
   if 'output_dt' in cfile_args['Driver']:
      output_dt = float(cfile_args['Driver']['output_dt'])
   else:
      output_dt = 0.0
   cfile_args['Driver']['output_dt'] = output_dt

   # Current time
   time = 0.0

   # Final time
   if 'tmax' in cfile_args['Driver']:
      tmax = float(cfile_args['Driver']['tmax'])
   else:
      raise Exception("tmax required") #TODO

   # ==========================================================================
   # Initialize the other components
   # - Each initialization updates its own parameters dictionary (adding
   #   default values for anything not specified on the command line or in the
   #   configuration file) and returns the updated dictionary for logging.

   # Initialize the grid
   if "Grid" not in cfile_args:
      cfile_args['Grid'] = {}
   cfile_args['Grid'] = Grid.init(cfile_args['Grid'])

   # Initialize the hydro
   if "Hydro" not in cfile_args:
      cfile_args['Hydro'] = {}
   cfile_args['Hydro'] = Hydro.init(cfile_args['Hydro'])

   # ==========================================================================
   # Write parameters to log file

   if not os.path.exists("output"):
      os.makedirs("output")

   f = open("output/parameters.txt",'w')
   for section in cfile_args:
      f.write("[ " + section + " ]\n")
      for item in cfile_args[section]:
         f.write("   " + item + " : " + str(cfile_args[section][item]) + "\n")
   f.close()

   # ==========================================================================
   # Set up initial conditions
   IC.set_initial_conditions()

   return cfile_args

# =============================================================================

def finalize():

   Grid.finalize()
   Hydro.finalize()

# =============================================================================

def compute_time_step():
   """
   Compute the time step.

   This may depend on multiple components, so it is packaged as a Driver
   function even though the current version only cares about the Hydro
   component.
   """

   dt = Hydro.compute_time_step()

   return dt

# =============================================================================

def main():
   """
   The main core of the program.
   """

   global time
   global MAX_ITER
   global output_dt

   # ==========================================================================
   # Initialize
   # - Returns a dictionary of dictionaries, which are the parameters from the
   #   configuration file (by section) and from the command line (listed as
   #   section 'CommandLine')
   params = init()

   # ==========================================================================
   # Evolution loop

   prev_write = -1

   for n_iter in xrange(MAX_ITER):

      # Exceeded maximum time
      if time >= tmax:
         break

      # Unexpected exit? (e.g. like "touch .dump_restart")
      # TODO

      # Write output
      do_write = False
      if output_dt > 0:
         curr_write = int(math.floor(time / output_dt))
         if curr_write > prev_write:
            do_write = True
         prev_write = curr_write
      if do_write:   # to allow for a condition based on step number
         Grid.write_data(n_iter)

      # Boundary condition fill
      Grid.fill_boundary_conditions()

      # Compute step size
      dt = compute_time_step()

      # Evolve a single step
      Hydro.one_step(dt)

      # Update time
      time = time + dt

   # End evolution loop

   # ==========================================================================
   # Finalize

   # Write final output
   Grid.write_data(n_iter+1)
   finalize()

