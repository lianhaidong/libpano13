# This file is part of libpano13, the hugin build environment.
# Licence details can be found in the file gpl.txt.
#

project(test4)

add_test(optimizer1 ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomateOptimizerTest.pl" "test1.txt")

