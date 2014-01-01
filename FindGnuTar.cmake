#
#  Copyright (c) 2013, Kornel Benko, <kornel@lyx.org>
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  
#  1. Redistributions of source code must retain the copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products 
#     derived from this software without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Try to find the gnu tar command
# Once done this will define
#
#       GNUTAR_FOUND
#       GNUTAR_EXECUTABLE
#
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

find_program(GNUTAR_EXECUTABLE NAMES "gtar" "tar")
mark_as_advanced(GNUTAR_EXECUTABLE)

if(GNUTAR_EXECUTABLE)
  execute_process(COMMAND ${GNUTAR_EXECUTABLE} --version
    OUTPUT_VARIABLE GNUTAR_output
    ERROR_VARIABLE GNUTAR_error
    RESULT_VARIABLE GNUTAR_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(${GNUTAR_result} EQUAL 0)
    string(REGEX REPLACE "[\n]+" ";" GNUTAR_version ${GNUTAR_output})
    foreach(_l ${GNUTAR_version})
      if(_l MATCHES "^tar +\\(GNU +tar\\) +[0-9]")
        string(REGEX REPLACE "^tar +\\(GNU +tar\\) +([0-9]\\.[0-9]+)" "\\1" GNUTAR_VERSION_STRING ${_l})
        break()
      endif()
    endforeach()
  endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set GNUTAR_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GNUTAR DEFAULT_MSG GNUTAR_EXECUTABLE GNUTAR_VERSION_STRING)
include(CMakeParseArguments)

