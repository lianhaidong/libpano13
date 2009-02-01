EXTRA_DIST = 060520_3398.TIF \
             ChangeLog simple.txt

SUBDIRS = reference

MAINTAINERCLEANFILES = \
	Makefile.in

all:

check: test

test:
	perl ../panoAutomateTest.pl simple.txt

verify:
	perl ../panoAutomateTest.pl -t simple.txt

clean:
	rm -f testOutput* _PTSti* tests/*.*

