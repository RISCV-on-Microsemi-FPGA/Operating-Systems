/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "fcb_test.h"

TEST_CASE(fcb_test_empty_walk)
{
    int rc;
    struct fcb *fcb;

#if 0
    fcb_test_wipe();
    fcb = &test_fcb;
    memset(fcb, 0, sizeof(*fcb));

    fcb->f_sector_cnt = 2;
    fcb->f_sectors = test_fcb_area;

    rc = fcb_init(fcb);
    TEST_ASSERT(rc == 0);
#endif
    fcb = &test_fcb;

    rc = fcb_walk(fcb, 0, fcb_test_empty_walk_cb, NULL);
    TEST_ASSERT(rc == 0);
}
