AC_DEFUN([AC_PSC_STARTER], [
  AC_MSG_NOTICE([Checking for starter...])
  AC_ARG_WITH([starter],
              AS_HELP_STRING([--with-starter=(supermuc|slurm)],
                             [Select the application starter.]),
              [enable_starter="${withval}"],
              [AC_MSG_ERROR([No starter selected. (use --with-starter)])])
  AC_CHECK_FILE(["${srcdir}/starterplugin/${enable_starter}/ptf-plugin.cc"], [], [AC_MSG_ERROR([Starter plugin not found!])])
  AC_CONFIG_LINKS([starterplugin/ptf-plugin.cc:starterplugin/${enable_starter}/ptf-plugin.cc])
])
