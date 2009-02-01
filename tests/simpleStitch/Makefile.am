EXTRA_DIST = 051221_6054_750.jpg 051221_6055_750.jpg \
             ChangeLog README.txt simple.pto simple.txt

MAINTAINERCLEANFILES = \
	Makefile.in

SUBDIRS = reference

all:

check: test

test:
	perl ../panoAutomateTest.pl simple.txt

verify:
	perl ../panoAutomateTest.pl -t simple.txt

clean:
	rm -f testOutput* _PTSti* tests/*.*

