## -*- mode: autoconf -*-
#
# The macros AC_PSC_SUMMARY is based on
# AC_SCOREP_SUMMARY http://www.score-p.org. AC_SCOREP_SUMMARY came with following license:
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

AC_DEFUN([AC_PSC_SUMMARY_INIT], [AS_ECHO(["$1:"]) >config.summary])

AC_DEFUN([AC_PSC_SUMMARY_SECTION],
         [AS_IF([test ! -f config.summary],
         [AC_MSG_WARN([PSC_SUMMARY_SECTION used without calling PSC_SUMMARY_INIT.])])
  AS_ECHO([" $1"]) >>config.summary
])

AC_DEFUN([AC_PSC_SUMMARY],
         [AS_IF([test ! -f config.summary],
         [AC_MSG_WARN([PSC_SUMMARY used without calling PSC_SUMMARY_INIT.])])
  AS_ECHO(["  $1: $2"]) >>config.summary
])

# additional output if ./configure is called with --verbose
AC_DEFUN([AC_PSC_SUMMARY_VERBOSE],
         [AS_IF([test ! -f config.summary],
         [AC_MSG_WARN([PSC_SUMMARY_VERBOSE used without calling PSC_SUMMARY_INIT.])])
  AS_IF([test "x${verbose}" = "xyes"], [AS_ECHO(["   $1: $2"]) >>config.summary])
])

# should be called after AC_OUTPUT
AC_DEFUN([AC_PSC_SUMMARY_COLLECT],
         [AS_IF([test -f config.summary],
         [cat config.summary])
  find */ -depth -name config.summary | xargs cat
])
