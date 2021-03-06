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
 *  bprandom.cpp
 *
 *  Created by Gordon Durand on 3/22/2010.
 *  
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include "bprandom.h"
#include "BPLog.h"

// we use openssl for true random numbers
#include <openssl/rand.h>

namespace bp {
namespace random {


int
generate()
{
    // Try to seed openssl RAND stuff.  If unable, fall back to rand().
    static bool seeded = false;
    static bool useRand = false;
    if (!seeded) {
        if (RAND_status()) {
            BPLOG_DEBUG("::RAND_status() happy, no need to seed");
        } else {
            BPLOG_DEBUG("::RAND_status() unhappy, seeding");
            int seed = ::rand();
            ::RAND_seed(&seed, sizeof(seed));
            if (::RAND_status() == 0) {
                BPLOG_WARN("::RAND_seed() failed, reverting to ::rand()");
                useRand = true;
            }
        }
        seeded = true;
    }

    unsigned int i = 0;
    if (!useRand) {
        if (::RAND_bytes((unsigned char *) &i, sizeof(i)) == 0) {
            BPLOG_WARN("::RAND_bytes() failed, reverting to ::rand()");
            useRand = true;
        }
    }
    if (useRand) {
        i = (unsigned int) ::rand();
    }
    return(i % ((unsigned)RAND_MAX + 1));
}

   
} // random
} // bp

