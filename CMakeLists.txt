cmake_minimum_required(VERSION 2.8.0)

# Included scripts do automatic cmake_policy PUSH and POP.
# OLD: CMake policy in included script will affect the invoker.
CMAKE_POLICY(SET CMP0011 OLD)
IF(POLICY CMP0065)
    CMAKE_POLICY(SET CMP0065 OLD)
ENDIF()

####################################################################
# Find cmake-fedora
#
LIST(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/Modules")
INCLUDE(ManageEnvironmentCommon OPTIONAL RESULT_VARIABLE MANAGE_ENVIRONMENT_PATH)

IF(MANAGE_ENVIRONMENT_PATH STREQUAL "NOTFOUND")
    # Is cmake-fedora a git submodule?
    IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/Modules)
	## Create a link if Modules/ does not exists
	IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/cmake-fedora)
	    IF(EXISTS ${CMAKE_SOURCE_DIR}/.gitmodules)
                ## cmake-fedora may be a git submodules
		EXECUTE_PROCESS(COMMAND git submodule init
		    COMMAND git submodule update
		    RESULT_VARIABLE gitSubmoduleFailed
		    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		    )
	    ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/.gitmodules)
	ENDIF(NOT EXISTS ${CMAKE_SOURCE_DIR}/cmake-fedora)

	IF(EXISTS ${CMAKE_SOURCE_DIR}/cmake-fedora/Modules)
            ## cmake-fedora is now available as subdirectory, try again
	    EXECUTE_PROCESS(COMMAND ln -s ${CMAKE_SOURCE_DIR}/cmake-fedora/Modules ${CMAKE_SOURCE_DIR}/Modules)
	    INCLUDE(ManageEnvironmentCommon RESULT_VARIABLE MANAGE_ENVIRONMENT_PATH)
	ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/cmake-fedora/Modules)
    ENDIF(NOT EXISTS ${CMAKE_SOURCE_DIR}/Modules)
ENDIF(MANAGE_ENVIRONMENT_PATH STREQUAL "NOTFOUND")

IF(MANAGE_ENVIRONMENT_PATH STREQUAL "NOTFOUND")
    MESSAGE(FATAL_ERROR "ManageEnvironmentCommon.cmake is not found in CMAKE_MODULE_PATH.
Please either:
    1) Use cmake-fedora as a git submodule:
       git submodule add https://git.fedorahosted.org/git/cmake-fedora.git
    2) Install cmake-fedora from repository:
       yum -y install cmake-fedora
    3) Download the source:
       wget https://fedorahosted.org/releases/c/m/cmake-fedora/cmake-fedora-modules-only-latest.tar.gz
       tar zxvf SOURCES/cmake-fedora-modules-only-latest.tar.gz")
ENDIF(MANAGE_ENVIRONMENT_PATH STREQUAL "NOTFOUND")

GET_FILENAME_COMPONENT(CMAKE_FEDORA_MODULE_DIR
    "${MANAGE_ENVIRONMENT_PATH}" PATH CACHE
    )

# Whether to build on fedora
SET(CMAKE_FEDORA_ENABLE_FEDORA_BUILD "0" CACHE STRING "Enable fedora build")

# Default Message level M_OFF (4)
SET(MANAGE_MESSAGE_LEVEL 7 CACHE STRING "Message (Verbose) Level")

####################################################################
# Project specific information
#
PROJECT(ibus-chewing)
SET(PRJ_SUMMARY "The Chewing engine for IBus input platform")
SET(SUMMARY_TRANSLATIONS
    "zh_TW" "IBus新酷音輸入法")

SET(PRJ_DESCRIPTION
"IBus-chewing is an IBus front-end of Chewing, an intelligent Chinese input
method for Zhuyin (BoPoMoFo) users.
It supports various Zhuyin keyboard layout, such as standard (DaChen),
IBM, Gin-Yeah, Eten, Eten 26, Hsu, Dvorak, Dvorak-Hsu, and DaChen26.

Chewing also support toned Hanyu pinyin input.")
SET(DESCRIPTION_TRANSLATIONS
"zh_TW"
"IBus-chewing 是新酷音輸入法的IBus前端。
新酷音輸入法是個智慧型注音輸入法，支援多種鍵盤布局，諸如：
標準注音鍵盤、IBM、精業、倚天、倚天26鍵、許氏、Dvorak、Dvorak許氏
及大千26鍵。

本輸入法也同時支援帶調漢語拼音輸入。")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

SET(AUTHORS "Peng Huang, Ding-Yi Chen")
SET(MAINTAINER "Ding-Yi Chen <dchen at redhat.com>")
SET(VENDOR "Red Hat, APAC, Inc.")
SET(LICENSE "GPLv2+")
SET(PRJ_GROUP "System Environment/Libraries")
SET(RPM_SPEC_URL "https://github.com/definite/${PROJECT_NAME}")
SET(RPM_SPEC_SOURCES "https://github.com/definite/%{name}/releases/download/%{version}/%{name}-%{version}-Source.tar.gz")

####################################################################
# Includes
#

# Use standard FHS if Fedora build is not enabled
IF(NOT CMAKE_FEDORA_ENABLE_FEDORA_BUILD)
    SET(LIBEXEC_DIR "${LIB_DIR}" CACHE PATH "LIBEXEC dir")
ENDIF(NOT CMAKE_FEDORA_ENABLE_FEDORA_BUILD)

INCLUDE(ManageEnvironment)
MANAGE_CMAKE_POLICY(CMP0017 NEW)

INCLUDE(ManageMessage)
INCLUDE(ManageVersion)
RELEASE_NOTES_READ_FILE()
INCLUDE(ManageArchive)
INCLUDE(ManageDependency)
INCLUDE(ManageFile)
INCLUDE(ManageSourceVersionControl)
INCLUDE(ManageTarget)
INCLUDE(ManageTranslation)
INCLUDE(ManageUninstall)

ENABLE_TESTING()

####################################################################
# Dependencies
#

# Enable GNOME_SHELL support by set the value to ON
SET(GNOME_SHELL ON CACHE BOOL "Enable GNOME Shell support")

# Enable GCONF2 support by set the value to ON
SET(GCONF2_SUPPORT OFF CACHE BOOL "Enable GConf2 support")

# Enable GSETTINGS support by set the value to ON
SET(GSETTINGS_SUPPORT ON CACHE BOOL "Enable GSettings support")

SET(IBUS_MIN_VER "1.3")
SET(CHEWING_MIN_VER "0.3.3")

########## Package Dependencies ##########

MANAGE_DEPENDENCY(BUILD_REQUIRES CMAKE VER "2.6.2" REQUIRED)

### ibus
MANAGE_DEPENDENCY(REQUIRES IBUS VER "${IBUS_MIN_VER}" REQUIRED PKG_CONFIG ${IBUS_PKG_CONFIG})

SET(IBUS_PKG_CONFIG "ibus-1.0" CACHE STRING "ibus pkg-config name")
MANAGE_DEPENDENCY(BUILD_REQUIRES IBUS VER "${IBUS_MIN_VER}" REQUIRED
    PKG_CONFIG ${IBUS_PKG_CONFIG} DEVEL
    )

### libchewing
SET(CHEWING_PACKAGE_NAME "libchewing" CACHE STRING "chewing package name")
MANAGE_DEPENDENCY(REQUIRES CHEWING VER "${CHEWING_MIN_VER}" REQUIRED
    PACKAGE_NAME "${CHEWING_PACKAGE_NAME}"
    )

SET(CHEWING_DEVEL_PACKAGE_NAME "${CHEWING_PACKAGE_NAME}-devel" CACHE STRING "chewing devel package name")
SET(CHEWING_PKG_CONFIG "chewing" CACHE STRING "chewing pkg-config name")
MANAGE_DEPENDENCY(BUILD_REQUIRES CHEWING VER "${CHEWING_MIN_VER}" REQUIRED
    PKG_CONFIG ${CHEWING_PKG_CONFIG}
    PACKAGE_NAME "${CHEWING_DEVEL_PACKAGE_NAME}"
    )

M_MSG(${M_INFO1} "CHEWING_VERSION=${CHEWING_VERSION}")
STRING_SPLIT(chewingVerList "." "${CHEWING_VERSION}")
LIST(GET chewingVerList 0 CHEWING_VERSION_MAJOR)
LIST(GET chewingVerList 1 CHEWING_VERSION_MINOR)
LIST(GET chewingVerList 2 CHEWING_VERSION_PATCH)
M_MSG(${M_INFO1} "CHEWING_VERSION_MAJOR=${CHEWING_VERSION_MAJOR}")
M_MSG(${M_INFO1} "CHEWING_VERSION_MINOR=${CHEWING_VERSION_MINOR}")
M_MSG(${M_INFO1} "CHEWING_VERSION_PATCH=${CHEWING_VERSION_PATCH}")
ADD_DEFINITIONS("-DCHEWING_VERSION_MAJOR=${CHEWING_VERSION_MAJOR}"
    "-DCHEWING_VERSION_MINOR=${CHEWING_VERSION_MINOR}"
    "-DCHEWING_VERSION_PATCH=${CHEWING_VERSION_PATCH}"
    )

## Find the correct chewin datadir
FOREACH(d ${CHEWING_CHEWING_DATADIR} ${CHEWING_LIBDIR}/libchewing  ${DATA_DIR}/chewing ${DATA_DIR}/libchewing)
    IF(EXISTS "${d}/symbols.dat")
	SET(CHEWING_DATADIR_REAL "${d}" CACHE STRING "CHEWING_DATADIR_REAL")
	BREAK()
    ENDIF()
ENDFOREACH()

IF("${CHEWING_DATADIR_REAL}" STREQUAL "")
    M_MSG(${M_ERROR} "Chewing: symbols.dat cannot be found, please reinstall chewing package")
ENDIF()

M_MSG(${M_INFO1} "CHEWING_DATADIR_REAL=${CHEWING_DATADIR_REAL}")
SET_COMPILE_ENV(CHEWING_DATADIR_REAL)

### Glib, Gtk and GOB
## Glib2 is not explicit list as Requires, as it should be pick up by rpm
SET(GLIB2_PKG_CONFIG "gio-2.0;glib-2.0;gmodule-2.0;gobject-2.0;gthread-2.0"
    CACHE STRING "glib2 pkg-config name")
MANAGE_DEPENDENCY(BUILD_REQUIRES GLIB2 REQUIRED
    PKG_CONFIG ${GLIB2_PKG_CONFIG} glib-2.0 DEVEL
    )

MANAGE_DEPENDENCY(REQUIRES GTK3  REQUIRED)
SET(GTK3_PKG_CONFIG "gtk+-3.0" CACHE STRING "gtk pkg-config name")
MANAGE_DEPENDENCY(BUILD_REQUIRES GTK3 REQUIRED
    PKG_CONFIG ${GTK3_PKG_CONFIG} DEVEL
    )

MANAGE_DEPENDENCY(BUILD_REQUIRES GOB2 VER "2.0.16" REQUIRED
    PROGRAM_NAMES "gob2"
    )

### X11
SET(X11_PKG_CONFIG "x11" CACHE STRING "x11 pkg-config name")
MANAGE_DEPENDENCY(BUILD_REQUIRES X11 REQUIRED
    PKG_CONFIG ${X11_PKG_CONFIG} PACKAGE_NAME "libX11-devel"
    )

####################################################################
# Building
#

#=== Compile flags
ADD_DEFINITIONS(-std=c99 -D_XOPEN_SOURCE)
IF(GSETTINGS_SUPPORT)
    ADD_DEFINITIONS(-DUSE_GSETTINGS)
ENDIF()
IF(GCONF2_SUPPORT)
    ADD_DEFINITIONS(-DUSE_GCONF2)
ENDIF()

## Directory that store ibus-chewing icons
SET_COMPILE_ENV(PRJ_ICON_DIR "${PRJ_DATA_DIR}/icons" CACHE STRING "PRJ_ICON_DIR")

#=== GSettings/GConf schemas
SET_COMPILE_ENV(PROJECT_SCHEMA_ID "org.freedesktop.IBus.Chewing" CACHE STRING "PROJECT_SCHEMA_ID")
SET_COMPILE_ENV(PROJECT_SCHEMA_BASE "/desktop/ibus/engine" CACHE STRING "PROJECT_SCHEMA_BASE")
SET_COMPILE_ENV(PROJECT_SCHEMA_SECTION "chewing" CACHE STRING "PROJECT_SCHEMA_SECTION")
SET_COMPILE_ENV(PROJECT_SCHEMA_DIR "${PROJECT_SCHEMA_BASE}/${PROJECT_SCHEMA_SECTION}" CACHE STRING "PROJECT_SCHEMA_DIR")
SET_COMPILE_ENV(PROJECT_GCONF2_SCHEMA_SECTION "Chewing" CACHE STRING "PROJECT_GCONF2_SCHEMA_SECTION")
SET_COMPILE_ENV(PROJECT_GCONF2_SCHEMA_DIR "${PROJECT_SCHEMA_BASE}/${PROJECT_GCONF2_SCHEMA_SECTION}"
    CACHE STRING "PROJECT_GCONF2_SCHEMA_DIR"
    )
SET_COMPILE_ENV(PROJECT_SCHEMA_PATH "${PROJECT_SCHEMA_DIR}/" CACHE STRING "PROJECT_SCHEMA_PATH")

#=== chewing.xml
SET(SYMBOL_XML "<symbol>&#x9177;</symbol>")
SET_COMPILE_ENV(CHEWING_HOTKEYS "Super+space")
SET(HOTKEYS_XML "<hotkeys>${CHEWING_HOTKEYS}</hotkeys>")
SET(SETUP_PROGRAM_XML "<setup>${LIBEXEC_DIR}/ibus-setup-chewing</setup>")

IF(IBUS_VERSION LESS 1.3.99)
    SET(SYMBOL_XML "<!-- ${SYMBOL_XML} -->")
    SET(HOTKEYS_XML "<!-- ${HOTKEYS_XML} -->")
    SET(SETUP_PROGRAM_XML "<!-- ${SETUP_PROGRAM_XML} -->")
ENDIF()

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/data/chewing.xml.in ${CMAKE_BINARY_DIR}/data/chewing.xml)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/data/ibus-setup-chewing.desktop.in ${CMAKE_BINARY_DIR}/data/ibus-setup-chewing.desktop)

#=== Sub directories
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(src bin)
ADD_SUBDIRECTORY(test test-bin)

####################################################################
# Installing
#

INSTALL(FILES AUTHORS README.md ChangeLog COPYING USER-GUIDE
    DESTINATION "${PRJ_DOC_DIR}"
    )

INSTALL(FILES ${CMAKE_BINARY_DIR}/data/chewing.xml
    DESTINATION "${DATA_DIR}/ibus/component")

INSTALL(FILES ${CMAKE_BINARY_DIR}/data/ibus-setup-chewing.desktop
    DESTINATION "${DATA_DIR}/applications")

INSTALL(DIRECTORY icons
    DESTINATION "${PRJ_DATA_DIR}"
    )

MANAGE_SOURCE_VERSION_CONTROL_GIT()

####################################################################
# Packing
#
SET(SOURCE_ARCHIVE_DIR ${CMAKE_BINARY_DIR} CACHE PATH "Source Archive Dir")
PACK_SOURCE_ARCHIVE("${SOURCE_ARCHIVE_DIR}"
    GITIGNORE ${CMAKE_SOURCE_DIR}/.gitignore
    INCLUDE ${PROJECT_NAME}.pot
    )

INCLUDE(ManageRelease)

IF(CMAKE_FEDORA_ENABLE_FEDORA_BUILD)
    SET(RPM_BUILD_SOURCES ${SOURCE_ARCHIVE_DIR})
    ADD_SUBDIRECTORY(rpm)
    LIST(APPEND _releaset_targets "release_fedora")
ENDIF(CMAKE_FEDORA_ENABLE_FEDORA_BUILD)


#####################################################################
# Hosting
#

#INCLUDE(ManageUpload)
#ADD_CUSTOM_TARGET(upload
#    COMMENT "Uploading all files"
#    )

####################################################################
# Custom
#

####################################################################
# Release
#

INCLUDE(ManageRelease)
SET(releaseTarget "tag_push")
IF(CMAKE_FEDORA_ENABLE_FEDORA_BUILD)
    LIST(APPEND releaseTargets release_fedora)
ENDIF(CMAKE_FEDORA_ENABLE_FEDORA_BUILD)
MANAGE_RELEASE(${releaseTargets})

