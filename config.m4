dnl $Id$
dnl config.m4 for extension simplate

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(simplate, for simplate support,
dnl Make sure that the comment is aligned:
dnl [  --with-simplate             Include simplate support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(simplate, whether to enable simplate support,
dnl Make sure that the comment is aligned:
[  --enable-simplate           Enable simplate support])

if test "$PHP_SIMPLATE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-simplate -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/simplate.h"  # you most likely want to change this
  dnl if test -r $PHP_SIMPLATE/$SEARCH_FOR; then # path given as parameter
  dnl   SIMPLATE_DIR=$PHP_SIMPLATE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for simplate files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SIMPLATE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SIMPLATE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the simplate distribution])
  dnl fi

  dnl # --with-simplate -> add include path
  dnl PHP_ADD_INCLUDE($SIMPLATE_DIR/include)

  dnl # --with-simplate -> check for lib and symbol presence
  dnl LIBNAME=simplate # you may want to change this
  dnl LIBSYMBOL=simplate # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SIMPLATE_DIR/lib, SIMPLATE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SIMPLATELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong simplate lib version or lib not found])
  dnl ],[
  dnl   -L$SIMPLATE_DIR/lib -lm -ldl
  dnl ])
  dnl
  PHP_SUBST(SIMPLATE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++,1,SIMPLATE_SHARED_LIBADD)
  PHP_REQUIRE_CXX()
  PHP_NEW_EXTENSION(simplate, simplate.cpp, $ext_shared)
fi
