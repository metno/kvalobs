librdkafka_version = librdkafka-1.2.2
librdkafka_src = $(librdkafka_version).tar.gz

libhttpserver_version = libhttpserver-0.17.5
libhttpserver_src = $(libhttpserver_version).tar.gz


all-local: librdkafka libhttpserver
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
		  ./configure --prefix=$(abs_top_builddir)/kvsupport/build ; \
			make ; \
			make install ) ; \
		touch do-recheck ; \
		touch $(librdkafka_version)-stamp ; \
	fi 

libhttpserver:
	if [ ! -f $(libhttpserver_version)-stamp ]; then \
		mkdir -p $(abs_top_builddir)/kvsupport/build ; \
		cp $(top_srcdir)/kvsupport/$(libhttpserver_src) $(abs_top_builddir)/kvsupport ; \
		tar zxpf $(libhttpserver_src) ; \
		(	cd $(libhttpserver_version) ; \
		  ./bootstrap --build --prefix=$(abs_top_builddir)/kvsupport/build ; \
			mkdir -p build ; \
			cd build ; \
			../configure	--prefix=$(abs_top_builddir)/kvsupport/build ; \
			make ; \
			make check ; \
			make install ; \
		) ; \
		touch do-recheck ; \
		touch $(libhttpserver_version)-stamp ; \
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
	
