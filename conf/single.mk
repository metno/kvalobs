DEPENDSFILE=make.depends

all:   mark $(PROJECT) install


mark:
	@echo "[0;34m============================================= [0m"

depends:
	rm -f $(DEPENDSFILE)
	echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -M  $(ALLSRC) >> $(DEPENDSFILE)

pretty:
	find . \( -name 'core' -o -name '*~' -o -name 'core.*' \) \
	     -exec rm -f {} \;

clean:
	$(MAKE) pretty
	rm -f *.o
	rm -f $(PROJECT)
	rm -f $(TEST_BIN)

veryclean:
	$(MAKE) pretty
	$(MAKE) clean
	rm -f $(DEPENDSFILE)
	touch $(DEPENDSFILE)


install:
	@echo "[1mMAKE INSTALL ...................[0m";
	@if [ $(TOBIN) ]; then                                         \
          if [ ! -d $(TOBIN) ]; then                                   \
             mkdir -p $(TOBIN);                                        \
          fi;                                                          \
          if [  -d $(TOBIN) ]; then                                    \
             cp $(PROJECT) $(TOBIN) ;                                  \
             echo "[4;34m Install bin(s): ..... $(PROJECT) [0;0m"; \
             echo "[1mINSTALL FINISHED................[0m";        \
          else                                                         \
             echo "[1;31mFAILED to install "${PROJECT}" !!!![0m";  \
          fi                                                           \
        else                                                           \
          echo "[1;31mNOTHING to install "${PROJECT}" !!!![0m";    \
        fi







