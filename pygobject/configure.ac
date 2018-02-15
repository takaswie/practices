# my versioning
m4_define([my_major], [0])
m4_define([my_minor], [0])
m4_define([my_micro], [0])
m4_define([my_version],	[my_major.my_minor.my_micro])

# Use -version option for libtool
m4_define([my_current], [m4_eval(my_major * 100 + my_minor)])
m4_define([my_revision],[my_minor])
m4_define([my_age],     [my_minor])
m4_define([my_iface],   [my_current:my_revision:my_age])

# Autoconf 2.62 is bootstrapped with Automake 1.10.1.
# https://lists.gnu.org/archive/html/autotools-announce/2008-04/msg00002.html
AC_PREREQ(2.62)
# Initialize Autoconf
AC_INIT([sample], [my_version], [o-takashi@sakamocchi.jp])
# The directory for M4 macros
AC_CONFIG_MACRO_DIR([m4])

# The directory for sources
AC_CONFIG_SRCDIR([src])

# Automake 1.10 or later
# GObject Introspection uses GNU Make-specific functionality.
AM_INIT_AUTOMAKE([1.10.1 -Wno-portability foreign])
# Don't output command lines
AM_SILENT_RULES([yes])

# GNU libtool 2.2.6 is bootstrapped with Automake 1.10.1 and Autoconf 2.62.
# https://lists.gnu.org/archive/html/autotools-announce/2008-09/msg00000.html
LT_PREREQ(2.2.6)
# Initialize GNU libtool
LT_INIT
# Define library version
LT_IFACE="my_iface"
AC_SUBST(LT_IFACE)

# Detect C language compiler
AC_PROG_CC

# Glib 2.34 or later
AM_PATH_GLIB_2_0([2.34.0], [], [], [gobject])

# GObject introspection 1.32.1 or later
GOBJECT_INTROSPECTION_REQUIRE([1.32.1])

# The files generated from *.in
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
])

# Generate scripts and launch
AC_OUTPUT
