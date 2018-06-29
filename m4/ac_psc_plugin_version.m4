AC_DEFUN([AC_PSC_PLUGIN_VERSION], [
	dnl 1: Plugin name in a regular sting
	dnl 2: Directory and library name of the plugin
	dnl 3: Prefix for the header definitions (all capital leters)
	
	$2_version=m4_esyscmd([(cat autotune/plugins/$2/VERSION_MAJOR; echo "."; cat autotune/plugins/$2/VERSION_MINOR) | tr -d '\n'])
	AC_SUBST([$3_VERSION],${$2_version})
	AC_DEFINE_UNQUOTED([$3_VERSION],${$2_version}, ["$1 plugin version"])
	$2_version_major=m4_esyscmd([tr -d '\n' <autotune/plugins/$2/VERSION_MAJOR])
  AC_SUBST([$3_VERSION_MAJOR], ${$2_version_major})
  AC_DEFINE_UNQUOTED([$3_VERSION_MAJOR], ${$2_version_major}, ["$1 plugin major version number"])
	$2_version_minor=m4_esyscmd([tr -d '\n' <autotune/plugins/$2/VERSION_MINOR])
  AC_SUBST([$3_VERSION_MINOR], ${$2_version_minor})
  AC_DEFINE_UNQUOTED([$3_VERSION_MINOR], ${$2_version_minor}, ["$1 plugin minor version number"])

	if test "$GIT" != ":"; then
		if test -d ".git/"; then
			git rev-list --count master autotune/plugins/$2/ > autotune/plugins/$2/REVISION
		fi
	fi

	$2_revision=m4_esyscmd([tr -d '\n' <autotune/plugins/$2/REVISION])
	AC_SUBST([$3_REVISION],${$2_revision})
  AC_DEFINE_UNQUOTED([$3_REVISION],${$2_revision}, ["$1 plugin revision number"])
])
