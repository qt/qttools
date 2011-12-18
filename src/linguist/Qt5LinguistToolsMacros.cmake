#=============================================================================
# Copyright 2005-2011 Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of Kitware, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

MACRO (QT5_EXTRACT_OPTIONS _qt5_files _qt5_options)
  SET(${_qt5_files})
  SET(${_qt5_options})
  SET(_QT5_DOING_OPTIONS FALSE)
  FOREACH(_currentArg ${ARGN})
    IF ("${_currentArg}" STREQUAL "OPTIONS")
      SET(_QT5_DOING_OPTIONS TRUE)
    ELSE ("${_currentArg}" STREQUAL "OPTIONS")
      IF(_QT5_DOING_OPTIONS)
        LIST(APPEND ${_qt5_options} "${_currentArg}")
      ELSE(_QT5_DOING_OPTIONS)
        LIST(APPEND ${_qt5_files} "${_currentArg}")
      ENDIF(_QT5_DOING_OPTIONS)
    ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
  ENDFOREACH(_currentArg)
ENDMACRO (QT5_EXTRACT_OPTIONS)


MACRO(QT5_CREATE_TRANSLATION _qm_files)
   QT5_EXTRACT_OPTIONS(_lupdate_files _lupdate_options ${ARGN})
   SET(_my_sources)
   SET(_my_dirs)
   SET(_my_tsfiles)
   SET(_ts_pro)
   FOREACH (_file ${_lupdate_files})
     GET_FILENAME_COMPONENT(_ext ${_file} EXT)
     GET_FILENAME_COMPONENT(_abs_FILE ${_file} ABSOLUTE)
     IF(_ext MATCHES "ts")
       LIST(APPEND _my_tsfiles ${_abs_FILE})
     ELSE(_ext MATCHES "ts")
       IF(NOT _ext)
         LIST(APPEND _my_dirs ${_abs_FILE})
       ELSE(NOT _ext)
         LIST(APPEND _my_sources ${_abs_FILE})
       ENDIF(NOT _ext)
     ENDIF(_ext MATCHES "ts")
   ENDFOREACH(_file)
   FOREACH(_ts_file ${_my_tsfiles})
     IF(_my_sources)
       # make a .pro file to call lupdate on, so we don't make our commands too
       # long for some systems
       GET_FILENAME_COMPONENT(_ts_name ${_ts_file} NAME_WE)
       SET(_ts_pro ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${_ts_name}_lupdate.pro)
       SET(_pro_srcs)
       FOREACH(_pro_src ${_my_sources})
         SET(_pro_srcs "${_pro_srcs} \"${_pro_src}\"")
       ENDFOREACH(_pro_src ${_my_sources})
       FILE(WRITE ${_ts_pro} "SOURCES = ${_pro_srcs}")
     ENDIF(_my_sources)
     ADD_CUSTOM_COMMAND(OUTPUT ${_ts_file}
        COMMAND ${QT_LUPDATE_EXECUTABLE}
        ARGS ${_lupdate_options} ${_ts_pro} ${_my_dirs} -ts ${_ts_file}
        DEPENDS ${_my_sources} ${_ts_pro} VERBATIM)
   ENDFOREACH(_ts_file)
   QT5_ADD_TRANSLATION(${_qm_files} ${_my_tsfiles})
ENDMACRO(QT5_CREATE_TRANSLATION)


MACRO(QT5_ADD_TRANSLATION _qm_files)
  FOREACH (_current_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(qm ${_abs_FILE} NAME_WE)
    GET_SOURCE_FILE_PROPERTY(output_location ${_abs_FILE} OUTPUT_LOCATION)
    IF(output_location)
      FILE(MAKE_DIRECTORY "${output_location}")
      SET(qm "${output_location}/${qm}.qm")
    ELSE(output_location)
      SET(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
    ENDIF(output_location)

    ADD_CUSTOM_COMMAND(OUTPUT ${qm}
       COMMAND ${QT_LRELEASE_EXECUTABLE}
       ARGS ${_abs_FILE} -qm ${qm}
       DEPENDS ${_abs_FILE} VERBATIM
    )
    SET(${_qm_files} ${${_qm_files}} ${qm})
  ENDFOREACH (_current_FILE)
ENDMACRO(QT5_ADD_TRANSLATION)
