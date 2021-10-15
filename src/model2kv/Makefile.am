pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
if BUILD_MODEL2KV
bin_PROGRAMS = model2kv
else
EXTRA_PROGRAMS = model2kv
endif
	
model2kv_SOURCES = model2kv.cc  setup.h  suLex.l  suParse.h  suParse.y
model2kv_CPPFLAGS = $(libpose_CFLAGS) $(putools_CFLAGS)
model2kv_LDFLAGS = $(libpose_LIBS) $(putools_LIBS) -lpose -lpuCtools -lparameter  -lpuTools  -lm

model2kv_YFLAGS = -p yysu
model2kv_LFLAGS = -Pyysu -olex.yy.c

CLEANFILES = suLex.c suParse.c lex.yy.c
