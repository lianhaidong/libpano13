
dnl Usage:
dnl AX_CHECK_JAVA
dnl Test for java, and defines
dnl - JAVA_CFLAGS (compiler flags)
dnl - LIB_JAVA (linker flags, stripping and path)
dnl prerequisites:

AC_DEFUN([AX_CHECK_JAVA],
[
AC_ARG_WITH([java],
            AC_HELP_STRING([  --with-java=PFX], [prefix where 'java' is installed.]),
            [with_java_prefix=$withval], 
	    [with_java_prefix=${JAVA_INSTALL_PATH:-/usr/java/j2sdk1.4.1_01}])
have_java='no'
LIB_JAVA=''
JAVA_FLAGS=''
JAVA_HOME=''
if test "x$with_java" != 'xno'
then
  AC_MSG_CHECKING([for JAVA installation at ${with_java}])
  AC_MSG_RESULT()
  if test "x$with_java" != 'x'
  then
    if test -d "$with_java"
    then
      JAVA_HOME="$with_java"
    else
      AC_MSG_WARN([Sorry, $with_java does not exist, checking usual places])
                  with_java=''
    fi
  fi
case "${target_os}" in
  "")
    java_inc_dir=include/libgcj
    ;;
  linux*)
    java_inc_dir=include/libgcj
    ;;
  darwin*)
    java_inc_dir=include/libgcj
    ;;
  *mingw32*)
    java_inc_dir=include
    ;;
  *)
    java_inc_dir=include/libgcj
    ;;
esac


  if test "x$JAVA_HOME" = 'x'
  then
    java_dirs="/usr /usr/local /opt /mingw"
    for i in $java_dirs;
    do
       if test -r "$i/$java_inc_dir/jni.h"; then
         JAVA_HOME="$i"
	 break
       fi
    done
    if test "x$JAVA_HOME" != 'x'
    then
      AC_MSG_NOTICE([java home set to $JAVA_HOME])
    else
      AC_MSG_NOTICE([cannot find the java directory, assuming it is specified in CFLAGS])
    fi
  fi

  failed=0;
  passed=0;
  JAVA_OLD_CPPFLAGS=$CPPFLAGS
  if test "x$JAVA_HOME" != 'x'
  then
    CPPFLAGS="$CPPFLAGS -I$JAVA_HOME/$java_inc_dir"
  fi
  AC_LANG_SAVE
  AC_LANG_C
  AC_CHECK_HEADER(jni.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_LANG_RESTORE
  CPPFLAGS="$JAVA_OLD_CPPFLAGS"

  AC_MSG_CHECKING(if JAVA package is complete)
  if test $passed -gt 0
  then
    if test $failed -gt 0
    then
      AC_MSG_RESULT(no -- some components failed test)
      have_java='no (failed tests)'
    else
      if test "x$JAVA_HOME" = 'x'
      then
        JAVA_FLAGS="-DHasJava"
      else
        LIB_JAVA="-L$PNG_HOME/lib -lpng"
        JAVA_FLAGS="-I$JAVA_HOME/$java_inc_dir -DHasJava"
      fi
      AC_DEFINE(HasJava,1,Define if you have Java)
      AC_MSG_RESULT(yes)
      have_java='yes'
    fi
  else
    AC_MSG_RESULT(no)
  fi
fi
AM_CONDITIONAL(HasJava, test "x$have_java" = 'xyes')
AC_SUBST(LIB_JAVA)
AC_SUBST(JAVA_FLAGS)
])

    
