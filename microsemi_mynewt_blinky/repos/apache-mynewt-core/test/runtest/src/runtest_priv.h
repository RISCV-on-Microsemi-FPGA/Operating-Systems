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
#ifndef __RUNTEST_PRIV_H__
#define __RUNTEST_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#if MYNEWT_VAL(RUNTEST_CLI)
extern struct shell_cmd runtest_cmd_struct;
#endif
#if MYNEWT_VAL(RUNTEST_NEWTMGR)
extern struct mgmt_group runtest_nmgr_group;
#endif

int runtest();

#ifdef __cplusplus
}
#endif

#endif /* __RUNTEST_PRIV_H__ */
