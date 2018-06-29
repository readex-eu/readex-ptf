dnl initial information about the project
AC_INIT([periscope],m4_esyscmd([tr -d '\n' <VERSION]),[periscope@lrr.in.tum.de])

dnl automake initialisation 
AM_INIT_AUTOMAKE([1.12])
AM_SILENT_RULES([yes])

AC_PSC_SUMMARY_INIT([Periscope configuration summary])

dnl check if the source folder is correct
AC_CONFIG_SRCDIR([aagent/src/analysisagent.cc])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

dnl setting extra locations for libraries and headers
AMCFLAGS="$CFLAGS -I/usr/local/include"
AMCXXFLAGS="$CXXFLAGS -I/usr/local/include"
AMLDFLAGS="$LDFLAGS -L/usr/local/lib"

AC_PSC_DETECT_PLATFORMS
AC_MSG_NOTICE([$ac_psc_compilers_mpi])
AC_PSC_WITH_COMPILER_SUITE
AC_MSG_NOTICE([Checking for MPI ...])
AC_PSC_MPI_WORKING
AC_PSC_WITH_MPI_COMPILER_SUITE

dnl check for C++ preprocessor and compiler and the library compiler if not yet detected by platform checks

AC_CHECK_PROGS(YACC, 'bison -y' byacc yacc, :)
if test "$YACC" = ":"; then
  AC_MSG_ERROR([ERROR: Required program YACC is missing.])
else
  AC_SUBST([BISON_VERSION],[`${YACC} --version | egrep '[[0-9.-]]+[[0-9]]$' | sed -e s/'.* \([[0-9.-]]*[[0-9]]\)$'/'\1'/ -e s/[[.-]]//g`])
  AC_MSG_NOTICE([BISON_VERSION=${BISON_VERSION}])
fi

AC_CHECK_PROGS(LEX, flex lex, :)
if test "$LEX" = ":"; then
  AC_MSG_ERROR([ERROR: Required program LEX is missing.])
fi

AC_CHECK_PROGS(GIT, git, :)
if test "$GIT" = ":"; then
  AC_MSG_NOTICE([Git command not available.])
fi

AC_PROG_YACC
AM_PROG_LEX

AC_PROG_CXXCPP
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_PROG_F77
AC_PROG_FC
AC_OPENMP

dnl initialize libtool
LT_INIT

dnl use the C++ compiler for the following checks
AC_LANG([C++])

dnl custom environment checks; using the new modular macro in ac_psc_dependency.m4
AC_PSC_LIBACE
AC_PSC_LIBBOOST
AC_PSC_LIBCUBE
AC_PSC_LIBCEREAL
AC_PSC_LIBTDA

dnl custom environment checks; old custom macros
AC_PSC_LIBSQLITE3
AC_PSC_LIBSCOREP

AC_PSC_PRECIOUS_VARS_MPI

AC_PSC_SUMMARY([---------------------------------------------------------------], [])

AC_PSC_STARTER
AC_PSC_PTHREADS
AC_PSC_CONFIG

dnl enable Doxygen HTML and PDF; will be disabled automatically by the macro when not available
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(OFF)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(ON)
DX_INIT_DOXYGEN([Periscope], [docs/doxygen/doxygen.config], [docs])

AC_PSC_SUMMARY([---------------------------------------------------------------], [])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

dnl distribute additional compiler and linker flags
AC_SUBST([AM_CXXFLAGS], [-std=c++14])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_FFLAGS])
AC_SUBST([AM_LDFLAGS])
AC_SUBST([lib_LIBRARIES])
AC_SUBST([lib_LTLIBRARIES])
AC_SUBST([bin_PROGRAMS])
AC_SUBST([TESTS])
AC_SUBST([check_PROGRAMS])

dnl files to generate via autotools (.am or .in source files)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([scripts/psc_generate_bare_plugin_from_skeleton])
AC_CONFIG_FILES([scripts/psc_source_code_directory])
AC_CONFIG_FILES([scripts/psc_result])
AC_CONFIG_FILES([REVISION])
AC_CONFIG_FILES([RELEASE])
AC_CONFIG_FILES([templates/psc_bashrc])
AC_CONFIG_FILES([templates/psc_cshrc])

AC_PSC_SUMMARY([Platform], [$ac_psc_platform])
AC_PSC_SUMMARY([Platform], [CC=${CC}])
AC_PSC_SUMMARY([Platform], [CXX=${CXX}])
AC_PSC_SUMMARY([Platform], [F77=${F77}])
AC_PSC_SUMMARY([Platform], [FC=${FC}])
AC_PSC_SUMMARY([Platform], [MPICC=${MPICC}])
AC_PSC_SUMMARY([Platform], [MPICXX=${MPICXX}])
AC_PSC_SUMMARY([Platform], [MPIF77=${MPIF77}])
AC_PSC_SUMMARY([Platform], [MPIFC=${MPIFC}])

AC_PSC_SUMMARY([---------------------------------------------------------------], [])

dnl generate the final Makefile etc.
AC_OUTPUT

cat config.summary
