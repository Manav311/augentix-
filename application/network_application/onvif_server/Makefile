TARGETS := onvif_service

.PHONY : all install uninstall clean distclean $(TARGETS)


all: $(TARGETS)

$(TARGETS):
	$(MAKE) -C $@

install: all
	@for f in $(TARGETS); do \
		(cd ./$$f && $(MAKE) install;) \
	done

uninstall:
	@for f in $(TARGETS); do \
		(cd ./$$f && $(MAKE) uninstall;) \
	done	

clean:
	@for f in $(TARGETS); do \
		(cd ./$$f; $(MAKE) clean;) \
	done

distclean: uninstall clean
