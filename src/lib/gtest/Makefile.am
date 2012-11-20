if MUST_COMPILE_GTEST

check_LIBRARIES = libgtest.a libgtest_main.a

libs:
	mkdir -p libs
	cd libs; $(CMAKE) /usr/src/gtest && $(MAKE)
		
libgtest.a: libs
	cp libs/$@ .

libgtest_main.a: libs
	cp libs/$@ .

clean-local:
	rm -rf libs

endif