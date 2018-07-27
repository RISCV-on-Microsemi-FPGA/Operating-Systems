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

#include "sysinit/sysinit.h"
#include "syscfg/syscfg.h"
#include "testutil/testutil.h"
#include "test_json.h"

TEST_CASE_DECL(test_json_simple_encode);
TEST_CASE_DECL(test_json_simple_decode);

TEST_SUITE(test_json_suite) {
    test_json_simple_encode();
    test_json_simple_decode();
}

#if MYNEWT_VAL(SELFTEST)
int
main(int argc, char **argv)
{
    sysinit();

    test_json_suite();

    return tu_any_failed;
}
#endif
