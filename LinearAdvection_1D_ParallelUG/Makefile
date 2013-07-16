#CCOMP = g++
CCOMP = mpic++
FLAGS = -I /opt/local/include
LDFLAGS = -L /opt/local/lib -l boost_filesystem-mt -l boost_system-mt

OBJDIR = build

Main :  $(OBJDIR)/Main.o $(OBJDIR)/Driver.o $(OBJDIR)/Grid.o \
	$(OBJDIR)/Hydro.o $(OBJDIR)/InitConds.o $(OBJDIR)/Log.o \
	$(OBJDIR)/Monitor.o $(OBJDIR)/Parameters.o
	$(CCOMP) $(FLAGS) $(LDFLAGS) -o Main $(OBJDIR)/*.o

$(OBJDIR)/Main.o : Main.cpp \
	                Driver.hpp \
						 Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Main.o -c Main.cpp

$(OBJDIR)/Driver.o : Driver.cpp Driver.hpp \
							Log.hpp Parameters.hpp \
	                  Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Driver.o -c Driver.cpp

$(OBJDIR)/Grid.o : Grid.cpp Grid.hpp \
	                Array.hpp Driver.hpp Log.hpp \
						 Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Grid.o -c Grid.cpp

$(OBJDIR)/Hydro.o : Hydro.cpp Hydro.hpp \
	                 Array.hpp Driver.hpp Grid.hpp \
						  Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Hydro.o -c Hydro.cpp

$(OBJDIR)/InitConds.o : InitConds.cpp InitConds.hpp \
	                     Driver.hpp Grid.hpp \
								Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/InitConds.o -c InitConds.cpp

$(OBJDIR)/Monitor.o : Monitor.cpp Monitor.hpp \
	                   Driver.hpp Grid.hpp \
							 Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Monitor.o -c Monitor.cpp

$(OBJDIR)/Parameters.o : Parameters.cpp Parameters.hpp \
								 Driver.hpp Log.hpp \
								 Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Parameters.o -c Parameters.cpp

$(OBJDIR)/Log.o : Log.cpp Log.hpp \
	               Driver.hpp Parameters.hpp \
						Defines.hpp $(OBJDIR)
	$(CCOMP) $(FLAGS) -o $(OBJDIR)/Log.o -c Log.cpp

clean :
	rm -f $(OBJDIR)/*.o

$(OBJDIR) :
	mkdir $(OBJDIR)

