
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

dnl these two lines should let u find most java installations
  java_dirs="/usr /usr/local /usr/lib/j2sdk1.4-sun /usr/lib/jvm/java /System/Library/Frameworks/JavaVM.framework/Versions/Current /opt /mingw"
  java_inc_dirs="include include/libgcj Headers"
  
  if test "x$with_java" != 'x'
  then
    if test -d "$with_java"
    then
      JAVA_HOME="$with_java"
      for j in $java_inc_dirs
      do
        if test -r "$JAVA_HOME/$j/jni.h"; then
	  java_inc_dir="$j"
	  break
        fi
      done
    else
      AC_MSG_WARN([Sorry, $with_java does not exist, checking usual places])
                  with_java=''
    fi
  fi

dnl now find the java dirs

  if test "x$JAVA_HOME" = 'x'
  then
    for i in $java_dirs;
    do
      for j in $java_inc_dirs
      do
        if test -r "$i/$j/jni.h"; then
          JAVA_HOME="$i"
	  java_inc_dir="$j"
	  break
        fi
      done
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
    case "${target_os}" in
      linux*)
        java_extra_inc=linux
        ;;
      darwin*)
        java_extra_inc=darwin
        ;;
      *mingw32*)
        java_extra_inc=win32
        ;;
      *cygwin*)
        java_extra_inc=win32
        ;;
    esac
    CPPFLAGS="$CPPFLAGS -I$JAVA_HOME/$java_inc_dir -I$JAVA_HOME/$java_inc_dir/$java_extra_inc"
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
        JAVA_FLAGS=
      else
        LIB_JAVA="-L$JAVA_HOME/lib"
        JAVA_FLAGS="-I$JAVA_HOME/$java_inc_dir -I$JAVA_HOME/$java_inc_dir/$java_extra_inc -DHasJava"
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

    
