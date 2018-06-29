AC_DEFUN([AC_PSC_DOCS], [

  AC_MSG_NOTICE([Checking for Documentation ...])

  AC_ARG_ENABLE([docs], AS_HELP_STRING([--enable-docs], [Enable generation of the documentation. Options: yes no (disabled by default)]))

  docs_enabled=no
  has_latex=no
  has_doxygen=no

  if test x"${enable_docs}" = xyes; then
    echo "The generation of the documentation is enabled."
    docs_enabled=yes

    AC_CHECK_PROGS(PDFLATEX, pdflatex, )
    if test "$PDFLATEX" = ""; then
      AC_MSG_WARN([WARNING: Required program PDFLATEX is missing.])
    else
      has_latex=yes
    fi

    AC_CHECK_PROGS(DOXYGEN, doxygen, )
    if test "$DOXYGEN" = ""; then
      AC_MSG_WARN([WARNING: Required program DOXYGEN is missing.])
    else
      has_doxygen=yes
    fi

    #if !${HAVE_DOXYGEN} && !${HAVE_LATEX}; then
    #  psc_have_docs=no
    #else
    #  psc_have_docs=yes
    #fi


    #if test "x${psc_have_docs}" = "xno"; then
    #  AC_MSG_ERROR([ERROR: The generation of the documentation is enabled but required tools are not available.])
    #  exit -1
    #fi
  else
    echo "The generation of the documentation is disabled."
    enable_docs=""
    docs_enabled=no
  fi

	AM_CONDITIONAL([HAVE_PDFLATEX], test x${has_latex} = xyes)
	AM_CONDITIONAL([HAVE_DOXYGEN],  test x${has_doxygen} = xyes)

  AC_SUBST([PSC_DOCS], [${enable_docs}])
  AM_CONDITIONAL([PSC_DOCS_ENABLE], [test x"${docs_enabled}" = xyes ])
  AC_PSC_SUMMARY([DOCS enabled], [${docs_enabled}])
])
