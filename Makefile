MYOS = $(shell uname -s)
MYREL = $(shell uname -r | cut -f1 -d-)
MYARCH = $(shell uname -p)

prefix ?= $(CURDIR)/$(MYOS)-$(MYREL)-$(MYARCH)

ifdef DESTDIR
    export INSTALL_ROOT := $(DESTDIR)
endif

export QMAKEFEATURES := $(PWD)/features
PROJECT_FILE = main.pro
REAL_MAKEFILE = Makefile.$(PROJECT_FILE)


ifeq ($(MYOS), Darwin)
QMAKE = qmake -spec macx-g++
else
QMAKE = qmake
endif

default: $(REAL_MAKEFILE)
	$(MAKE) -f $(REAL_MAKEFILE) all


install: $(REAL_MAKEFILE)
	$(MAKE) -f $(REAL_MAKEFILE) install

%: $(REAL_MAKEFILE)
	$(MAKE) -f $(REAL_MAKEFILE) $@


$(REAL_MAKEFILE): $(PROJECT_FILE)
	$(QMAKE) prefix=$(prefix) -o $(REAL_MAKEFILE) $(PROJECT_FILE)

$(PROJECT_FILE): ;
