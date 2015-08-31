%.h:	%.ui
	mkdir -p `dirname $@` && $(QT_UIC) $< -o $@

%.cc: %.ui %.h
	mkdir -p `dirname $@` && $(QT_UIC) -impl $*.h $< -o $@

%.moc.cc: %.h
	mkdir -p `dirname $@` && $(QT_MOC) -o $@ $<
