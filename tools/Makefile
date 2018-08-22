#
# Makefile
# Adrian Perez, 2018-08-22 14:52
#

all: chkcap

chkcap: override LDLIBS += -lcap
chkcap: override CFLAGS += -std=c11

clean:
	$(RM) chkcap chkcap.o

# vim:ft=make
#
