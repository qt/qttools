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

include(MacroAddFileDependencies)

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


MACRO(QT5_ADD_DBUS_INTERFACE _sources _interface _basename)
  GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
  SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  GET_SOURCE_FILE_PROPERTY(_nonamespace ${_interface} NO_NAMESPACE)
  IF(_nonamespace)
    SET(_params -N -m)
  ELSE(_nonamespace)
    SET(_params -m)
  ENDIF(_nonamespace)

  GET_SOURCE_FILE_PROPERTY(_classname ${_interface} CLASSNAME)
  IF(_classname)
    SET(_params ${_params} -c ${_classname})
  ENDIF(_classname)

  GET_SOURCE_FILE_PROPERTY(_include ${_interface} INCLUDE)
  IF(_include)
    SET(_params ${_params} -i ${_include})
  ENDIF(_include)

  ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} -p ${_basename} ${_infile}
      DEPENDS ${_infile} VERBATIM)

  SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

  QT5_GENERATE_MOC(${_header} ${_moc})

  SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

ENDMACRO(QT5_ADD_DBUS_INTERFACE)


MACRO(QT5_ADD_DBUS_INTERFACES _sources)
  FOREACH (_current_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_infile ${_current_FILE} ABSOLUTE)
    # get the part before the ".xml" suffix
    STRING(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.xml" "\\2" _basename ${_current_FILE})
    STRING(TOLOWER ${_basename} _basename)
    QT5_ADD_DBUS_INTERFACE(${_sources} ${_infile} ${_basename}interface)
  ENDFOREACH (_current_FILE)
ENDMACRO(QT5_ADD_DBUS_INTERFACES)


MACRO(QT5_GENERATE_DBUS_INTERFACE _header) # _customName OPTIONS -some -options )
  QT5_EXTRACT_OPTIONS(_customName _qt4_dbus_options ${ARGN})

  GET_FILENAME_COMPONENT(_in_file ${_header} ABSOLUTE)
  GET_FILENAME_COMPONENT(_basename ${_header} NAME_WE)

  IF (_customName)
    if (IS_ABSOLUTE ${_customName})
      get_filename_component(_containingDir ${_customName} PATH)
      if (NOT EXISTS ${_containingDir})
        file(MAKE_DIRECTORY "${_containingDir}")
      endif()
      SET(_target ${_customName})
    else()
      SET(_target ${CMAKE_CURRENT_BINARY_DIR}/${_customName})
    endif()
  ELSE (_customName)
    SET(_target ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.xml)
  ENDIF (_customName)

  ADD_CUSTOM_COMMAND(OUTPUT ${_target}
      COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${_qt4_dbus_options} ${_in_file} -o ${_target}
      DEPENDS ${_in_file} VERBATIM
  )
ENDMACRO(QT5_GENERATE_DBUS_INTERFACE)


MACRO(QT5_ADD_DBUS_ADAPTOR _sources _xml_file _include _parentClass) # _optionalBasename _optionalClassName)
  GET_FILENAME_COMPONENT(_infile ${_xml_file} ABSOLUTE)

  SET(_optionalBasename "${ARGV4}")
  IF (_optionalBasename)
    SET(_basename ${_optionalBasename} )
  ELSE (_optionalBasename)
    STRING(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.xml" "\\2adaptor" _basename ${_infile})
    STRING(TOLOWER ${_basename} _basename)
  ENDIF (_optionalBasename)

  SET(_optionalClassName "${ARGV5}")
  SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  IF(_optionalClassName)
    ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
       COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -a ${_basename} -c ${_optionalClassName} -i ${_include} -l ${_parentClass} ${_infile}
       DEPENDS ${_infile} VERBATIM
    )
  ELSE(_optionalClassName)
    ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
       COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -a ${_basename} -i ${_include} -l ${_parentClass} ${_infile}
       DEPENDS ${_infile} VERBATIM
     )
  ENDIF(_optionalClassName)

  QT5_GENERATE_MOC(${_header} ${_moc})
  SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

  SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
ENDMACRO(QT5_ADD_DBUS_ADAPTOR)
