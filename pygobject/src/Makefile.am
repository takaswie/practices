# Remove auto-generated files when cleaning
CLEANFILES =					\
	sample_sigs_marshal.*


AM_CPPFLAGS =					\
	-I$(top_builddir)			\
	-I$(top_srcdir)

AM_CFLAGS =					\
	$(GLIB_CFLAGS)				\
	-Wall

lib_LTLIBRARIES =				\
	libsample.la

libsample_la_LDFLAGS =				\
	-version-info $(LT_IFACE)		\
	-export-symbols $(top_srcdir)/src/sample.symbols

libsample_la_LIBADD =				\
	$(GLIB_LIBS)

BUILT_SOURCES =					\
	sample_sigs_marshal.h			\
	sample_sigs_marshal.c

SCANNED_SOURCES =				\
	$(BUILT_SOURCES)			\
	sample_context.h			\
	sample_context.c			\
	sample_listener.h			\
	sample_listener.c			\
	sample.symbols

libsample_la_SOURCES =				\
	$(SCANNED_SOURCES)

sample_sigs_marshal.list:
	$(AM_V_GEN)( find | grep \.c$$ | xargs cat | 			\
	sed -n -e 's/.*sample_sigs_marshal_\([A-Z]*__[A-Z_]*\).*/\1/p' |\
	sed -e 's/__/:/' -e 'y/_/,/' ) | sort -u > $@

sample_sigs_marshal.h: sample_sigs_marshal.list
	$(AM_V_GEN)( glib-genmarshal --header				\
		--prefix=sample_sigs_marshal $< ) > $@

sample_sigs_marshal.c: sample_sigs_marshal.list sample_sigs_marshal.h
	$(AM_V_GEN)(echo '#include "sample_sigs_marshal.h"';		\
		glib-genmarshal --body --prefix=sample_sigs_marshal $< ) > $@

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
Sample-1.0.gir: libsample.la
Sample_1_0_gir_PACKAGES =
Sample_1_0_gir_EXPORT_PACKAGES = sample
Sample_1_0_gir_INCLUDES = GObject-2.0
Sample_1_0_gir_CFLAGS =
Sample_1_0_gir_LIBS = libsample.la
Sample_1_0_gir_FILES = $(SCANNED_SOURCES)
Sample_1_0_gir_SCANNERFLAGS =			\
	--identifier-prefix=Sample		\
	--symbol-prefix=sample
INTROSPECTION_GIRS += Sample-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES +=					\
	$(gir_DATA)				\
	$(typelib_DATA)
endif
