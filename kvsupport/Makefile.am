librdkafka_version = librdkafka-1.2.2
librdkafka_src = $(librdkafka_version).tar.gz

libhttpserver_version = libhttpserver-0.19.0-20241211
libhttpserver_src = $(libhttpserver_version).tar.gz

#sqlite3_version = sqlite-autoconf-3360000
#sqlite3_version = sqlite-autoconf-3460000
sqlite3_version = sqlite-autoconf-3470200
sqlite3_src=$(sqlite3_version).tar.gz


microhttpd_version=$(shell pkg-config --modversion libmicrohttpd)

#all-local: librdkafka libhttpserver libsqlite
all-local: libsqlite libhttpserver
	if [ -f do-recheck ]; then \
		( cd ..; ./config.status --recheck ) ; \
		rm -f do-recheck ; \
	fi

librdkafka: 
	if [ ! -f $(librdkafka_version)-stamp ]; then \
		mkdir -p $(abs_top_builddir)/kvsupport/build ; \
		cp $(top_srcdir)/kvsupport/$(librdkafka_src) $(abs_top_builddir)/kvsupport ; \
		tar zxpf $(librdkafka_src) ; \
		(	cd $(librdkafka_version) ; \
			export LD_RUN_PATH="$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@/lib" ; \
			./configure --prefix=$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@ ; \
			make ; \
			make DESTDIR=$(abs_top_builddir)/kvsupport/build install ) ; \
		touch do-recheck ; \
		touch $(librdkafka_version)-stamp ; \
	fi 

libhttpserver:
	echo "microhttpd_version: $(microhttpd_version)"
	if [ ! -f $(libhttpserver_version)-stamp ]; then \
		mkdir -p $(abs_top_builddir)/kvsupport/build ; \
		cp $(top_srcdir)/kvsupport/$(libhttpserver_src) $(abs_top_builddir)/kvsupport ; \
		tar zxpf $(libhttpserver_src) ; \
		(	cd $(libhttpserver_version) ; \
			export LD_RUN_PATH="$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@/lib" ; \
			./bootstrap --build --prefix=$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@ ; \
			mkdir -p build ; \
			cd build ; \
			#Hack to build on xenial \
			if [ $(microhttpd_version) = "0.9.44" ]; then \
				../configure	--prefix=$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@ --enable-epoll=no ; \
			else \
				../configure	--prefix=$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@ ; \
			fi ;\
			make ; \
			make check ; \
			make DESTDIR=$(abs_top_builddir)/kvsupport/build install ; \
		) ; \
		touch do-recheck ; \
		touch $(libhttpserver_version)-stamp ; \
	fi 

libsqlite:
	echo "sqlite3: $(sqlite3_version)"
	if [ ! -f $(sqlite3_version)-stamp ]; then \
		mkdir -p $(abs_top_builddir)/kvsupport/build ; \
		cp $(top_srcdir)/kvsupport/$(sqlite3_src) $(abs_top_builddir)/kvsupport ; \
		tar zxpf $(sqlite3_src) ; \
		(	cd $(sqlite3_version) ; \
			export LD_RUN_PATH="$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@/lib" ; \
			./configure --prefix=$(prefix)/lib/kvalobs@KVALOBSLIBS_BIN_VERSION@ --enable-shared=no --with-pic; \
			make ; \
			make DESTDIR=$(abs_top_builddir)/kvsupport/build install ) ; \
		touch do-recheck ; \
		touch $(sqlite3_version)-stamp ; \
	fi 
	


# if MUST_COMPILE_GTEST
# gtestdir=${checkdir}
# gtest_LIBRARIES = libgtest.a libgtest_main.a

# libgtest_a_SOURCES = 
# libgtest_main_a_SOURCES = 

# lib_gtest:
# 	mkdir -p gtest
# 	cd gtest; $(CMAKE) $(gtest_src) && $(MAKE)
		
# libgtest.a: lib_gtest
# 	cp gtest/$@ .

# libgtest_main.a: lib_gtest
# 	cp gtest/$@ .


# endif


# if MUST_COMPILE_GMOCK
# gmockdir=${checkdir}
# gmock_LIBRARIES = libgmock.a libgmock_main.a

# libgmock_a_SOURCES = 
# libgmock_main_a_SOURCES = 

# lib_gmock:
# 	mkdir -p gmock
# 	cd gmock; $(CMAKE) $(gmock_src) && $(MAKE)
		
# libgmock.a: lib_gmock
# 	cp gmock/$@ .

# libgmock_main.a: lib_gmock
# 	cp gmock/$@ .

# endif

clean-local:
	rm -rf librdkafka*
	rm -rf libhttpserver*
	rm -rf build*
	rm -rf sqlite*
	
	
