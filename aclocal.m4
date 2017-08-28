AC_DEFUN([CHECK_SNET],
[
    AC_MSG_CHECKING(for snet)
    snetdir="libsnet"
    AC_ARG_WITH(snet,
	    AC_HELP_STRING([--with-snet=DIR], [path to snet]),
	    snetdir="$withval")
    if test -f "$snetdir/snet.h"; then
	found_snet="yes";
	CPPFLAGS="$CPPFLAGS -I$snetdir";
    fi
    if test x_$found_snet != x_yes; then
	AC_MSG_ERROR(cannot find snet libraries)
    else
	LIBS="$LIBS -lsnet";
	LDFLAGS="$LDFLAGS -L$snetdir";
    fi
    AC_MSG_RESULT(yes)
])

AC_DEFUN([CHECK_SSL],
[
    AC_MSG_CHECKING(for ssl)
    ssldirs="/usr/local/openssl /usr/lib/openssl /usr/openssl \
	    /usr/local/ssl /usr/lib/ssl /usr/ssl \
	    /usr/pkg /usr/local /usr"
    AC_ARG_WITH(ssl,
	    AC_HELP_STRING([--with-ssl=DIR], [path to ssl]),
	    ssldirs="$withval")
    for dir in $ssldirs; do
	ssldir="$dir"
	if test -f "$dir/include/openssl/ssl.h"; then
	    found_ssl="yes";
	    CPPFLAGS="$CPPFLAGS -I$ssldir/include";
	    break;
	fi
	if test -f "$dir/include/ssl.h"; then
	    found_ssl="yes";
	    CPPFLAGS="$CPPFLAGS -I$ssldir/include";
	    break
	fi
    done
    if test x_$found_ssl != x_yes; then
	AC_MSG_ERROR(cannot find ssl libraries)
    else
	AC_DEFINE(HAVE_LIBSSL)
	LIBS="$LIBS -lssl -lcrypto";
	LDFLAGS="$LDFLAGS -L$ssldir/lib";
    fi
    AC_MSG_RESULT(yes)
])

AC_DEFUN([CHECK_ZLIB],
[
    AC_MSG_CHECKING(for zlib)
    zlibdirs="/usr /usr/local"
	withval=""
    AC_ARG_WITH(zlib,
            [AC_HELP_STRING([--with-zlib=DIR], [path to zlib])],
            [])
    if test x_$withval != x_no; then
	if test x_$withval != x_yes -a \! -z "$withval"; then
		zlibdirs="$withval"
	fi
	for dir in $zlibdirs; do
	    zlibdir="$dir"
	    if test -f "$dir/include/zlib.h"; then
			found_zlib="yes";
			break;
		fi
	done
	if test x_$found_zlib = x_yes; then
		if test "$dir" != "/usr"; then
			CPPFLAGS="$CPPFLAGS -I$zlibdir/include";
			LDFLAGS="$LDFLAGS -L$zlibdir/lib";
 	   		ac_configure_args="$ac_configure_args --with-zlib=$dir";
	    fi
		LIBS="$LIBS -lz";
	    AC_DEFINE(HAVE_ZLIB)
	    AC_MSG_RESULT(yes)
	else
	    AC_MSG_RESULT(no)
	fi
    else
 	   ac_configure_args="$ac_configure_args --with-zlib=no";
		AC_MSG_RESULT(no)
    fi
])

AC_DEFUN([SET_NO_SASL],
[
    ac_configure_args="$ac_configure_args --with-sasl=no";
    AC_MSG_RESULT(Disabled SASL)
])

AC_DEFUN([CHECK_UNIVERSAL_BINARIES],
[
    AC_ARG_ENABLE(universal_binaries,
        AC_HELP_STRING([--enable-universal-binaries], [build universal binaries (default=no)]),
        ,[enable_universal_binaries=no])
    if test "${enable_universal_binaries}" = "yes"; then
        case "${host_os}" in
	  darwin8*)
	    macosx_sdk="MacOSX10.4u.sdk"
	    arches="-arch i386 -arch ppc"
	    ;;

	  darwin9*|darwin10*|darwin11*)
	    dep_target="-mmacosx-version-min=10.4"
	    macosx_sdk="MacOSX10.5.sdk"
	    arches="-arch i386 -arch x86_64 -arch ppc -arch ppc64"
	    LDFLAGS="$LDFLAGS -L/Developer/SDKs/$macosx_sdk/usr/lib"
	    ;;

	  *)
            AC_MSG_ERROR([Building universal binaries on ${host_os} is not supported])
	    ;;
	esac

	echo ===========================================================
	echo Setting up universal binaries for ${host_os}
	echo ===========================================================
	OPTOPTS="$OPTOPTS -isysroot /Developer/SDKs/$macosx_sdk $dep_target $arches"
    fi
])

AC_DEFUN([CHECK_XATTR],
[
    AC_ARG_ENABLE(xattr, AC_HELP_STRING([--enable-xattr], [enable extended attribute support (default=no)]), [xattr_support=yes], [xattr_support=no])

    AC_MSG_CHECKING(whether to enable extended attribute support)
    if test x"${xattr_support}" = x"yes"; then
	AC_MSG_RESULT( yes)
	AC_DEFINE(ENABLE_XATTR)
	AC_CHECK_HEADERS([sys/xattr.h], [have_sys_xattr_h=yes],
			[have_sys_xattr_h=no], [])
	AC_CHECK_HEADERS([sys/extattr.h], [have_sys_extattr_h=yes],
			[have_sys_extattr_h=no], [])
	if test x"$have_sys_xattr_h" = x"yes"; then
	    AC_CHECK_FUNCS([lgetxattr getxattr lsetxattr setxattr lremovexattr removexattr])
	elif test x"$have_sys_extattr_h" = x"yes"; then
	    AC_CHECK_FUNCS([extattr_get_file extattr_get_link])
	fi
    else
	AC_MSG_RESULT( no)
    fi
])
