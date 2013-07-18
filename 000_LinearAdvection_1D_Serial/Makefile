FLAGS = -I /opt/local/include
LDFLAGS = -L /opt/local/lib -l boost_filesystem-mt -l boost_system-mt

OBJDIR = build

Main :  $(OBJDIR)/Main.o $(OBJDIR)/Driver.o $(OBJDIR)/Grid.o \
	$(OBJDIR)/Hydro.o $(OBJDIR)/InitConds.o $(OBJDIR)/Monitor.o
	g++ $(FLAGS) $(LDFLAGS) -o Main $(OBJDIR)/*.o

$(OBJDIR)/Main.o : Main.cpp Driver.hpp $(OBJDIR)
	g++ $(FLAGS) -o $(OBJDIR)/Main.o -c Main.cpp

$(OBJDIR)/Driver.o : Driver.cpp Driver.hpp $(OBJDIR)
	g++ $(FLAGS) -o $(OBJDIR)/Driver.o -c Driver.cpp

$(OBJDIR)/Grid.o : Grid.cpp Grid.hpp Array.hpp Driver.hpp $(OBJDIR)
	g++ $(FLAGS) -o $(OBJDIR)/Grid.o -c Grid.cpp

$(OBJDIR)/Hydro.o : Hydro.cpp Hydro.hpp Array.hpp Driver.hpp Grid.hpp $(OBJDIR)
	g++ $(FLAGS) -o $(OBJDIR)/Hydro.o -c Hydro.cpp

$(OBJDIR)/InitConds.o : InitConds.cpp InitConds.hpp Driver.hpp Grid.hpp \
	$(OBJDIR)
	g++ $(FLAGS) -o $(OBJDIR)/InitConds.o -c InitConds.cpp

$(OBJDIR)/Monitor.o : Monitor.cpp Monitor.hpp Driver.hpp Grid.hpp $(OBJDIR)
	g++ $(FLAGS) -o $(OBJDIR)/Monitor.o -c Monitor.cpp

clean :
	rm -f $(OBJDIR)/*.o

$(OBJDIR) :
	mkdir $(OBJDIR)

