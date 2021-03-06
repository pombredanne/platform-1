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

#include "PusherMan.h"
#include <iostream>
#include "bphttp/HttpRequest.h"
#include "bphttp/HttpSyncTransaction.h"
#include "BPUtils/bpfile.h"

#define PUBLISH_BASE_PATH "/v4/internal/service/"

using namespace bp;
using namespace bp::http;
using namespace bp::http::client;
using namespace std;


bool
pushFile(boost::filesystem::path file, string baseurl,
         string serviceName, string serviceVersion,
         string platform)
{
    baseurl.append(PUBLISH_BASE_PATH);
    baseurl.append(serviceName);
    baseurl.append("/");    
    baseurl.append(serviceVersion);        
    baseurl.append("/");    
    baseurl.append(platform);            

    RequestPtr req;
    try {
        req.reset(new Request(Method::HTTP_POST, baseurl));
    } catch (const bp::url::ParseError& e) {
        cerr << "unable to parse " << baseurl << ": " << e.what() << endl;
        return false;
    }

    // now let's populate the request body
    req->body.fromPath(file);

    SyncTransactionPtr tran = SyncTransaction::alloc( req );
    SyncTransaction::FinalStatus results;

    cout << "publishing service: "
         << "(" << bp::file::size(file) << " bytes) "
         << "to " << baseurl << "..." << endl;

    ResponsePtr resp = tran->execute( results );

    bool succeeded = false;
    switch (results.code)
    {
        case SyncTransaction::FinalStatus::eOk:
            if (resp->status.code() == http::Status::OK) {
                cout << "server returned: " << resp->status.toString() << endl;
                succeeded = true;
            } else {
                cerr << "server returned: " << resp->status.toString() << endl;
            }
            break;
        case SyncTransaction::FinalStatus::eTimedOut:
            cerr << "transaction timed out." << endl;
            break;
        case SyncTransaction::FinalStatus::eCancelled:
            cerr << "transaction was cancelled." << endl;
            break;
        case SyncTransaction::FinalStatus::eError:
            cerr << "transaction had error: " << results.message << endl;
            break;
        default:
            cerr << "transaction returned unrecognized status." << endl;
            break;
    }
   
    return succeeded;
}

