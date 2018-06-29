AC_DEFUN([AC_PSC_PTHREADS], [

  test_success=1
  AC_MSG_NOTICE([Checking for PTHREADS ...])
  AC_CHECK_HEADER(pthread.h, [], [test_success=0])
  if test $test_success == 0; then
    AC_MSG_NOTICE([ERROR: Required library PTHREADS is missing.])
    exit -1
  fi

])