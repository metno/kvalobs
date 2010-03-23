AM_CPPFLAGS = -I$(top_srcdir)/src/lib  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

bin_PROGRAMS = kvManagerd kvmgrclt
kvManagerd_SOURCES =  mgrApp.cc  mgrApp.h \
                      kvManagerd.cc \
                      managerInputImpl.cc managerInputImpl.h \
                      NewDataCommand.cc NewDataCommand.h \
                      checkedInputImpl.cc checkedInputImpl.h \
                      PreProcessJob.cc PreProcessJob.h \
                      NewDataJob.cc NewDataJob.h \
                      OneTimeJob.cc OneTimeJob.h \
                      PreProcessWorker.cc PreProcessWorker.h \
                      InitLogger.cc InitLogger.h \
                      PreProcessMissingData.cc PreProcessMissingData.h \
                      kvCheckedDataThread.cc kvCheckedDataThread.h \
                      CheckForMissingObsMessages.cc CheckForMissingObsMessages.h \
                      MissingObsCheck.cc MissingObsCheck.h \
                      AdminImpl.cc AdminImpl.h \
                      SelectDataToProcess.cc SelectDataToProcess.h \
                      SendDataToQa.cc SendDataToQa.h \
                      CheckedDataHelper.cc  CheckedDataHelper.h \
                      CheckedDataCommand.cc CheckedDataCommand.h \
                      GenCacheElem.cc GenCacheElem.h \
                      ServiceCheckedInputImpl.cc ServiceCheckedInputImpl.h \
                      ServiceDataCommand.cc ServiceDataCommand.h \
                      CheckedDataCommandBase.h
                      
                      
kvManagerd_CPPFLAGS = $(AM_CPPFLAGS)
kvManagerd_LDFLAGS = -pthread -export-dynamic
kvManagerd_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                   $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                   $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                   $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                   $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                   $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                   $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                   $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
                   $(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
                   $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                   $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) \
                   -ldl -lm


kvmgrclt_SOURCES = kvmgrclt.cc  kvmgrcltApp.cc kvmgrcltApp.h
kvmgrclt_LDFLAGS = -pthread -export-dynamic
kvmgrclt_LDADD = $(kvManagerd_LDADD)   
