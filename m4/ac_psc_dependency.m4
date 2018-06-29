AC_DEFUN([AC_PSC_DEPENDENCY], [
  dnl 1 : Name of the library
  dnl 2 : Lower case version of the name
  dnl 3 : Upper case version of the name
  dnl 4 : Library header to test with
  dnl 5 : Name for library load on check (prepended by -l)
  dnl 6 : LDFLAGS
  dnl 7 : required version
  dnl 8 : mandatory dependency (yes or no)
  dnl 9 : requires a link option in psc_instrument (yes or no)
  dnl 10: perform library test with symbol 'main'

  AC_MSG_NOTICE(["-----------------------------------------------------------------------------------------------------"])
  AC_MSG_NOTICE([Checking for $1; mandatory dependency: $8 ...])

  if test "$8" = "no"; then
    AC_ARG_ENABLE([$2], AS_HELP_STRING([--enable-$2], [Enable the use of $1. Options: yes no (disabled by default)]))
  fi

  dnl header options and check
  AC_ARG_WITH([$2-include],
              [AS_HELP_STRING([--with-$2-include=<path-to-$2-include-directory>],
                              [Location of the $1 headers (if not following GNU conventions, i.e. /usr/local/include).])],
                              [psc_$2_include_directory="${withval}"], [psc_$2_include_directory=""])

  psc_$2_cppflags=""
  if test "x${psc_$2_include_directory}" != "x"; then
    psc_$2_cppflags="-I${psc_$2_include_directory}"
  fi

  psc_$2_header_check_pass="no"
  AC_LANG_PUSH([C++])
  cppflags_save="$CPPFLAGS"
  CPPFLAGS="$psc_$2_cppflags $CPPFLAGS"
  AC_CHECK_HEADER([$4], [psc_$2_header_check_pass="yes"], [psc_$2_header_check_pass="no"])
  CPPFLAGS="$cppflags_save"
  AC_LANG_POP([C++])

  if [[ "x${psc_$2_header_check_pass}" = "xno" ]] && [[ "$8" = "yes" ]] ; then
    AC_MSG_ERROR([required $1 ($7) headers missing.])
  fi

  dnl library options and check
  AC_ARG_WITH([$2-lib],
              [AS_HELP_STRING([--with-$2-lib=<path-to-$1-libraries-directory>],
                              [Location of the $1 libraries (if not following GNU conventions, i.e. /usr/local/lib).])],
              [psc_$2_libraries_directory="${withval}"],
              [psc_$2_libraries_directory=""])

  psc_$2_ldflags="$6"
  if test "x${psc_$2_libraries_directory}" != "x"; then
    psc_$2_ldflags="-L${psc_$2_libraries_directory} ${psc_$2_ldflags} $LDFLAGS"
  fi

  psc_$2_library_check_pass="yes"
  if test "$10" = "yes"; then
    AC_LANG_PUSH([C++])
    ldflags_save="$LDFLAGS"
    LDFLAGS="${psc_$2_ldflags} $LDFLAGS"
    AC_CHECK_LIB([$5], main, [psc_$2_library_check_pass="yes"], [psc_$2_library_check_pass="no"])
    LDFLAGS="$ldflags_save"
    AC_LANG_POP([C++])
  fi

  if [[ "x${psc_$2_library_check_pass}" = "xno" ]] && [[ "$8" = "yes" ]] ; then
    AC_MSG_ERROR([required $1 ($7) library missing.])
  fi

  dnl checking if both header and library checks passed
  psc_$2_available="no"
  if [[ "x${psc_$2_header_check_pass}" = "xyes" ]] && [[ "x${psc_$2_library_check_pass}" = "xyes" ]] ; then
    psc_$2_available="yes"
  fi

  dnl if optional, check if the user enabled it and is available
  psc_$2_enabled="no"

  dnl file processing based on results
  AC_SUBST([PSC_$3_CPPFLAGS], [${psc_$2_cppflags}])
  AC_SUBST([PSC_$3_LDFLAGS], [${psc_$2_ldflags}])

  if test x"${enable_$2}" = xyes; then
    if test x"${psc_$2_available}" = xyes; then
      psc_$2_enabled="yes"
      AC_DEFINE([PSC_$3_ENABLED], [1], [Defined when $1 is available and enabled by the user.])
    else
      AC_MSG_ERROR([optional dependency $1 enabled by the user but not available in the system.])
    fi
  else
    if test "$8" = "no"; then
      dnl file processing based on results
      AC_SUBST([PSC_$3_CPPFLAGS], [""])
      AC_SUBST([PSC_$3_LDFLAGS], [""])
    fi
  fi

  dnl conditionals for Makefire.am files
  AM_CONDITIONAL([PSC_$3_AVAILABLE], [test "x${psc_$2_available}" = "xyes"])
  AM_CONDITIONAL([PSC_$3_ENABLED], [test "x${psc_$2_enabled}" = "xyes"])

  dnl summary output
  AC_PSC_SUMMARY([---------------------------------------------------------------], [])
  if test "$8" = "yes"; then
    AC_PSC_SUMMARY([$1          ], [mandatory])
  else
    AC_PSC_SUMMARY([$1 available], [${psc_$2_available}])
    AC_PSC_SUMMARY([$1 enabled  ], [${psc_$2_enabled}])
  fi
  AC_PSC_SUMMARY([$1 CPPFLAGS ], [${psc_$2_cppflags}])
  AC_PSC_SUMMARY([$1 LDFLAGS  ], [${psc_$2_ldflags}])

])
