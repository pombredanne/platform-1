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
SET(progName ServiceRunner)

SET(${progName}_MAJOR_VERSION 0)
SET(${progName}_MINOR_VERSION 1)
SET(${progName}_LINK_STATIC BPUtils ConsoleLib platform_utils ServiceRunnerLib DistributionClient Permissions)

YBT_BUILD(BINARY ${progName})

ADD_CUSTOM_COMMAND(
  TARGET ${progName} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../support/signing/Combined.crt ${CMAKE_INSTALL_PREFIX}/bin/BrowserPlus.crt
)
