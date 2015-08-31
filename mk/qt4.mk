%.h:	%.ui
	mkdir -p `dirname $@` && $(UIC3) $< -o $@

%.cc: %.ui %.h
	mkdir -p `dirname $@` && $(UIC3) -impl $*.h $< -o $@

%.moc.cc: %.h
	mkdir -p `dirname $@` && $(MOC4) -o $@ $<
