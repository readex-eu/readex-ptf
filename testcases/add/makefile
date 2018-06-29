FC  = mpif90
#FC = mpxlf90_r
  
# Instrument without MPI but with user region for faster analysis
#IFC = psc_instrument -s add.sir -t user,loop,call,mpi -v -d $(FC) $(MPI_INC)
#IFC = psc_instrument -s add.sir -v -d $(FC) $(MPI_INC)

# Instrument without MPI but without user region for application restart
#  IFC = psc_instrument -s add.sir -t loop,call -v $(FC) 

#Score-P instrumentation
IFC = scorep --online-access --user --nocompiler --noopenmp $(FC)
#IFC = $(FC)

CFLAGS=-O3

all: add.exe

add.exe: add.o
	$(IFC) $(CFLAGS) -o $@ $<

add.o: add.F90
	$(IFC) $(CFLAGS) add.F90 -c

clean:
	rm -rf add.exe add.o test.cmi
