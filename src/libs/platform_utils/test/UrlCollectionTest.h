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
 * UrlCollectionTest.h
 * A test of the bp::URLCollection component
 *
 * Created by Lloyd Hilaiel on Tues Apr 29
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __URLCOLLECTIONTEST_H__
#define __URLCOLLECTIONTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "BPUtils/bpfile.h"

class URLCollectionTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(URLCollectionTest);
    CPPUNIT_TEST(bogusPath);
    CPPUNIT_TEST(malformedJSON);
    CPPUNIT_TEST(fullTest);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    
  protected:
    void bogusPath();
    void malformedJSON();
    void fullTest();
	boost::filesystem::path m_path;
};

#endif
