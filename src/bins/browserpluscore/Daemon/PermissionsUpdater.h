/**
 * ***** BEGIN LICENSE BLOCK *****
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is BrowserPlus (tm).
 * 
 * The Initial Developer of the Original Code is Yahoo!.
 * Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * PermissionsUpdater.h - A singleton responsible for periodic update of 
 *                    recently used services
 *
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __PERMISSIONSUPDATER_H__
#define __PERMISSIONSUPDATER_H__


namespace PermissionsUpdater
{
    /**
     * Start up the permissions updater.  should occur once per process
     */
    void startup(unsigned int pollInSeconds);

    /**
     * shut down the permissions updater
     */
    void shutdown();

    /**
     * Returns whether we're currently checking for updates.  
     * This should delay daemon shutdown.
     */
    bool isBusy();
};

#endif
