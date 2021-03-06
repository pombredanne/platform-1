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
 *  BPLog.cpp
 *
 *  Implements convenience functions for setting up the logging system.
 *  
 *  Created by David Grigsby on 9/20/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLog.h"

#include "BPLogConsoleAppender.h"
#include "BPLogDebuggerAppender.h"
#include "BPLogFileAppender.h"
#include "BPLogLogger.h"
#include "BPLogStockLayouts.h"
#include "BPUtils/bpstrutil.h"


namespace bp {
namespace log {


static Logger s_rootLogger;


void removeAllAppenders( Logger& logger )
{
    logger.removeAllAppenders();
}


Logger& rootLogger()
{
    return s_rootLogger;
}


void setLogLevel( const Level& level,
                  Logger& logger )
{
    logger.setLevel( level );
}


void setupLogToConsole( const Level& level,
                        const std::string& sConsoleTitle,
                        const TimeFormat& timeFormat,
                        const std::string& sLayout,
                        Logger& logger )
{
    LayoutPtr layout( layoutFromString( sLayout ) );
    layout->setTimeFormat( timeFormat );

    ConsoleAppenderPtr apdr( new ConsoleAppender( layout, sConsoleTitle ) );
    apdr->setThreshold( level );
    logger.addAppender( apdr ); 
}


void setupLogToDebugger( const Level& level,
                         const TimeFormat& timeFormat,
                         const std::string& sLayout,
                         Logger& logger )
{
    LayoutPtr layout( layoutFromString( sLayout ) );
    layout->setTimeFormat( timeFormat );

    DebuggerAppenderPtr apdr( new DebuggerAppender( layout ) );
    apdr->setThreshold( level );
    logger.addAppender( apdr ); 
}


void setupLogToFile( const boost::filesystem::path& logFilePath,
                     const Level& level,
                     const FileMode& fileMode,
                     const TimeFormat& timeFormat,
                     const std::string& sLayout,
                     unsigned int nRolloverSizeKB,
                     Logger& logger )
{
    LayoutPtr layout( layoutFromString( sLayout ) );
    layout->setTimeFormat( timeFormat );

    FileAppenderPtr apdr( new FileAppender( logFilePath, layout,
                                            fileMode, nRolloverSizeKB ) );
    apdr->setThreshold( level );
    logger.addAppender( apdr ); 
}


ScopeLogger::ScopeLogger( const char* cszFile, const char* cszFunc, int nLine )
{
    // We'll treat scope messages as debug level.
    if (bp::log::rootLogger().isLevelEnabled( bp::log::LEVEL_DEBUG ))
    {
        // Lazily init this member for performance.
        m_location = LocationInfo( cszFile, cszFunc, nLine );
        
        std::string sMsg = "> Entry.";
        bp::log::rootLogger()._forcedLog( bp::log::LEVEL_DEBUG, sMsg,
                                          m_location );
    } 
}


ScopeLogger::~ScopeLogger()
{
    // We'll treat scope messages as debug level.
    if (bp::log::rootLogger().isLevelEnabled( bp::log::LEVEL_DEBUG ))
    {
        std::string sMsg = "< Exit.";
        bp::log::rootLogger()._forcedLog( bp::log::LEVEL_DEBUG, sMsg,
                                          m_location );
    } 
}


LayoutPtr layoutFromString( const std::string& sLayout )
{
    if (bp::strutil::isEqualNoCase( sLayout, "raw" ) )
    {
        return LayoutPtr( new RawLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "source" ))
    {
        return LayoutPtr( new SourceLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "standard" ) )
    {
        return LayoutPtr( new StandardLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "ThrdLvlFuncMsg" ) )
    {
        return LayoutPtr( new ThrdLvlFuncMsgLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "TimeLvlMsg" ) )
    {
        return LayoutPtr( new TimeLvlMsgLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "FuncMsg" ) )
    {
        return LayoutPtr( new FuncMsgLayout );
    }
    else
    {
        return LayoutPtr( new StandardLayout );
    }
}


} // log
} // bp
