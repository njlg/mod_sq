/**
 * mod_sq - Apache module for running Squirrel files
 *
 * Copyright 2011 Nathan Levin-Greenhaw <nathan@njlg.info>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MOD_SQ_H_
#define _MOD_SQ_H_

#include "httpd.h"
#include "http_log.h"
#include "http_config.h"

#include "apr_optional.h"

#include "squirrel.h"

request_rec* getRequestRec(HSQUIRRELVM v);

// squirrel functions
SQInteger file_exists(HSQUIRRELVM v);
SQInteger file_get_contents(HSQUIRRELVM v);
SQInteger file_put_contents(HSQUIRRELVM v);
SQInteger header(HSQUIRRELVM v);
SQInteger nreverse(HSQUIRRELVM v);
SQInteger print_args(HSQUIRRELVM v);
SQInteger sq_unlink(HSQUIRRELVM v);

#endif
