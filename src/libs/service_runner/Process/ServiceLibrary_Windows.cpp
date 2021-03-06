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
 * ServiceLibrary windows-specific implementations.
 */

#include "ServiceLibrary.h"
#include <Windows.h>
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"

using namespace ServiceRunner;

void * 
ServiceLibrary::dlopenNP(const boost::filesystem::path & path)
{
    // At the time we load a library, it must be able to find
    // DLLs that it uses.  Additionally, the service should not be
    // able to find any dlls that BrowserPlusCore uses.  To attain
    // this goal, we use LoadLibraryEx with the
    // LOAD_WITH_ALTERED_SEARCH_PATH argument
  
    // determine the target directory
    void * libptr = (void *) LoadLibraryExW(bp::file::nativeString(path).c_str(),
                                            (HANDLE) NULL,
                                            LOAD_WITH_ALTERED_SEARCH_PATH);
    if (libptr == NULL) {
        BPLOG_ERROR_STRM("LoadLibraryExW for " << path.string() << " failed: "
                         << bp::error::lastErrorString());

        bp::file::FileInfo fi;
        if (statFile(path, fi)) {
            BPLOG_ERROR_STRM("Size of " << path.string() << ": "
                             << fi.sizeInBytes << " bytes.");
        }
    }
    
    return libptr;
}

void
ServiceLibrary::dlcloseNP(void * handle)
{
    FreeLibrary((HMODULE) handle);
}

void *
ServiceLibrary::dlsymNP(void * handle, const char * sym)
{
    return (void *) GetProcAddress((HMODULE) handle, sym);
}
