bin_PROGRAMS =			\
	test

test_CPPFLAGS =			\
	-I$(top_srcdir)/src

test_CFLAGS = 			\
	$(GLIB_CFLAGS)		\
	-g -Wall

test_LDADD =			\
	$(GLIB_LIBS)		\
	$(top_srcdir)/src/libsample.la

test_SOURCES =			\
	test.c
