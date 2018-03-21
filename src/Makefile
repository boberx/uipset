CC = gcc
CCFL = -O2 -Wall -Wextra -I/usr/include/nanopb $(CFLAGS)
BUILDDIR = build

EXESRC = main.c debug.c ipset.c
EXEOBJ = $(patsubst %.c,$(BUILDDIR)/%.o,$(EXESRC))
EXEEXE = uipsetd

LIBSRC = uipset.c common.c
LIBOBJ = $(patsubst %.c,$(BUILDDIR)/%.o,$(LIBSRC))
LIBOUT = libuipset

NPBSRC = uipset.proto
NPBOBJ = $(patsubst %.proto,$(BUILDDIR)/%.pb.c,$(NPBSRC))
NPBOUT = $(patsubst %.proto,$(BUILDDIR)/%.pb.o,$(NPBSRC))

EXECFL = -O2 -Wall -Wextra $(CFLAGS)

all: dir $(BUILDDIR)/$(EXEEXE)

$(BUILDDIR)/$(EXEEXE): $(BUILDDIR)/$(LIBOUT).so $(EXEOBJ)
	$(CC) $(CCFL) $^ -o $@ -lipset -L $(BUILDDIR) -luipset

$(EXEOBJ): $(BUILDDIR)/%.o : %.c dir
	$(CC) $(CCFL) -c $< -o $@

$(BUILDDIR)/$(LIBOUT).so: $(NPBOUT) $(LIBOBJ)
	$(CC) $(CCFL) -fPIC -shared -Wl,-soname,$(LIBOUT).so $^ -o $@ -l:libprotobuf-nanopb.a

$(LIBOBJ): $(BUILDDIR)/%.o : %.c dir
	$(CC) $(CCFL) -c $< -o $@

$(NPBOBJ): $(NPBSRC) dir
	protoc --nanopb_out=$(BUILDDIR) $<

$(NPBOUT): $(NPBOBJ)
	$(CC) $(CCFL) -c $< -o $@

test: dir
	$(CC) -o $(BUILDDIR)/test test.c ipset.c debug.c -lipset

testclient: dir $(BUILDDIR)/$(EXEEXE)
	$(CC) $(CCFL) testclient.c -o $(BUILDDIR)/testclient -L$(BUILDDIR) -luipset

dir:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
