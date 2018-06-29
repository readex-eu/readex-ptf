## -*- mode: autoconf -*-
#
# The macros AC_PSC_SYS_DETECTION is based on
# AC_SCOREP_SYS_DETECTION http://www.score-p.org. AC_SCOREP_SYS_DETECTION came with following license:
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

AC_DEFUN([_AC_PSC_DETECT_LINUX_PLATFORMS],
[
    if test "x${ac_psc_platform}" = "xunknown"; then
        case ${build_os} in
            linux*)
                AS_IF([test "x${build_cpu}" = "xia64"      -a -f /etc/sgi-release],
                          [ac_psc_platform="altix";    ac_psc_cross_compiling="no"],
                      [test "x${build_cpu}" = "xpowerpc64" -a -d /bgl/BlueLight],
                          [ac_psc_platform="bgl";      ac_psc_cross_compiling="yes"],
                      [test "x${build_cpu}" = "xpowerpc64" -a -d /bgsys],
                          [ac_psc_platform="bgp";      ac_psc_cross_compiling="yes"],
                      [test "x${build_cpu}" = "xx86_64"    -a -d /opt/cray/xt-asyncpe],
                          [ac_psc_platform="crayxt";   ac_psc_cross_compiling="yes"],
                      [test "x${build_cpu}" = "xmips64"    -a -d /opt/sicortex],
                          [ac_psc_platform="sicortex"; ac_psc_cross_compiling="yes"],
                      [ac_psc_platform=linux]
                )
            ;;
        esac
    fi
])


AC_DEFUN([_AC_PSC_DETECT_NON_LINUX_PLATFORMS],
[
    if test "x${ac_psc_platform}" = "xunknown"; then
        case ${build_os} in
            sunos* | solaris*)
                ac_psc_platform="sun"
                ac_psc_cross_compiling="no"
                ;;
            darwin*)
                ac_psc_platform="mac"
                ac_psc_cross_compiling="no"
                ;;
            aix*)
                ac_psc_platform="ibm"
                ac_psc_cross_compiling="no"
                ;;
            unicosmp*)
                ac_psc_platform="crayx1"
                ac_psc_cross_compiling="no"
                ;;
            superux*)
                ac_psc_platform="necsx"
                ac_psc_cross_compiling="yes"
                ;;
        esac
    fi
])

# The purpose of platform detection is to provide reasonable default
# compilers, mpi-implementations, OpenMP flags etc.  The user always has the
# possibility to override the defaults by setting environment variables, see
# section "Some influential environment variables" in configure --help.  On
# some systems there may be no reasonable defaults for the mpi-implementation,
# so specify them using --with-mpi=... I think we need to specify one or more
# paths too. Also, on some systems there are different compiler-suites available
# which can be choosen via --with-compiler=(gnu?|intel|sun|ibm|...)
# Have to think this over...

AC_DEFUN([AC_PSC_DETECT_PLATFORMS],
[
    AC_REQUIRE([AC_CANONICAL_BUILD])
    ac_psc_platform="unknown"
    ac_psc_cross_compiling="no"
    ac_psc_platform_detection=""
    ac_psc_platform_detection_given=""

    ac_psc_compilers_frontend=""
    ac_psc_compilers_backend=""
    ac_psc_compilers_mpi=""

    path_to_compiler_files="$srcdir/m4/platforms/"

    if test "x${host_alias}" != "x"; then
        AC_CANONICAL_HOST
        if test "x${build}" != "x${host}"; then
            ac_psc_cross_compiling="yes"
        fi
    fi

    AC_ARG_ENABLE([platform-detection],
                  [AS_HELP_STRING([--enable-platform-detection],
                                  [autodetect platform [yes]])],
                  [ac_psc_platform_detection_given="$enableval"],
                  [AS_IF([test "x${build_alias}" = "x" -a "x${host_alias}" = "x"],
                         [ac_psc_platform_detection="yes"],
                         [ac_psc_platform_detection="no"])])

    if test "x${ac_psc_platform_detection_given}" = "xyes"; then
        if test "x${build_alias}" != "x" -o "x${host_alias}" != "x"; then
            AC_MSG_ERROR([it makes no sense to request for platform detection while providing --host and/or --build.])
        fi
    fi
    if test "x${ac_psc_platform_detection_given}" != "x"; then
        ac_psc_platform_detection="$ac_psc_platform_detection_given"
    fi

    if test "x${ac_psc_platform_detection}" = "xyes"; then
        _AC_PSC_DETECT_LINUX_PLATFORMS
        _AC_PSC_DETECT_NON_LINUX_PLATFORMS
        AC_MSG_CHECKING([for platform])
        if test "x${ac_psc_platform}" = "xunknown"; then
            AC_MSG_RESULT([$ac_psc_platform, please contact <AC_PACKAGE_BUGREPORT> if you encounter any problems.])
        else
            AC_MSG_RESULT([$ac_psc_platform])
            ac_psc_compilers_frontend="${path_to_compiler_files}platform-frontend-${ac_psc_platform}"
            ac_psc_compilers_backend="${path_to_compiler_files}platform-backend-${ac_psc_platform}"
            ac_psc_compilers_mpi="${path_to_compiler_files}platform-mpi-${ac_psc_platform}"
        fi
        AC_MSG_CHECKING([for cross compilation])
        AC_MSG_RESULT([$ac_psc_cross_compiling])
    elif test "x${ac_psc_platform_detection}" = "xno"; then
        AC_MSG_NOTICE([platform detection disabled.])
        AC_MSG_CHECKING([for cross compilation])
        AC_MSG_RESULT([$ac_psc_cross_compiling]) 
        ac_psc_compilers_frontend="${path_to_compiler_files}platform-frontend-user-provided"
        ac_psc_compilers_backend="${path_to_compiler_files}platform-backend-user-provided"
        ac_psc_compilers_mpi="${path_to_compiler_files}platform-mpi-user-provided"
    else
        AC_MSG_ERROR([unknown value for ac_psc_platform_detection: $ac_psc_platform_detection])
    fi
])


AC_DEFUN([AC_PSC_PLATFORM_SETTINGS],
[
    AC_REQUIRE([AC_PSC_DETECT_PLATFORMS])

    AM_CONDITIONAL([PLATFORM_ALTIX],    [test "x${ac_psc_platform}" = "xaltix"])
    AM_CONDITIONAL([PLATFORM_POWER6],   [test "x${ac_psc_platform}" = "xibm" -a "x${build_cpu}" = "xpowerpc"])
    AM_CONDITIONAL([PLATFORM_BGL],      [test "x${ac_psc_platform}" = "xbgl"])
    AM_CONDITIONAL([PLATFORM_BGP],      [test "x${ac_psc_platform}" = "xbgp"])
    AM_CONDITIONAL([PLATFORM_CRAYXT],   [test "x${ac_psc_platform}" = "xcrayxt"])
    AM_CONDITIONAL([PLATFORM_SICORTEX], [test "x${ac_psc_platform}" = "xsicortex"])
    AM_CONDITIONAL([PLATFORM_LINUX],    [test "x${ac_psc_platform}" = "xlinux"])
    AM_CONDITIONAL([PLATFORM_SUN],      [test "x${ac_psc_platform}" = "xsun"])
    AM_CONDITIONAL([PLATFORM_MAC],      [test "x${ac_psc_platform}" = "xmac"])
    AM_CONDITIONAL([PLATFORM_CRAYX1],   [test "x${ac_psc_platform}" = "xcrayx1"])
    AM_CONDITIONAL([PLATFORM_NECSX],    [test "x${ac_psc_platform}" = "xnecsx"])
])
