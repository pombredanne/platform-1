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

/*
 *  FileBrowseDialogs_Windows.h
 */

#include <vector>
#include <windows.h>

#include "BPUtils/bpfile.h"


struct FileOpenDialogParms
{
    HWND parentWnd;
    std::string locale;
    std::wstring title;
    std::wstring filterName;
    std::wstring filterPattern;
    bool chaseLinks;
};


// return false on error.
// User cancel is not considered an error.
bool runFileOpenDialog(const FileOpenDialogParms& parms,
                       std::vector<boost::filesystem::path>& vPaths);

// return false on error.
// User cancel is not considered an error.
bool runFileOpenDialogXP(const FileOpenDialogParms& parms,
                         std::vector<boost::filesystem::path>& vPaths);


struct FileSaveDialogParms
{
    HWND parentWnd;
    std::string locale;
    std::wstring title;
    std::wstring initialName;
};


// return false on error.
// User cancel is not considered an error.
bool runFileSaveDialog(const FileSaveDialogParms& parms,
                       boost::filesystem::path& path);

