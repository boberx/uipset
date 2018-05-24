NANOPB_DIR = /usr/include/nanopb

CC = gcc
CCFL = -O2 -Wall -Wextra -DPB_NO_ERRMSG -DPB_FIELD_16BIT -I${NANOPB_DIR} $(CFLAGS)
BUILDDIR = build

NANOPB_SRC = pb_encode.c pb_decode.c pb_common.c
NANOPB_OBJ = $(patsubst %.c,$(BUILDDIR)/%.o,$(NANOPB_SRC))

EXESRC = main.c ipset.c
EXEOBJ = $(patsubst %.c,$(BUILDDIR)/%.o,$(EXESRC))
EXEEXE = uipsetd

LIBSRC = uipset.c debug.c common.c ${NANOPB_CORE}
LIBOBJ = $(patsubst %.c,$(BUILDDIR)/%.o,$(LIBSRC))
LIBOUT = libuipset

NPBSRC = uipset.proto
NPBOBJ = $(patsubst %.proto,$(BUILDDIR)/%.pb.c,$(NPBSRC))
NPBOUT = $(patsubst %.proto,$(BUILDDIR)/%.pb.o,$(NPBSRC))

.PHONY: all clean install uninstall

all: dir $(BUILDDIR)/$(EXEEXE)

$(NANOPB_OBJ): $(BUILDDIR)/%.o : ${NANOPB_DIR}/%.c dir
	$(CC) $(CCFL) -c $< -o $@

$(BUILDDIR)/$(EXEEXE): $(BUILDDIR)/$(LIBOUT).so $(EXEOBJ)
	$(CC) $(CCFL) $^ -o $@ -lipset -L $(BUILDDIR) -luipset

$(EXEOBJ): $(BUILDDIR)/%.o : %.c dir
	$(CC) $(CCFL) -c $< -o $@

$(BUILDDIR)/$(LIBOUT).so: $(NPBOUT) $(LIBOBJ) $(NANOPB_OBJ)
	$(CC) $(CCFL) -fPIC -shared -Wl,-soname,$(LIBOUT).so $^ -o $@

$(LIBOBJ): $(BUILDDIR)/%.o : %.c dir
	$(CC) $(CCFL) -c $< -o $@

$(NPBOBJ): $(NPBSRC) dir
	protoc --nanopb_out=$(BUILDDIR) $<

$(NPBOUT): $(NPBOBJ)
	$(CC) $(CCFL) -c $< -o $@

dir:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
