AC_DEFUN([AC_PSC_CONFIG], [

  AC_MSG_NOTICE([Setting up configuration data ...])

  AC_DEFINE_UNQUOTED([PERISCOPE_INSTALL_DIRECTORY], ["$prefix"], [Periscope install's location.])

  AC_DEFINE_UNQUOTED([PERISCOPE_DOCUMENTATION_DIRECTORY], ["${prefix}/docs"], [Periscope documentation directory.])

  AC_DEFINE_UNQUOTED([PERISCOPE_SOURCE_DIRECTORY], ["${srcdir}"], [Periscope source code location.])
	AC_SUBST([PERISCOPE_SOURCE_DIRECTORY_IN_SCRIPTS], ["${srcdir}"])
	AC_SUBST([PERISCOPE_FULL_PATH_SOURCE_DIRECTORY_IN_SCRIPTS], ["${PWD}/${srcdir}"])

  AC_DEFINE_UNQUOTED([PERISCOPE_PLUGINS_DIRECTORY], ["${prefix}/plugins"], [Periscope plugins directory.])

  AC_SUBST([plugindir], ['${prefix}/plugins'])

  AC_DEFINE_UNQUOTED([PERISCOPE_SEARCH_ALGORITHMS_DIRECTORY], ["${prefix}/search"], [Periscope search algorithms directory.])
  AC_SUBST([searchdir], ['${prefix}/search'])

  AC_SUBST([global_compiler_flags], ['-shared-libgcc -fexceptions'])

  AC_SUBST([autotune_plugin_base_cxxflags], ['${global_compiler_flags} \
                                              -std=c++14 \
                                              ${PSC_ACE_CPPFLAGS} \
                                              ${PSC_BOOST_CPPFLAGS} \
                                              -I$(top_srcdir)/autotune/datamodel/include \
                                              -I$(top_srcdir)/autotune/plugins/include \
                                              -I$(top_srcdir)/autotune/searchalgorithms/include \
                                              -I$(top_srcdir)/autotune/services/include \
                                              -I$(top_srcdir)/quality_expressions/include \
                                              -I$(top_srcdir)/aagent/include \
                                              -I$(top_srcdir)/frontend/include \
                                              -I$(top_srcdir)/util/include \
                                              -I$(top_srcdir)/registry/include'])

  AC_SUBST([autotune_plugin_base_ldflags], [${PSC_BOOST_LDFLAGS}])

  AC_SUBST([autotune_plugin_base_libadd], ['$(top_builddir)/autotune/datamodel/src/libdatamodel.la\
                                            $(top_builddir)/autotune/services/src/libservices.la\
  '])

  AC_SUBST([autotune_search_base_cxxflags], ['${global_compiler_flags} \
                                              -std=c++14 \
                                              ${PSC_ACE_CPPFLAGS} \
                                              ${PSC_BOOST_CPPFLAGS} \
                                              -I$(top_srcdir)/autotune/datamodel/include \
                                              -I$(top_srcdir)/autotune/plugins/include \
                                              -I$(top_srcdir)/autotune/searchalgorithms/include \
                                              -I$(top_srcdir)/autotune/services/include \
                                              -I$(top_srcdir)/quality_expressions/include \
                                              -I$(top_srcdir)/aagent/include \
                                              -I$(top_srcdir)/frontend/include \
                                              -I$(top_srcdir)/util/include \
                                              -I$(top_srcdir)/registry/include'])
  AC_SUBST([autotune_search_base_ldflags], [])
  AC_SUBST([autotune_search_base_libadd], ['$(top_builddir)/autotune/datamodel/src/libautotune.la'])

  AC_SUBST([autotune_test_base_cxxflags], ['${global_compiler_flags} \
                                            -std=c++14 \
                                            -rdynamic -Wl,-export-dynamic \
                                            ${PSC_ACE_CPPFLAGS} \
                                            ${PSC_BOOST_CPPFLAGS} \
                                            ${PSC_SQLITE3_CPPFLAGS} \
                                            -D_REENTRANT \
                                            -I$(top_srcdir)/aagent/include \
                                            -I$(top_srcdir)/frontend/include \
                                            -I$(top_srcdir)/autotune/datamodel/include \
                                            -I$(top_srcdir)/autotune/plugins/include \
                                            -I$(top_srcdir)/autotune/searchalgorithms/include \
                                            -I$(top_srcdir)/autotune/services/include \
                                            -I$(top_srcdir)/util/include \
                                            -I$(top_srcdir)/quality_expressions/include \
                                            -I$(top_srcdir)/test/autotune/fixtures \
                                            -I$(top_srcdir)/registry/include'])

  AC_SUBST([autotune_test_base_ldflags], [])

  AC_SUBST([autotune_test_base_ldadd], ['-L./ libtest.la \
                                         libpscreg.a \
                                         libpscutil.a \
                                         libstrategies.a \
                                         libpscproperties.a \
                                         libdatamodel.la \
                                         libservices.la \
                                         ${PSC_ACE_LDFLAGS} ${PSC_ACE_LIBS} \
                                         ${PSC_BOOST_LDFLAGS} ${PSC_BOOST_LIBS} \
                                         ${PSC_SQLITE3_LDFLAGS} ${PSC_SQLITE3_LIBS}'])

  AC_SUBST([autotune_test_base_dependencies], ['libtest.la \
                                                libpscreg.a \
                                                libpscutil.a \
                                                libstrategies.a \
                                                libpscproperties.a \
                                                libdatamodel.la \
                                                libservices.la'])

  AC_ARG_ENABLE([developer-mode], AS_HELP_STRING([--enable-developer-mode], [Enable features that are targeted for developers (including the tutorials).]))
  developer_mode="no"
	if test x"${enable_developer_mode}" = xyes; then
		echo "Developer mode enabled."
		developer_mode="yes"
  else
    echo "Disabled developer mode."
	fi
  AM_CONDITIONAL([DEVELOPER_MODE], [test "x${developer_mode}" = "xyes"])
	AC_PSC_SUMMARY([Developer mode: ],[${developer_mode}])

	if test "$GIT" != ":"; then
		if test -d ".git/"; then
			git rev-list --count master > REVISION
		fi
	fi

	AC_SUBST([REVISION],m4_esyscmd([tr -d '\n' <REVISION]))
  AC_SUBST([PERISCOPE_VERSION],m4_esyscmd([tr -d '\n' <PERISCOPE_VERSION]))
  AC_SUBST([PATHWAY_VERSION],m4_esyscmd([tr -d '\n' <PATHWAY_VERSION]))

	AC_PSC_PLUGIN_VERSION([Compiler Flags Selection (CFS)],[compilerflags],[COMPILERFLAGS])
	AC_PSC_PLUGIN_VERSION([DVFS],[dvfs],[DVFS])
	AC_PSC_PLUGIN_VERSION([MasterWorker],[masterworker],[MASTERWORKER])
	AC_PSC_PLUGIN_VERSION([MPI Parameters],[mpiparameters],[MPIPARAMETERS])
	AC_PSC_PLUGIN_VERSION([Pipeline Pattern],[pipeline],[PIPELINE])

])

