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
 * An abstraction around the dynamic library that composes a service.
 * abstracts dlloading and all interaction.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/15
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "ServiceLibrary_v4.h"
#include "BPUtils/bpstrutil.h"
#include "V4ObjectConverter.h"

using namespace std;
using namespace std::tr1;
using namespace ServiceRunner;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;

// BEGIN call-in points for dynamically loaded services

/**
 * a structure holding data from instance runloop back to 
 * main runloop
 */
struct InstanceResponse_v4
{
    enum { T_Results, T_Error, T_CallBack, T_Prompt } type;

    // common
    unsigned int tid;

    // results
    bp::Object * o;

    // error
    std::string error;
    std::string verboseError;    

    // callback
    long long int callbackId;

    // prompt
    std::string dialogPath;
    sapi_v4::BPUserResponseCallbackFuncPtr responseCallback;
    void * responseCookie;
    unsigned int promptId;

    InstanceResponse_v4()
        : type(T_Results), tid(0), o(NULL), callbackId(0),
          dialogPath(), responseCallback(NULL),
          responseCookie(NULL), promptId(0)  {  }
    ~InstanceResponse_v4() { if (o) delete o; }
};

static ServiceLibrary_v4 * s_libObjectPtr = NULL;
    
void
ServiceLibrary_v4::postResultsFunction(unsigned int tid,
                                       const struct sapi_v4::BPElement_t * results)
{
    InstanceResponse_v4 * ir = new InstanceResponse_v4;
    ir->type = InstanceResponse_v4::T_Results;
    ir->tid = tid;
    ir->o = (results ? sapi_v4::v4ToBPObject(results) : NULL);
    s_libObjectPtr->hop(ir);
}

void
ServiceLibrary_v4::postErrorFunction(unsigned int tid,
                                     const char * error,
                                     const char * verboseError)
{
    InstanceResponse_v4 * ir = new InstanceResponse_v4;
    ir->type = InstanceResponse_v4::T_Error;
    ir->tid = tid;
    if (error) ir->error.append(error);
    if (verboseError) ir->verboseError.append(verboseError);    
    s_libObjectPtr->hop(ir);
}

void
ServiceLibrary_v4::logFunction(unsigned int level, const char * fmt, ...)
{
// copying va_args.
#ifndef va_copy
# ifdef __va_copy
#  define va_copy(DEST,SRC) __va_copy((DEST),(SRC))
# else
#  define va_copy(DEST,SRC) ((DEST) = (SRC))
# endif
#endif
    
    va_list ap;
    va_start(ap, fmt);
    
    // how big a string do we need?
    char* buf = NULL;
    unsigned int sz = 0;
    va_list test;
    va_copy(test, ap);
#ifdef WIN32
    // ok to disable warning, this call doesn't actually write any data
#pragma warning(push)
#pragma warning(disable:4996)
#endif
    sz = vsnprintf(buf, 0, fmt, test);
#ifdef WIN32
#pragma warning(pop)
#endif
    va_end(test);

    // limit size of log entry.  no evil corelets
    // trying to get us to malloc a gigabyte!
    if (sz > 4095) {
        BPLOG_WARN_STRM("log entry reduced from " << sz << " to 4096 bytes");
        sz = 4095;  // a sz++ will be done below, yielding max of 4k
    }
    
    if (sz > 0) {
        sz++;
        buf = new char[sz];
        if (!buf) {
            BPLOG_ERROR("unable to allocate char buffer");
            return;
        }
#ifdef WIN32
        sz = vsnprintf_s(buf, sz, _TRUNCATE, fmt, ap);
#else
        sz = vsnprintf(buf, sz, fmt, ap);
#endif
    }

    // Handle buf=NULL.
    std::string str = bp::strutil::safeStr(buf);

    s_libObjectPtr->logServiceEvent(level, str);
    
    delete[] buf;
    va_end(ap);
}


static bp::log::Level BPLevelFromServiceLevel(unsigned int level)
{
    switch (level)
    {
        case BP_DEBUG:  return bp::log::LEVEL_DEBUG;
        case BP_INFO:   return bp::log::LEVEL_INFO;
        case BP_WARN:   return bp::log::LEVEL_WARN;
        case BP_ERROR:  return bp::log::LEVEL_ERROR;
        case BP_FATAL:  return bp::log::LEVEL_FATAL;
        default:        return bp::log::LEVEL_ALL;
    }
}


///////////////////////////////////////////////////////////////////////////
// Setup logging done by the service proper.  That is, logging done by
// the actual service dll through the service api, as oppposed to logging done
// by the "harness" exe.  Note this also distinguishes code written by
// service authors from platform code written by the browserplus team.
// 
// If the serviceLogMode setting is "combined" (default) this method
// will take no action and logging from the harness and the dll will
// both go to the destination setup by Process.cpp: setupLogging().
// 
// If the serviceLogMode setting is "separate", this method will setup
// a separate destination for logging from the dll.
//
void
ServiceLibrary_v4::setupServiceLogging()
{
    bp::log::Configurator cfg;
    cfg.loadConfigFile();

    // If "combined" mode, nothing for us to do.
    m_serviceLogMode = cfg.getServiceLogMode();
    if (m_serviceLogMode == bp::log::kServiceLogCombined) {
        return;
    }

    bp::log::Destination dest = cfg.getDestination();
    if (dest == bp::log::kDestFile) {
        bfs::path p(name().empty() ? "service" : name());
        p.replace_extension("log");
        cfg.setPath(p);
    }
	else if (dest == bp::log::kDestConsole) {
        string nameVer = description().nameVersionString();
        string consoleTitle = nameVer.empty() ? "BrowserPlus Service"
                                              : nameVer + " Service";
        cfg.setConsoleTitle(consoleTitle);
    }
    else {
    }
    
    // Configure the logging system.
    cfg.configure(m_serviceLogger);
}


//////////////////
// Log events from the dll over the Service API come in here.
void
ServiceLibrary_v4::logServiceEvent(unsigned int level, const std::string& msg)
{
    static bool s_firstTime = true;

    if (s_firstTime) {
        setupServiceLogging();
        s_firstTime = false;
    }

    // Convert the level.
    bp::log::Level bpLevel = BPLevelFromServiceLevel(level);
    
    if (m_serviceLogMode == bp::log::kServiceLogSeparate)
    {
        // Send event to "separate" destination.
        BPLOG_LEVEL_LOGGER(bpLevel, m_serviceLogger, msg);
    }
    else
    {
        // Prepend service name/version to the log message, if we know it.
        string nameVer = description().nameVersionString();
        string logMsg = nameVer.length() ? string("(") + nameVer + ") " + msg
                                         : msg;

        // Send event to "combined" destination.
        BPLOG_LEVEL(bpLevel, logMsg);
    }
}


void
ServiceLibrary_v4::invokeCallbackFunction(unsigned int tid,
                                       long long int callbackHandle,
                                       const struct sapi_v4::BPElement_t * results)
{
    InstanceResponse_v4 * ir = new InstanceResponse_v4;
    ir->type = InstanceResponse_v4::T_CallBack;
    ir->tid = tid;
    ir->callbackId = callbackHandle;
    ir->o = (results ? sapi_v4::v4ToBPObject(results) : NULL);
    s_libObjectPtr->hop(ir);
}

unsigned int
ServiceLibrary_v4::promptUserFunction(
    unsigned int tid,
    const char * utf8PathToHTMLDialog,
    const sapi_v4::BPElement * args,
    sapi_v4::BPUserResponseCallbackFuncPtr responseCallback,
    void * cookie)
{
    // management of unique prompt ids
    static unsigned int s_promptId = 1000;
    static bp::sync::Mutex s_lock;

    // allocate a new unique prompt id
    unsigned int cpid;
    {
        bp::sync::Lock _lock(s_lock);
        cpid = s_promptId++;
    }

    InstanceResponse_v4 * ir = new InstanceResponse_v4;
    ir->type = InstanceResponse_v4::T_Prompt;
    ir->tid = tid;
    if (utf8PathToHTMLDialog) ir->dialogPath.append(utf8PathToHTMLDialog);
    ir->o = (args ? sapi_v4::v4ToBPObject(args) : NULL);
    ir->responseCallback = responseCallback;
    ir->responseCookie = cookie;
    ir->promptId = cpid;

    s_libObjectPtr->hop(ir);

    return cpid;
}
// END call-in points for dynamically loaded services


// BEGIN runloop hooks for instance threads
struct InstanceState_v4 
{
    // arguments to allocate
    bp::Map context;
    std::string name;
    std::string version;
    // instance cookie retured by service, to be passed into subsequent
    // calls
    void * cookie;
    // attachment id.  Largely obsolete, a remnant from a time when
    // multiple different dependent services could be loaded into the same
    // process.
    unsigned int attachID;
    // the function table, call-in points to the service
    const sapi_v4::BPPFunctionTable * funcTable;
};

static void
onThreadStartFunc(void * c)
{
    InstanceState_v4 * is = (InstanceState_v4 *) c;

    if (is->funcTable->allocateFunc != NULL)
    {
        sapi_v4::BPElement * e = sapi_v4::v5ElementToV4(is->context.elemPtr());

        int rv = is->funcTable->allocateFunc(
            &(is->cookie), is->attachID, e);

        sapi_v4::freeDynamicV4Element(e);

        if (rv != 0) {
            BPLOG_ERROR_STRM(
                "Failed to allocate instance of "
                << is->name << " (" << is->version
                << ") - BPPAllocate returns non-zero code: "
                << rv);

            // in debug mode we'll bring the process down.
            // TODO: eventually we should be getting this error back
            // to BrowserPlusCore - and a single instance allocation
            // should not bring down the whole process.
            BPASSERT(rv == 0);
        }
    }
}

/**
 * events that can be sent to an instance runloop include:
 * 1. function invocation
 * 2. prompt user results
 */
struct InstanceEvent_v4
{
    enum { T_Invoke, T_PromptResults } type;
    
    // for function invocation
    unsigned int tid;
    bp::Map args;
    std::string name;

    // for prompt results
    unsigned int promptId;
    sapi_v4::BPUserResponseCallbackFuncPtr callback;
    void * cookie;
    bp::Object * results;
};

static void
processEventFunc(void * c, bp::runloop::Event e)
{
    InstanceState_v4 * is = (InstanceState_v4 *) c;
    InstanceEvent_v4 * ie = (InstanceEvent_v4 *) e.payload();
    BPASSERT(ie != NULL);
    
    if (ie->type == InstanceEvent_v4::T_Invoke) {
        if (is->funcTable->invokeFunc != NULL)
        {
            sapi_v4::BPElement * e = sapi_v4::v5ElementToV4(ie->args.elemPtr());

            is->funcTable->invokeFunc(is->cookie,
                                      ie->name.c_str(),
                                      ie->tid,
                                      e);

            sapi_v4::freeDynamicV4Element(e);
        }
    } else if (ie->type == InstanceEvent_v4::T_PromptResults) {
        if (ie->callback) {
            sapi_v4::BPElement * r = NULL;
            if (ie->results) r = sapi_v4::v5ElementToV4(ie->results->elemPtr());
            ie->callback(ie->cookie, ie->promptId, r);
            if (r) sapi_v4::freeDynamicV4Element(r);
        }
    }

    if (ie) delete ie;
}

static void
onThreadEndFunc(void * c)
{
    InstanceState_v4 * is = (InstanceState_v4 *) c;

    if (is->funcTable->destroyFunc != NULL)
    {
        is->funcTable->destroyFunc(is->cookie);
    }

    delete is;
}
// END runloop hooks for instance threads

// the static callback function table
const void *
ServiceLibrary_v4::getFunctionTable()
{
    static sapi_v4::BPCFunctionTable * table = NULL;
    static sapi_v4::BPCFunctionTable tableData;
    if (table != NULL) return table;
    memset((void *) &tableData, 0, sizeof(tableData));
    tableData.postResults = postResultsFunction;
    tableData.postError = postErrorFunction;
    tableData.log = logFunction;
    tableData.invoke = invokeCallbackFunction;
    tableData.prompt = promptUserFunction;

    table = &tableData;
    return (void *) table;
}

ServiceLibrary_v4::ServiceLibrary_v4() :
    m_currentId(1), m_attachId(0), m_funcTable(NULL),
    m_desc(), m_coreletAPIVersion(0), m_instances(), m_listener(NULL),
    m_promptToTransaction(), 
    m_serviceLogMode( bp::log::kServiceLogCombined ), m_serviceLogger()
{
    s_libObjectPtr = this;
}
        
ServiceLibrary_v4::~ServiceLibrary_v4()
{
    // deallocate all instances
    while (m_instances.size() > 0)
    {
        std::map<unsigned int,
            shared_ptr<bp::runloop::RunLoopThread> >::iterator it;

        it = m_instances.begin();
        it->second->stop();
        it->second->join();
        m_instances.erase(it);
    }

    // detach dependent services
    if (m_summary.type() == bp::service::Summary::Dependent) {
        const sapi_v4::BPPFunctionTable * table =
            (const sapi_v4::BPPFunctionTable *) m_funcTable;
        if (table && table->detachFunc) {
            table->detachFunc(m_attachId);
        }
    }
    
    // shutdown and unload the library
    shutdownService(true);

    s_libObjectPtr = NULL;
}

std::string
ServiceLibrary_v4::version()
{
    return m_desc.versionString();
}

std::string
ServiceLibrary_v4::name()
{
    return m_desc.name();
}

// load the service
bool
ServiceLibrary_v4::load(const bp::service::Summary &summary,
                        const bp::service::Summary &provider,
                        void * functionTable)
{
    const sapi_v4::BPPFunctionTable * funcTable = (const sapi_v4::BPPFunctionTable *) functionTable;
    

    bool success = true;
    // meaningful when success == false;
    bool callShutdown = true;
    
    BPASSERT(m_funcTable == NULL);    

    m_summary = summary;
    m_funcTable = funcTable;

    // now let's determine the path to the shared library.  For
    // dependent corelets this will be extracted from the manifest
    bfs::path path;
    bfs::path servicePath;
    if (m_summary.type() == bp::service::Summary::Dependent) {
        servicePath = provider.path();
        path = provider.path() / provider.serviceLibraryPath();
    } else {
        servicePath = m_summary.path();
        path = m_summary.path() / m_summary.serviceLibraryPath();
    }
    path = bpf::absolutePath(path);
    
    BPLOG_INFO_STRM("loading v4 service library: " << path.filename());

    {
        // set up the parameters to the initialize function
        bp::Map params;
        params.add("CoreletDirectory", new bp::String(servicePath.generic_string()));

        if (funcTable == NULL || funcTable->initializeFunc == NULL)
        {
            BPLOG_WARN_STRM("invalid v4 corelet, NULL initialize function (" << path << ")");
            success = false;
            // still call shutdown
        } else {
            BPASSERT(funcTable->coreletAPIVersion == 4);

            m_coreletAPIVersion = funcTable->coreletAPIVersion;
            
            const sapi_v4::BPCoreletDefinition * def = NULL;
            
            sapi_v4::BPElement * p = NULL;
            p = sapi_v4::v5ElementToV4(params.elemPtr());
            def = funcTable->initializeFunc(
                (const sapi_v4::BPCFunctionTable *) getFunctionTable(),
                p);
            if (p) sapi_v4::freeDynamicV4Element(p);

                    
            if (def == NULL)
            {
                BPLOG_WARN_STRM("invalid corelet, NULL return from "
                                << "initialize function ("
                                << path << ")");
                success = false;
                // don't call shutdown.  This corelet has already
                // violated a contract
                callShutdown = false;
            }
            // TODO: It's unnecesary in the dependent case to
            // extract an API definition, because the providers'
            // api will not be exposed.  In this case the m_desc
            // structure will be re-populated below
            // XXX: v4 and v5 service definitions are the same
            else if (!m_desc.fromBPServiceDefinition((const ::BPServiceDefinition *) def))
            {
                BPLOG_WARN_STRM("couldn't populate Description "
                                "from returned corelet structure ");
                success = false;
                callShutdown = true;
            }
        }
    }
    
    // now if we've successfully initialized, and this is a dependent
    // service, let's attach!
    if (success && m_summary.type() == bp::service::Summary::Dependent &&
        funcTable->attachFunc != NULL)
    {
        m_attachId = 1000;

        // set up the parameters to the attach function
        bp::Map params;
        params.add("CoreletDirectory",
                   new bp::String(m_summary.path().generic_string()));

        // add in arguments from dependent manifest
        std::map<std::string, std::string> summaryArgs =
            m_summary.arguments();

        std::map<std::string, std::string>::iterator it;

        bp::Map * sargs = new bp::Map;
        for (it = summaryArgs.begin(); it != summaryArgs.end(); it++)
        {
            sargs->add(it->first.c_str(), new bp::String(it->second));
        }
        params.add("Parameters", sargs);

        boost::filesystem::path curdir;
#ifdef WIN32
        wchar_t* cwd = _wgetcwd(NULL, 0);
#else
        char* cwd = getcwd(NULL,0);
#endif
        if (cwd) {
            curdir = cwd;
            free(cwd);
        }
        std::string paramStr = params.toJsonString();
        BPLOG_INFO_STRM("attach to " << path << " with " << paramStr 
                        << ", cwd = " << curdir);

        const sapi_v4::BPCoreletDefinition * def = NULL;
        sapi_v4::BPElement * p = sapi_v4::v5ElementToV4(params.elemPtr());
        def = funcTable->attachFunc(m_attachId, p);
        if (p) sapi_v4::freeDynamicV4Element(p);

        if (def == NULL) {
            BPLOG_WARN_STRM("attachFunc returns NULL description");
            if (funcTable->detachFunc) {
                funcTable->detachFunc(m_attachId);
            }
            success = false;
            callShutdown = true;
        // XXX: v4 and v5 service definitions are the same
        } else if (!m_desc.fromBPServiceDefinition((const ::BPServiceDefinition *) def)) {
            BPLOG_WARN_STRM("couldn't populate Description "
                            "from structure returned from attach function");
            success = false;
            callShutdown = true;
        }
    }
    
    if (!success) shutdownService(callShutdown);

    return success;
}

// shutdown the corelet, NULL out m_handle, and m_def
void
ServiceLibrary_v4::shutdownService(bool callShutdown)
{
    const sapi_v4::BPPFunctionTable * table = (const sapi_v4::BPPFunctionTable *) m_funcTable;
    
    // first call shutdown
    if (callShutdown && table != NULL && table->shutdownFunc != NULL) {
        table->shutdownFunc();
    }
    m_funcTable = NULL;
}

unsigned int
ServiceLibrary_v4::allocate(std::string uri, boost::filesystem::path dataDir,
                            boost::filesystem::path tempDir, std::string locale,
                            std::string userAgent, unsigned int clientPid)
{
    unsigned int id = m_currentId++;

    // allocate runloop thread and add to map
    shared_ptr<bp::runloop::RunLoopThread>
        rlthr(new bp::runloop::RunLoopThread);

    // now let's allocate and populate the instance's state structure
    // this will be freed by the thread in the onThreadEndFunc
    InstanceState_v4 * is = new InstanceState_v4;
    is->name = m_summary.name();
    is->version = m_summary.version();

    // set up (v4 sytle) arguments - they weren't AT ALL consistent :(
    is->context.add("uri", new bp::String(uri));
    is->context.add("data_dir", new bp::String(bpf::nativeUtf8String(dataDir)));
    is->context.add("temp_dir", new bp::String(bpf::nativeUtf8String(tempDir)));
    is->context.add("locale", new bp::String(locale));
    is->context.add("userAgent", new bp::String(userAgent));
    is->context.add("clientPid", new bp::Integer(clientPid));
    is->context.add("corelet_dir", new bp::String(m_summary.path().generic_string()));
    is->context.add("service_dir", new bp::String(m_summary.path().generic_string()));

    is->cookie = NULL;
    is->attachID = m_attachId; 
    is->funcTable = (const sapi_v4::BPPFunctionTable *) m_funcTable;

    rlthr->setCallBacks(onThreadStartFunc, (void *) is,
                        onThreadEndFunc, (void *) is,
                        processEventFunc, (void *) is);

    m_instances[id] = rlthr;
    
    rlthr->run();

    return id;
}

void
ServiceLibrary_v4::destroy(unsigned int id)
{
    std::map<unsigned int,
             shared_ptr<bp::runloop::RunLoopThread> >::iterator it;

    it = m_instances.find(id);
    if (it != m_instances.end()) {
        it->second->stop();
        it->second->join();
        m_instances.erase(it);
    }
}

bool
ServiceLibrary_v4::invoke(unsigned int id, unsigned int tid,
                          const std::string & function,
                          const bp::Object * arguments,
                          std::string & err)
{
    // first we'll add the transaction to the transaction map
    // (removed in postError or postResults functions)
    beginTransaction(tid, id);

    // argument validation. does function exist?  are parameters
    // correct?
    bp::service::Function funcDesc;
    if (!m_desc.getFunction(function.c_str(), funcDesc))
    {
        std::stringstream ss;
        ss << "no such function: " << function;
        err = ss.str();
        postErrorFunction(tid, "bp.noSuchFunction", err.c_str());
        return true;
    }

    if (arguments && arguments->type() != BPTMap) {
        err.append("arguments must be a map");
        postErrorFunction(tid, "bp.invokeError", err.c_str());
        return true;
    }

    // now we've got the arguments and description, we're in a
    // position where we can validate the args.
    err = bp::service::validateArguments(funcDesc, (bp::Map *) arguments);
    
    if (!err.empty()) {
        postErrorFunction(tid, "bp.invokeError", err.c_str());
        return true;
    }
    
    // finally, does the specified instance exist?
    std::map<unsigned int,
        shared_ptr<bp::runloop::RunLoopThread> >::iterator it;
    it = m_instances.find(id);

    if (it == m_instances.end()) {
        std::stringstream ss;
        ss << "no such instance: " << id;
        err = ss.str();
        postErrorFunction(tid, "bp.invokeError", err.c_str());
        return true;
    }
    
    // good to go!  now we're ready to actually invoke the function!

    InstanceEvent_v4 * ie = new InstanceEvent_v4;
    ie->type = InstanceEvent_v4::T_Invoke;
    ie->tid = tid;
    ie->name = function;
    if (arguments && arguments->type() == BPTMap) {
        ie->args = *((bp::Map *) arguments);
    }

    if (!it->second->sendEvent(bp::runloop::Event(ie))) {
        delete ie;
        err.append("couldn't send event to instance runloop");
        postErrorFunction(tid, "bp.invokeError", err.c_str());
    }

    return true;
}


int
ServiceLibrary_v4::installHook(const boost::filesystem::path&,
                               const boost::filesystem::path&)
{
    return 0;
}


int
ServiceLibrary_v4::uninstallHook(const boost::filesystem::path&,
                                 const boost::filesystem::path&)
{
    return 0;
}


void
ServiceLibrary_v4::promptResponse(unsigned int promptId,
                               const bp::Object * arguments)
{
    PromptContext ctx;
    unsigned int instance;
    std::map<unsigned int,
        shared_ptr<bp::runloop::RunLoopThread> >::iterator it;
    
    if (!findContextFromPromptId(promptId, ctx)) {
        BPLOG_ERROR_STRM("prompt response with unknown prompt ID: "
                         << promptId);
    } else if (!findInstanceFromTransactionId(ctx.tid, instance)) {
        BPLOG_ERROR_STRM("prompt response associated with unknown transaction "
                         "id: " << ctx.tid);
    } else if ((it = m_instances.find(instance)) == m_instances.end()) {
        BPLOG_ERROR_STRM("prompt response associated with unknown instance "
                         "id: " << instance);
    } else {
        // we got all we need! let's pass it to the correct instance
        // runloop
        InstanceEvent_v4 * ie = new InstanceEvent_v4;
        ie->type = InstanceEvent_v4::T_PromptResults;
        ie->callback = ctx.cb;
        ie->cookie = ctx.cookie;
        ie->promptId = promptId;

        if (arguments) ie->results = arguments->clone();
        else ie->results = NULL;

        if (!it->second->sendEvent(bp::runloop::Event(ie))) {
            BPLOG_ERROR_STRM("couldn't relay prompt event to correct "
                             "runloop, dropping");
            delete ie;
        }
    }

    // regardless of the outcome, let's ensure we've removed our
    // record of the prompt
    (void) endPrompt(promptId);
}

void
ServiceLibrary_v4::onHop(void * context)
{
    InstanceResponse_v4 * ir = (InstanceResponse_v4 *) context;
    BPASSERT(ir != NULL);

    // all InstanceResponses have a populated tid, all need an
    // instance id to go with it.
    unsigned int instance;
        
    if (!findInstanceFromTransactionId(ir->tid, instance)) {
        BPLOG_ERROR_STRM("function call from service with bogus tid: "
                         << ir->tid << " - not associated with any instance");
        if (ir) delete ir;        
        return;
    }
    
    if (m_listener) {    
        switch (ir->type) {
            case InstanceResponse_v4::T_Results: {
                m_listener->onResults(instance, ir->tid, ir->o);

                // remove the transaction
                endTransaction(ir->tid);
                break;
            }
            case InstanceResponse_v4::T_Error: {
                m_listener->onError(instance, ir->tid, ir->error,
                                    ir->verboseError);

                // remove the transaction
                endTransaction(ir->tid);
                break;
            }
            case InstanceResponse_v4::T_CallBack: {
                m_listener->onCallback(instance, ir->tid,
                                       ir->callbackId, ir->o);
                break;
            }
            case InstanceResponse_v4::T_Prompt: {
                if (!transactionKnown(ir->tid)) {
                    BPLOG_ERROR_STRM("can't send prompt from, unknown "
                                     "transaction: " << ir->tid);
                } else {
                    // squirrel away what we need to correctly associate
                    // the response with the correct instance
                    beginPrompt(ir->promptId, ir->tid, ir->responseCallback,
                                ir->responseCookie);
                    m_listener->onPrompt(instance, ir->promptId,
                                         bfs::path(ir->dialogPath),
                                         ir->o);
                }
                break;
            }
        }
    }
    
    if (ir) delete ir;
}

void
ServiceLibrary_v4::setListener(IServiceLibraryListener * listener)
{
    m_listener = listener;
}
    
bool
ServiceLibrary_v4::transactionKnown(unsigned int tid)
{
    std::map<unsigned int, unsigned int>::iterator i;
    i = m_transactionToInstance.find(tid);
    return (i != m_transactionToInstance.end());
}


void
ServiceLibrary_v4::beginTransaction(unsigned int tid, unsigned int instance)
{
    if (transactionKnown(tid)) {
        BPLOG_ERROR_STRM("duplicate transaction id detected: " << tid);
    } else {
        m_transactionToInstance[tid] = instance;
    }
}

void
ServiceLibrary_v4::endTransaction(unsigned int tid)
{
    std::map<unsigned int, unsigned int>::iterator i;
    i = m_transactionToInstance.find(tid);
    if (i == m_transactionToInstance.end()) {
        BPLOG_ERROR_STRM("attempt to end unknown transaction: " << tid);
    } else {
        m_transactionToInstance.erase(i);
    }

    // as an added bonus, let's purge record of all prompts associated
    // with this completed transaction
    std::map<unsigned int, PromptContext>::iterator it;
    unsigned int numPurged = 0;
    for (it = m_promptToTransaction.begin();
         it != m_promptToTransaction.end();
         it++)
    {
        if (it->second.tid == tid) {
            numPurged++;
            // erase and restart iteration
            m_promptToTransaction.erase(it);
            it = m_promptToTransaction.begin();
        }
    }

    if (numPurged > 0) {
        BPLOG_INFO_STRM("Transaction ends (" << tid << ") with "
                        << numPurged << " outstanding user prompts");        
    }
}

bool
ServiceLibrary_v4::findInstanceFromTransactionId(unsigned int tid,
                                              unsigned int & instance)
{
    instance = 0;
    
    std::map<unsigned int, unsigned int>::iterator i;
    i = m_transactionToInstance.find(tid);
    if (i == m_transactionToInstance.end()) return false;
    instance = i->second;
    return true;
}

bool
ServiceLibrary_v4::promptKnown(unsigned int promptId)
{
    std::map<unsigned int, PromptContext>::iterator i;
    i = m_promptToTransaction.find(promptId);
    return (i != m_promptToTransaction.end());
}

void
ServiceLibrary_v4::beginPrompt(unsigned int promptId, unsigned int tid,
                               sapi_v4::BPUserResponseCallbackFuncPtr cb,
                               void * cookie)
{
    if (promptKnown(promptId)) {
        BPLOG_ERROR_STRM("duplicate prompt id detected: " << promptId);
    } else {
        PromptContext ctx;
        ctx.tid = tid;
        ctx.cb = cb;
        ctx.cookie = cookie;
        m_promptToTransaction[promptId] = ctx;
    }
}

void
ServiceLibrary_v4::endPrompt(unsigned int promptId)
{
    std::map<unsigned int, PromptContext>::iterator i;
    i = m_promptToTransaction.find(promptId);
    if (i == m_promptToTransaction.end()) {
        BPLOG_ERROR_STRM("attempt to end unknown prompt: " << promptId);
    } else {
        m_promptToTransaction.erase(i);
    }
}

bool
ServiceLibrary_v4::findContextFromPromptId(unsigned int promptId,
                                        PromptContext & ctx)
{
    std::map<unsigned int, PromptContext>::iterator i;
    i = m_promptToTransaction.find(promptId);
    if (i == m_promptToTransaction.end()) return false;
    ctx = i->second;
    return true;
}

unsigned int
ServiceLibrary_v4::apiVersion()
{
    return m_coreletAPIVersion;
}
