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
#include "log_test.h"

TEST_CASE(log_flush_fcb)
{
    struct log_offset log_offset = { 0 };
    int rc;

    rc = log_flush(&my_log);
    TEST_ASSERT(rc == 0);

    rc = log_walk(&my_log, log_test_walk2, &log_offset);
    TEST_ASSERT(rc == 0);
}
