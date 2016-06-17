# General variables, needed because of the configure script

prefix		= /usr/local
exec_prefix	= ${prefix}

# Some variables to easily adapt the version number of the library

LIBRARYNAME = libjrtp
LIBRARYMAJORVERSION = 2
LIBRARYMINORVERSION = 9
LIBRARYFULLVERSION = $(LIBRARYMAJORVERSION).$(LIBRARYMINORVERSION)
LIBRARYFULLNAME = $(LIBRARYNAME).so-$(LIBRARYFULLVERSION)
LIBRARYINTERNALNAME = $(LIBRARYNAME).so.$(LIBRARYFULLVERSION)
ARCHIVE = $(LIBRARYNAME).a

LDFLAGS = -shared -soname
ARFLAGS = qc

# Where to install the libraries. This is determined by the configure
# script

LIBRARYINSTALLDIR = ${exec_prefix}/lib
HEADERINSTALLDIR = ${prefix}/include/jrtplib

# Flags and objects

CC 	= g++
FLAGS 	= -O2
OBJECTS = rtpconnection.o rtpcontributingsources.o rtpdestlist.o \
          rtperror.o rtpiptable.o rtppacket.o rtppacketprocessor.o \
          rtprandom.o rtprtcpmodule.o rtpsession.o rtpsourcedata.o \
          rtpsourcedescription.o rtpsources.o rtpsrclist.o rtptimeutil.o \
          rtplocalinfo.o rtphandlers.o rtpdebug.o rtpmcasttable.o

# Rules to make the compilation possible

.SUFFIXES: .cpp

.cpp.o:
	$(CC) $(FLAGS) -c -o $@ $<

all: sharedlib staticlib

clean:
	rm -f $(OBJECTS) $(LIBRARYFULLNAME) $(ARCHIVE)

distclean: clean
	rm -f config.log config.cache config.status Makefile rtpconfig_unix.h
	
dummymsg:
	@echo "Nothing to build"
	
sharedlib: $(LIBRARYFULLNAME)
	
staticlib: $(ARCHIVE)

install: all
	if ! [ -d $(LIBRARYINSTALLDIR) ] ; then \
		mkdir -m 755 -p $(LIBRARYINSTALLDIR) ; \
	fi
	if ! [ -d $(HEADERINSTALLDIR) ] ; then \
		mkdir -m 755 -p $(HEADERINSTALLDIR) ; \
	fi
	if [ -e $(LIBRARYFULLNAME) ] ; then \
		install -m 755 $(LIBRARYFULLNAME) $(LIBRARYINSTALLDIR) ; \
	fi
	if [ -e $(ARCHIVE) ] ; then \
		install -m 644 $(ARCHIVE) $(LIBRARYINSTALLDIR) ; \
	fi
	install -m 644 *.h $(HEADERINSTALLDIR)
	(cd $(LIBRARYINSTALLDIR) ; ln -sf $(LIBRARYFULLNAME) $(LIBRARYINTERNALNAME) )
	(cd $(LIBRARYINSTALLDIR) ; ln -sf $(LIBRARYFULLNAME) $(LIBRARYNAME).so )
	

$(LIBRARYFULLNAME): $(OBJECTS)
	rm -f $(LIBRARYFULLNAME)
	ld $(LDFLAGS) $(LIBRARYINTERNALNAME) -o $(LIBRARYFULLNAME) $(OBJECTS)

$(ARCHIVE): $(OBJECTS)
	rm -f $(ARCHIVE)
	ar $(ARFLAGS) $(ARCHIVE) $(OBJECTS)

rtpconnection.o: rtpconnection.cpp
rtpcontributingsources.o: rtpcontributingsources.cpp
rtpdestlist.o: rtpdestlist.cpp
rtperror.o: rtperror.cpp
rtpiptable.o: rtpiptable.cpp
rtppacket.o: rtppacket.cpp
rtppacketprocessor.o: rtppacketprocessor.cpp
rtprandom.o: rtprandom.cpp
rtprtcpmodule.o: rtprtcpmodule.cpp
rtpsession.o: rtpsession.cpp
rtpsourcedata.o: rtpsourcedata.cpp
rtpsourcedescription.o: rtpsourcedescription.cpp
rtpsources.o: rtpsources.cpp
rtpsrclist.o: rtpsrclist.cpp
rtptimeutil.o: rtptimeutil.cpp
rtplocalinfo.o: rtplocalinfo.cpp
rtphandlers.o: rtphandlers.cpp
rtpdebug.o: rtpdebug.cpp
rtpmcasttable.o: rtpmcasttable.cpp

