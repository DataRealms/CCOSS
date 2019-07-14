all:
	cd libsndio && ${MAKE}
	cd sndiod && ${MAKE}
	cd aucat && ${MAKE}
	cd midicat && ${MAKE}

install:
	cd libsndio && ${MAKE} install
	cd sndiod && ${MAKE} install
	cd aucat && ${MAKE} install
	cd midicat && ${MAKE} install

uninstall:
	cd midicat && ${MAKE} uninstall	
	cd aucat && ${MAKE} uninstall	
	cd sndiod && ${MAKE} uninstall
	cd libsndio && ${MAKE} uninstall

clean:
	cd aucat && ${MAKE} clean
	cd midicat && ${MAKE} clean
	cd sndiod && ${MAKE} clean
	cd libsndio && ${MAKE} clean
	cd examples && ${MAKE} clean

distclean:	clean
	rm -f \
		Makefile aucat/Makefile midicat/Makefile sndiod/Makefile \
		libsndio/Makefile \
		examples/Makefile \
		contrib/init.d.sndiod \
		contrib/sndiod.service
