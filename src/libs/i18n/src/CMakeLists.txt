# ***** BEGIN LICENSE BLOCK *****
# The contents of this file are subject to the Mozilla Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
# 
# The Original Code is BrowserPlus (tm).
# 
# The Initial Developer of the Original Code is Yahoo!.
# Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
# All rights reserved.
# 
# Contributor(s): 
# ***** END LICENSE BLOCK *****
######################################################################
# cmakelists.txt for BP I18n library
#
SET(libName I18n)

# How do we use this?
SET(${libName}_MAJOR_VERSION 0)
SET(${libName}_MINOR_VERSION 1)

# Tell build templates who we consume.
# This enables bt to emit link statements for either ourselves (dll case) 
# or our clients (static lib case).
# Note: currently (01/28/09): libs listed here must have name ending *_s
IF (UNIX)
    SET(${libName}_LINK_STATIC BPUtils)
ELSE ()
    SET(${libName}_LINK_STATIC BPUtils icuuc icudt icuin)
ENDIF ()

# Setup some paths.
SET(BPEXTERNAL_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../external/dist")

# Setup preprocessor definitions.
ADD_DEFINITIONS(-DU_STATIC_IMPLEMENTATION)

# Include path to icu includes so that inter-header inclusions can be
# resolved.
INCLUDE_DIRECTORIES("${BPEXTERNAL_PATH}/include/icu")

# Setup link dirs.
# LINK_DIRECTORIES("${BP_EXTERNAL}/lib")

# Build our target as a static lib.
YBT_BUILD(LIBRARY_STATIC ${libName})
ADD_DEPENDENCIES(${libName}_s BPUtils_s)

# This is (currently) the place to specify .libs upon which we depend whose
# names do not end in *_s
IF (UNIX)
    TARGET_LINK_LIBRARIES(${libName}_s sicui18n sicuuc sicudata)
ENDIF ()
