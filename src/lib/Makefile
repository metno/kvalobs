TOP=../..


DIRLIST:=confparser milog miutil fileutil threadutil db  dbdrivers \
	corbahelper kvalobs  decoderbase  decodeutility autoobsdecoder \
	tips kldecoder kv2kvdecoder dummydecoder rejectdecoder comobsdecoder

#decoder


export DIRLIST

all:
	for DIR in $$DIRLIST ; do \
	   (cd $$DIR; $(MAKE) ) \
	done

veryclean:
	for DIR in $$DIRLIST ; do \
		(cd $$DIR; $(MAKE) veryclean) \
	done

clean:
	for DIR in $$DIRLIST ; do \
		(cd $$DIR; $(MAKE) clean) \
	done

depends:
	for DIR in $$DIRLIST ; do \
		(cd $$DIR; $(MAKE) depends) \
	done





