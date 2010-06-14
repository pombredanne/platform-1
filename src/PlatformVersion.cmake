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
#
#  Here is the single source of truth for the BrowserPlus
#  platform version number.  This file sets up a bunch
#  of cmake variables which BuildSDK.cmake uses in 
#  CONFIGURE_FILE statements. 

# A bunch of variables for use in .erb files are defined.  
# Array form is suitable for  ruby/javascript, e.g. "v1", "v2", "v3" 
#
# Generally useful variables are:
#   VersionString - our version, e.g 1.1.0
#   MimeType - our mimetype
#   ActiveXGuid - our ActiveX control's CLSID
#   BackwardCompatibleMimeTypesArray - previous mimetypes which we support
#   MimeTypes - all mimetypes which we support
#   MimeTypesArray - all mimetypes which we support

##########################################################################
# Start edits here
########################################################################## 

# The major/minor/micro of this version
#
SET(BrowserPlusPlatform_MAJOR_VERSION 2)
SET(BrowserPlusPlatform_MINOR_VERSION 10)
SET(BrowserPlusPlatform_MICRO_VERSION 1)

# Previous mimetypes which we also support as a CMAKE list
# Syntax is:
#  SET(BackwardCompatibleMimeTypes 
#        mimetype1 mimetype2 ... mimetypeN)
#
SET(BackwardCompatibleMimeTypes)

# activeX control's clsid and typelib guid
# clsid (ActiveXGuid) should be changed EVERY TIME YOU BUMP PLATFORM VERSION
SET(ActiveXGuid "62F85BAB-32C2-4CEB-BA69-A276551E868F")
SET(TypeLibGuid "399F26B4-E0C6-4345-8AD6-7AC1D86DAAA5") 

# guid for control panel item on vista and later
SET(ControlPanelGuid "30E4F6D0-8773-492d-9AB6-444A4F671459")

########################################################################
# From here down, you shouldn't have to change anything.  Should you decide to change something,
# be forewarned that the quoting to satifsy ruby, sh, and the Windows shell is very tricky.  You will waste
# hours and suffer endless aggravation.
########################################################################

SET(NoEditWarning "THIS FILE IS GENERATED BY THE BUILD SYSTEM.  EDIT THE CORRESPONDING .erb FILE.")

ADD_DEFINITIONS(-DBP_VERSION_MAJOR=${BrowserPlusPlatform_MAJOR_VERSION}
                -DBP_VERSION_MINOR=${BrowserPlusPlatform_MINOR_VERSION}
                -DBP_VERSION_MICRO=${BrowserPlusPlatform_MICRO_VERSION})
                
SET(VersionString
    "${BrowserPlusPlatform_MAJOR_VERSION}.${BrowserPlusPlatform_MINOR_VERSION}.${BrowserPlusPlatform_MICRO_VERSION}")    

SET(WindowsVersionInfo 
    "${BrowserPlusPlatform_MAJOR_VERSION},${BrowserPlusPlatform_MINOR_VERSION},${BrowserPlusPlatform_MICRO_VERSION},0")

# Our mimetype
#
SET(MimeType application/x-yahoo-browserplus_${BrowserPlusPlatform_MAJOR_VERSION})
SET(FullMimeType application/x-yahoo-browserplus_${BrowserPlusPlatform_MAJOR_VERSION}.${BrowserPlusPlatform_MINOR_VERSION}.${BrowserPlusPlatform_MICRO_VERSION})

# Plugin name and description 
SET(PluginName "BrowserPlus (from Yahoo!) v${VersionString}")
SET(PluginDescription "BrowserPlus -- Improve your browser! -- http://browserplus.yahoo.com/")

# Build list/array for supported previous mimetypes.  
#
SET(BackwardCompatibleMimeTypesArray)
FOREACH(v ${BackwardCompatibleMimeTypes})
    IF (NOT BackwardCompatibleMimeTypesArray)
        SET(BackwardCompatibleMimeTypesArray "\"${v}\"")
    ELSE()
        SET(BackwardCompatibleMimeTypesArray "${BackwardCompatibleMimeTypesArray}, \"${v}\"")
    ENDIF()
ENDFOREACH()

# All Service API versions that this platform supports
SET(ServiceAPIVersionsSupported "4,5")

# All mimetypes that we support
#
IF (NOT BackwardCompatibleMimeTypesArray)
    SET(MimeTypes ${MimeType} ${FullMimeType})
    SET(MimeTypesArray "\"${MimeType}\", \"${FullMimeType}\"")
ELSE()
    SET(MimeTypes ${MimeType} ${FullMimeType} ${BackwardCompatibleMimeTypes})
    SET(MimeTypesArray "\"${MimeType}\", \"${FullMimeType}\", 
        ${BackwardCompatibleMimeTypesArray}")
ENDIF()

# Mimetypes string for windows npapi plugin
#
SET(WindowsNPAPIMimeTypes)

# Setup windows/mac mimetype resources.
#
FOREACH(v ${MimeTypes})
    IF (NOT WindowsNPAPIMimeTypes)
       SET(WindowsNPAPIMimeTypes "${v}")
    ELSE()
        SET(WindowsNPAPIMimeTypes "${WindowsNPAPIMimeTypes}|${v}")
    ENDIF() 
    
    IF (NOT MacPlistMimeTypes)
        SET(MacPlistMimeTypes
            "<key>${v}</key>
             <dict>
                <key>WebPluginTypeDescription</key>
                <string>Yahoo! BrowserPlus</string>
             </dict>")
    ELSE()
        SET(MacPlistMimeTypes
            "${MacPlistMimeTypes}
            <key>${v}</key>
            <dict>
                <key>WebPluginTypeDescription</key>
                <string>Yahoo! BrowserPlus</string>
                </dict>")
    ENDIF()

    IF (NOT MacSTRResources)
       SET(MacSTRResources
           "\"${v}\",
           \"\"")
    ELSE()
        SET(MacSTRResources
            "${MacSTRResources},
            \"${v}\",
            \"\"")
    ENDIF()
        
ENDFOREACH()

MESSAGE("!! Platform ${VersionString} configured to support:")
MESSAGE("!!    mimetypes: ${MimeTypes}") 
MESSAGE("!!    service API versions: ${ServiceAPIVersionsSupported}") 



