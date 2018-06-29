## -*- mode: autoconf -*-

# The macros AC_PSC_COMPILER_AND_FLAGS is based on
# AC_SCOREP_COMPILER_AND_FLAGS http://www.score-p.org. AC_SCOREP_COMPILER_AND_FLAGS came with following license:
#
#Copyright (c) 2009-2011,
#   RWTH Aachen University, Germany
#   Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
#   Technische Universitaet Dresden, Germany
#   University of Oregon, Eugene, USA
#   Forschungszentrum Juelich GmbH, Germany
#   German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
#   Technische Universitaet Muenchen, Germany
#
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are
#met:
#
#* Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#* Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#* Neither the names of
#   RWTH Aachen University,
#   Gesellschaft fuer numerische Simulation mbH Braunschweig,
#   Technische Universitaet Dresden,
#   University of Oregon, Eugene,
#   Forschungszentrum Juelich GmbH,
#   German Research School for Simulation Sciences GmbH, or the
#   Technische Universitaet Muenchen,
#  nor the names of their contributors may be used to endorse or promote
#  products derived from this software without specific prior written
#  permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AC_DEFUN([AC_PSC_CONVERT_FOR_BUILD_FLAGS],
[
if test "x${ac_cv_env_[$1]_FOR_BUILD_set}" != "xset"; then
   # don't use the default flags if nothing is specified for the frontend
   unset [$1]
else
   # use the FOR_BUILD flags 
   [$1]="$ac_cv_env_[$1]_FOR_BUILD_value"
fi
])

AC_DEFUN([AC_PSC_CONVERT_MPI_FLAGS],
[
if test "x${ac_cv_env_MPI_[$1]_set}" != "xset"; then
   # don't use the default flags if nothing is specified for MPI
   unset [$1]
else
   # use the MPI flags 
   [$1]="$ac_cv_env_MPI_[$1]_value"
fi
])

AC_DEFUN([AC_PSC_CHECK_COMPILER_VAR_SET],
[
if test "x${ac_cv_env_[$1]_set}" != "xset"; then
    AC_MSG_ERROR([argument $1 not provided in configure call.], [1])
fi
])


AC_DEFUN([AC_PSC_CONVERT_FOR_BUILD_COMPILER],
[
if test "x${ac_cv_env_[$1]_FOR_BUILD_set}" != "xset"; then
    # don't use the default compiler if nothing is specified for the frontend
    unset [$1]
else
    [$1]="$ac_cv_env_[$1]_FOR_BUILD_value"
fi
])

AC_DEFUN([AC_PSC_CONVERT_MPI_COMPILER],
[
if test "x${ac_cv_env_MPI[$1]_set}" != "xset"; then
    # don't use the default compiler if nothing is specified for MPI
    unset [$1]
else
    [$1]="$ac_cv_env_MPI[$1]_value"
fi
])


dnl dont' use together with AC_PSC_WITH_NOCROSS_COMPILER_SUITE
AC_DEFUN([AC_PSC_WITH_COMPILER_SUITE],
[
m4_pattern_allow([AC_PSC_WITH_COMPILER_SUITE])
m4_pattern_allow([AC_PSC_WITH_NOCROSS_COMPILER_SUITE])
if test "x${ac_psc_compiler_suite_called}" != "x"; then
    # We need m4 quoting magic here ...
    AC_MSG_ERROR([cannot use [AC_PSC_WITH_COMPILER_SUITE] and [AC_PSC_WITH_NOCROSS_COMPILER_SUITE] in one configure.ac.])
else
    ac_psc_compiler_suite_called="yes"
fi

path_to_compiler_files="$srcdir/m4/platforms/"

AC_ARG_WITH([compiler-suite],
            [AS_HELP_STRING([--with-compiler-suite=(gcc|ibm|intel|pathscale|pgi|studio)], 
                            [The compiler suite to build this package in non cross-compiling environments with. Needs to be in $PATH [gcc].])],
            [AS_IF([test "x${ac_psc_cross_compiling}" = "xno"], 
                   [AS_CASE([$withval],
                            ["gcc"],       [ac_psc_compilers_backend="${path_to_compiler_files}compiler-nocross-gcc"],
                            ["ibm"],       [ac_psc_compilers_backend="${path_to_compiler_files}compiler-nocross-ibm"],
                            ["intel"],     [ac_psc_compilers_backend="${path_to_compiler_files}compiler-nocross-intel"],
                            ["pathscale"], [ac_psc_compilers_backend="${path_to_compiler_files}compiler-nocross-pathscale"],
                            ["pgi"],       [ac_psc_compilers_backend="${path_to_compiler_files}compiler-nocross-pgi"],
                            ["studio"],    [ac_psc_compilers_backend="${path_to_compiler_files}compiler-nocross-studio"],
                            [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-compiler-suite, ignoring.])])],
                   [AC_MSG_ERROR([Option --with-compiler-suite not supported in cross-compiling mode. Please use --with-backend-compiler-suite and --with-frontend-compiler-suite instead.])])],
            [])
cat ${ac_psc_compilers_backend} > compiler_suite
. ./compiler_suite
dnl backend-compiler-suite is not very useful. if we are on a cross-compiling
dnl platform, we usually want to use the vendor tools that should be detected
dnl automatically. otherwise, use platform-*-user-provided
dnl AC_ARG_WITH([backend-compiler-suite],
dnl             [AS_HELP_STRING([--with-backend-compiler-suite=(ibm|sx)], 
dnl                             [The compiler suite to build the backend parts of this package in cross-compiling environments with. Needs to be in $PATH [gcc].])],
dnl             [AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"], 
dnl                    [AS_CASE([$withval],
dnl                             ["ibm"],       [ac_scorep_compilers_backend="${path_to_compiler_files}compiler-backend-ibm"],
dnl                             ["sx"],        [ac_scorep_compilers_backend="${path_to_compiler_files}compiler-backend-sx"],
dnl                             [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-backend-compiler-suite, ignoring.])])], 
dnl                    [AC_MSG_ERROR([Option --with-backend-compiler-suite not supported in non cross-compiling mode. Please use --with-nocross-compiler-suite instead.])])],
dnl             [])

dnl For now Periscope doesn't have cross-compiling 
dnl
dnl AC_ARG_WITH([frontend-compiler-suite],
dnl            [AS_HELP_STRING([--with-frontend-compiler-suite=(gcc|ibm|intel|pathscale|pgi|studio)], 
dnl                            [The compiler suite to build the frontend parts of this package in cross-compiling environments with. Needs to be in $PATH [gcc].])],
dnl            [AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"], 
dnl                   [AS_CASE([$withval],
dnl                            ["gcc"],       [ac_scorep_compilers_frontend="${path_to_compiler_files}compiler-frontend-gcc"],
dnl                            ["ibm"],       [ac_scorep_compilers_frontend="${path_to_compiler_files}compiler-frontend-ibm"],
dnl                            ["intel"],     [ac_scorep_compilers_frontend="${path_to_compiler_files}compiler-frontend-intel"],
dnl                            ["pathscale"], [ac_scorep_compilers_frontend="${path_to_compiler_files}compiler-frontend-pathscale"],
dnl                            ["pgi"],       [ac_scorep_compilers_frontend="${path_to_compiler_files}compiler-frontend-pgi"],
dnl                            ["studio"],    [ac_scorep_compilers_frontend="${path_to_compiler_files}compiler-frontend-studio"],
dnl                            [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-frontend-compiler-suite, ignoring.])])],
dnl                   [AC_MSG_ERROR([Option --with-frontend-compiler-suite not supported in non cross-compiling mode. Please use --with-nocross-compiler-suite instead.])])],
dnl            [])
])


AC_DEFUN([AC_PSC_WITH_MPI_COMPILER_SUITE],
[
echo `pwd`
echo $path_to_compiler_files
path_to_compiler_files="$srcdir/m4/platforms/"

AC_ARG_WITH([mpi],
            [AS_HELP_STRING([--with-mpi=(mpich2|impi|openmpi)], 
                            [The mpi compiler suite to build this package with. Needs to be in $PATH [mpich2].])],
            [AS_CASE([$withval],
                     ["mpich2"],      [ac_psc_compilers_mpi="${path_to_compiler_files}compiler-mpi-mpich2"],
                     ["impi"],        [ac_psc_compilers_mpi="${path_to_compiler_files}compiler-mpi-impi"],
                     ["openmpi"],     [ac_psc_compilers_mpi="${path_to_compiler_files}compiler-mpi-openmpi"],
                     [AC_MSG_WARN([MPI compiler suite "${withval}" not supported by --with-mpi, ignoring.])])],
            [])

# use mpi_compiler_suite as input for process_arguments.awk
cat ${ac_psc_compilers_mpi} > mpi_compiler_suite

# find suitable defaults if not already set by platform detection or
# configure arguments. Note that we can't source
# ${ac_psc_compilers_mpi} directly as it may contain lines like
# 'MPIF77=mpiifort -fc=${F77}' which are not valid shell code. Adding
# quotes like in 'MPIF77="mpiifort -fc=${F77}"' would solve the
# problem here but cause headaches using AC_CONFIG_SUBDIR_CUSTOM
$AWK '{print $[]1}' ${ac_psc_compilers_mpi} | grep MPI > mpi_compiler_suite_to_source
. ./mpi_compiler_suite_to_source

if test "x${MPICC}" = "x"; then
	dnl AC_MSG_WARN([MPI flavour was not specified, environment will be checked for MPICC])
    AC_CHECK_PROGS(MPICC, mpicc hcc mpxlc_r mpxlc mpcc cmpicc mpiicc, $CC)
    if test "x${MPICC}" = "x"; then
	AC_MSG_ERROR([MPICC not found.])
	exit -1
    fi
    echo "MPICC=${MPICC}" >> mpi_compiler_suite
fi

if test "x${MPICXX}" = "x"; then
	dnl AC_MSG_WARN([MPI flavour was not specified, environment will be checked for MPICXX])
    AC_CHECK_PROGS(MPICXX, mpic++ mpicxx mpiCC hcp mpxlC_r mpxlC mpCC cmpic++ mpiicpc, $CXX)
    if test "x${MPICXX}" = "x"; then
	AC_MSG_ERROR([MPICXX not found.])
	exit -1
    fi
    echo "MPICXX=${MPICXX}" >> mpi_compiler_suite
fi

if test "x${MPIF77}" = "x"; then
	dnl AC_MSG_WARN([MPI flavour was not specified, environment will be checked for MPIF77])
    AC_CHECK_PROGS(MPIF77, mpif77 hf77 mpxlf_r mpxlf mpf77 cmpifc mpiifort, $F77)
    if test "x${MPIF77}" = "x"; then
	AC_MSG_ERROR([MPIF77 not found.])
	exit -1
    fi
    echo "MPIF77=${MPIF77}" >> mpi_compiler_suite
fi

if test "x${MPIFC}" = "x"; then
	dnl AC_MSG_WARN([MPI flavour was not specified, environment will be checked for MPIFC])
    AC_CHECK_PROGS(MPIFC, mpif90 mpxlf95_r mpxlf90_r mpxlf95 mpxlf90 mpf90 cmpif90c mpiifort, $FC)
    if test "x${MPIFC}" = "x"; then
	AC_MSG_ERROR([MPIFC not found.])
	exit -1
    fi
    echo "MPIFC=${MPIFC}"   >> mpi_compiler_suite
fi

])


AC_DEFUN([AC_PSC_PRECIOUS_VARS_MPI],
[
AC_ARG_VAR(MPICC,[MPI C compiler command])
AC_ARG_VAR(MPICXX,[MPI C++ compiler command])
AC_ARG_VAR(MPIF77,[MPI Fortran 77 compiler command])
AC_ARG_VAR(MPIFC,[MPI Fortran compiler command])
AC_ARG_VAR(MPI_CPPFLAGS, [MPI (Objective) C/C++ preprocessor flags, e.g. -I<include dir> if you have headers in a nonstandard directory <include dir>])
AC_ARG_VAR(MPI_CFLAGS, [MPI C compiler flags])
AC_ARG_VAR(MPI_CXXFLAGS, [MPI C++ compiler flags])
AC_ARG_VAR(MPI_FFLAGS, [MPI Fortran 77 compiler flags])
AC_ARG_VAR(MPI_FCFLAGS, [MPI Fortran compiler flags])
AC_ARG_VAR(MPI_LD_FLAGS, [MPI linker flags, e.g. -L<lib dir> if you have libraries in a nonstandard directory <lib dir>])
AC_ARG_VAR(MPI_LIBS, [MPI libraries to pass to the linker, e.g. -l<library>])
])

AC_DEFUN([AC_PSC_PRECIOUS_VARS_FOR_BUILD],
[
AC_ARG_VAR(CC_FOR_BUILD, [C compiler command for the frontend build])
AC_ARG_VAR(CXX_FOR_BUILD, [C++ compiler command for the frontend build])
AC_ARG_VAR(F77_FOR_BUILD, [Fortran 77 compiler command for the frontend build])
AC_ARG_VAR(FC_FOR_BUILD, [Fortran compiler command for the frontend build])
AC_ARG_VAR(CPPFLAGS_FOR_BUILD, [(Objective) C/C++ preprocessor flags for the frontend build, e.g. -I<include dir> if you have headers in a nonstandard directory <include dir>])
AC_ARG_VAR(CFLAGS_FOR_BUILD, [C compiler flags for the frontend build])
AC_ARG_VAR(CXXFLAGS_FOR_BUILD, [C++ compiler flags for the frontend build])
AC_ARG_VAR(FFLAGS_FOR_BUILD, [Fortran 77 compiler flags for the frontend build])
AC_ARG_VAR(FCFLAGS_FOR_BUILD, [Fortran compiler flags for the frontend build])
AC_ARG_VAR(LD_FLAGS_FOR_BUILD, [linker flags for the frontend build, e.g. -L<lib dir> if you have libraries in a nonstandard directory <lib dir>])
AC_ARG_VAR(LIBS_FOR_BUILD, [libraries to pass to the linker for the frontend build, e.g. -l<library>])
])
