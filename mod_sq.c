/**
 * mod_sq - Apache module for running Squirrel files
 *
 * Copyright 2011-2013 Nathan Levin-Greenhaw <nathan@njlg.info>
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

#include "mod_sq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mod_sq.h"

#define MOD_SQ_VERSION "0.1.0"
module AP_MODULE_DECLARE_DATA squirrel_module;


/**
 * Quickly prints out the SQ stack
 */
SQInteger print_args(HSQUIRRELVM v) {
	char* type;
	char* buffer;
	SQInteger n;
	SQInteger nargs = sq_gettop(v); // number of arguments
	for(n = 1; n <= nargs; n++) {
		switch(sq_gettype(v, n)) {
			case OT_NULL:
				type = "null";
				break;
			case OT_INTEGER:
				type = "integer";
				break;
			case OT_FLOAT:
				type = "float";
				break;
			case OT_STRING:
				type = "string";
				break;
			case OT_TABLE:
				type = "table";
				break;
			case OT_ARRAY:
				type = "array";
				break;
			case OT_USERDATA:
				type = "userdata";
				break;
			case OT_CLOSURE:
				type = "closure(function)";
				break;
			case OT_NATIVECLOSURE:
				type = "native closure(C function)";
				break;
			case OT_GENERATOR:
				type = "generator";
				break;
			case OT_USERPOINTER:
				type = "userpointer";
				break;
			case OT_CLASS:
				type = "class";
				break;
			case OT_INSTANCE:
				type = "instance";
				break;
			case OT_WEAKREF:
				type = "weak reference";
				break;
			default:
				return sq_throwerror(v,"invalid param"); //throws an exception
		}

		ap_log_error(APLOG_MARK, APLOG_ERR, OK, NULL, "mod_sq: arg %d is a %s", (int)n, type);
	}

	return 1;
}

/**
 * Helper function to lowercase strings
 * Used for adding server variables to the _SERVER global
 */
void to_lower_case(char* string) {
	char c;
	int i, len = strlen(string);
	for( i = 0; i < len; i++ ) {
		c = string[i];
		if( c == '-' ) {
			c = '_';
		}

		string[i] = tolower(c);
	}
}

/**
 * Helper function to quickly grab the request_rec from
 * SQ's registry table
 */
request_rec* get_request_rec(HSQUIRRELVM v) {
	int top;
	SQUserPointer p;

	ap_log_error(APLOG_MARK, APLOG_DEBUG, OK, NULL, "mod_sq: get_request_rec()");

	top = sq_gettop(v);
	sq_pushregistrytable(v);
	sq_pushstring(v, "request_rec", -1);

	if( SQ_FAILED(sq_get(v, -2)) ) {
		ap_log_error(APLOG_MARK, APLOG_DEBUG, OK, NULL, "mod_sq:   could not get request_rec");
		return NULL;
	}

	sq_getuserpointer(v, -1, &p);
	sq_settop(v, top);

	return (request_rec*) p;
}

/**
 * Error Hanlder for Squirrel VM compiler
 */
void compile_error_handler(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column) {
	request_rec* r = get_request_rec(v);

	if( r != NULL ) {
		// should we send error to client?
		ap_set_content_type(r, "text/html");
		ap_rprintf(r, "<pre>COMPILE ERROR: %s\nFile %s, line %d, column %d</pre>", desc, source, (int)line, (int)column);

		// log the issue
		ap_log_rerror(APLOG_MARK, APLOG_ERR, OK, r, "mod_sq: COMPILE ERROR: %s, on line %d, column %d", desc, (int)line, (int)column);
	}
	else {
		ap_log_error(APLOG_MARK, APLOG_ERR, OK, NULL, "mod_sq: COMPILE ERROR: %s, on line %d, column %d", desc, (int)line, (int)column);
	}
}

/**
 * Print function
 */
void printfunc(HSQUIRRELVM v, const SQChar* s, ...) {
	char buffer[PATH_MAX];
	va_list vl;
	va_start(vl, s);
	vsprintf(buffer, s, vl);
	va_end(vl);

	request_rec* r = get_request_rec(v);
	if( r != NULL ) {
		ap_rputs(buffer, r);
	}

	//ap_log_error(APLOG_MARK, APLOG_ERR, OK, NULL, "mod_sq: %s", buffer);
}

/**
 * Error Handler
 */
void errorfunc(HSQUIRRELVM v, const SQChar* s, ...) {
	char buffer[PATH_MAX];
	va_list vl;
	va_start(vl, s);
	vsprintf(buffer, s, vl);
	va_end(vl);

	request_rec* r = get_request_rec(v);
	if( r != NULL ) {
		// should we sent the error to the client?
		if( 1 ) {
			// set a header here
			ap_set_content_type(r, "text/html");
			ap_rputs("<pre>", r);
			ap_rputs(buffer, r);
			ap_rputs("</pre>", r);
		}

		// put errors to log
		ap_log_rerror(APLOG_MARK, APLOG_ERR, OK, r, "mod_sq: %s", buffer);
	}
	else {
		ap_log_error(APLOG_MARK, APLOG_ERR, OK, NULL, "mod_sq: %s", buffer);
	}

	// set error return status?
	//r->status = 500;
}

/**
 * Populate the _ARGS global
 *
 * This differs from the regular SQ vargv in that it is a table
 * where key => val and not just an array
 */
void populate_args(HSQUIRRELVM v, request_rec* r) {
	char* tok;
	char* key;
	char* val;
	char* query;

	if( r->args == NULL ) {
		query = "";
	}
	else {
		query = malloc(strlen(r->args) + 1);
		strcpy(query, r->args);
	}

	sq_pushstring(v, "_ARGS", -1);
	sq_newtable(v);
	tok = strtok(query, "&");

	while( tok != NULL ) {
		// now split the name from the value
		val = strchr(tok, '=');
		if( val != NULL ) {
			tok[(val - tok)] = '\0';
			sq_pushstring(v, tok, -1);
			sq_pushstring(v, (val + 1), -1);
			sq_newslot(v, -3, SQFalse);
			ap_log_rerror(APLOG_MARK, APLOG_ERR, OK, r, "mod_sq: args: %s -> %s", tok, (val + 1));
		}
		else {
			sq_pushstring(v, tok, -1);
			sq_pushstring(v, "", -1);
			sq_newslot(v, -3, SQFalse);
		}

		tok = strtok(NULL, "&");
	}

	sq_newslot(v, -3, SQFalse);
}

/**
 * Populate the _COOKIE global from the request headers
 */
void populate_cookie(HSQUIRRELVM v, request_rec* r) {
	char* tok;
	char* key;
	char* val;
	char* cookies = (char*) apr_table_get(r->headers_in, "cookie");

	if( cookies == NULL ) {
		cookies = "";
	}

	sq_pushstring(v, "_COOKIE", -1);
	sq_newtable(v);
	tok = strtok(cookies, ";");

	while( tok != NULL ) {
		// now split the name from the value
		val = strchr(tok, '=');
		if( val != NULL ) {
			tok[(val - tok)] = '\0';
			sq_pushstring(v, tok, -1);
			sq_pushstring(v, (val + 1), -1);
			sq_newslot(v, -3, SQFalse);
			ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "cookie: %s -> %s", tok, (val + 1));
		}
		else {
			sq_pushstring(v, tok, -1);
			sq_pushstring(v, "", -1);
			sq_newslot(v, -3, SQFalse);
			ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "cookie: %s -> ''", tok);
		}

		tok = strtok(NULL, ";");
	}

	sq_newslot(v, -3, SQFalse);
}

/**
 * Populate the _SERVER global
 */
void populate_server(HSQUIRRELVM v, request_rec* r) {
	sq_pushstring(v, "_SERVER", -1);
	sq_newtable(v);

	// TODO: there has to be a better way to do this,
	// since I am doing the same three lines of code
	// over and over again for different values
	//
	// if I could refer to struct members via a string
	// I could loop through a string pointer array

	sq_pushstring(v, "the_request", -1);
	sq_pushstring(v, r->the_request, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "protocol", -1);
	sq_pushstring(v, r->protocol, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "hostname", -1);
	sq_pushstring(v, r->hostname, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "status_line", -1);
	sq_pushstring(v, r->status_line, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "method", -1);
	sq_pushstring(v, r->method, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "range", -1);
	sq_pushstring(v, r->range, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "content_type", -1);
	sq_pushstring(v, r->content_type, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "handler", -1);
	sq_pushstring(v, r->handler, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "content_encoding", -1);
	sq_pushstring(v, r->content_encoding, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "vlist_validator", -1);
	sq_pushstring(v, r->vlist_validator, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "user", -1);
	sq_pushstring(v, r->user, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "ap_auth_type", -1);
	sq_pushstring(v, r->ap_auth_type, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "unparsed_uri", -1);
	sq_pushstring(v, r->unparsed_uri, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "uri", -1);
	sq_pushstring(v, r->uri, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "filename", -1);
	sq_pushstring(v, r->filename, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "canonical_filename", -1);
	sq_pushstring(v, r->canonical_filename, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "path_info", -1);
	sq_pushstring(v, r->path_info, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "args", -1);
	sq_pushstring(v, r->args, -1);
	sq_newslot(v, -3, SQFalse);

	// now pull out stuff from the r->server rec
	sq_pushstring(v, "server_admin", -1);
	sq_pushstring(v, r->server->server_admin, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "server_hostname", -1);
	sq_pushstring(v, r->server->server_hostname, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "error_file", -1);
	sq_pushstring(v, r->server->error_fname, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "server_path", -1);
	sq_pushstring(v, r->server->path, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "server_scheme", -1);
	sq_pushstring(v, r->server->server_scheme, -1);
	sq_newslot(v, -3, SQFalse);

	// get the inbound headers from the request
	const apr_array_header_t* barr = apr_table_elts(r->headers_in);
	apr_table_entry_t* belt = (apr_table_entry_t *)barr->elts;
	int i, j;
	char key[25];
	for (i = 0; i < barr->nelts; ++i) {
		sprintf(key, "http_%s", belt[i].key);
		to_lower_case(key);

		sq_pushstring(v, key, -1);
		sq_pushstring(v, belt[i].val, -1);
		sq_newslot(v, -3, SQFalse);
	}

	// get connection information from the request
	sq_pushstring(v, "remote_ip", -1);
	sq_pushstring(v, r->connection->remote_ip, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "remote_host", -1);
	sq_pushstring(v, r->connection->remote_host, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "remote_port", -1);
	sq_pushinteger(v, r->connection->remote_addr->port);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "server_ip", -1);
	sq_pushstring(v, r->connection->local_ip, -1);
	sq_newslot(v, -3, SQFalse);

	sq_pushstring(v, "server_port", -1);
	sq_pushinteger(v, r->connection->local_addr->port);
	sq_newslot(v, -3, SQFalse);


	// commit the table to the stack
	sq_newslot(v, -3, SQFalse);
}

/**
 * the _ARGS table would be better than this,
 * but i would like to preserve some functionality
 * that is accessible in the regular squirrel
 */
int populate_vargv(HSQUIRRELVM v, char* query) {
	int retval = 1;
	if( query == NULL ) {
		return retval;
	}

	char* tok = strtok(query, "&");

	while( tok != NULL ) {
		sq_pushstring(v, tok, -1);
		retval++;

		tok = strtok(NULL, "&");
	}

	return retval;
}

// test loading a function
SQInteger nreverse(HSQUIRRELVM v) {
	const SQChar* msg;
	sq_getstring(v, -1, &msg);

	int i;
	int len = strlen(msg);
	char* new = malloc(len);
	len--;
	for( i = 0; i <= len; i++ ) {
		new[len - i] = msg[i];
	}
	new[len + 1] = '\0';

	sq_pushstring(v, new, -1);

	return 1;
}

// test loading a function
SQInteger header(HSQUIRRELVM v) {
	char* key;
	char* val;
	const char* param;
	SQInteger status;
	request_rec* r = get_request_rec(v);

	// grab function param
	if( sq_gettype(v, 2) != OT_STRING ) {
		return SQ_ERROR;
	}
	sq_getstring(v, 2, &param);

	// get values
	key = strtok((char*) param, ":");
	if( key != NULL ) {
		val = strtok(NULL, ":");
	}
	else {
		val = "";
	}

	// deal with second parameter
	if( sq_gettype(v, 3) == OT_INTEGER ) {
		sq_getinteger(v, 3, &status);
		r->status = (int)status;
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_sq: header set status: %d", (int)status);

		if( status >= 300 ) {
			ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "  header(%s, %s)", key, val);
			apr_table_add(r->err_headers_out, key, val);
			return 0;
		}
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "  header(%s, %s)", key, val);
	apr_table_add(r->headers_out, key, val);

	return 0;
}

/**
 * Helper function to load functions into the root table
 */
SQInteger register_global_func(HSQUIRRELVM v, SQFUNCTION f, const char *fname, const char* params) {
	// push the root table so we can add to it
	sq_pushroottable(v);

	// push the function's name
	sq_pushstring(v, fname, -1);

	// create a new function
	sq_newclosure(v, f, 0);

	// enforce the parameters
	sq_setparamscheck(v, 0, params);

	// adds the function
	sq_createslot(v, -3);

	// pops the root table
	sq_pop(v, 1);
}

/**
 * MOD_SQ's request handler
 */
static int sq_handler(request_rec *r) {
	int result;
	apr_file_t *fd = NULL;
	unsigned char buf[r->finfo.size + 1];
	apr_size_t nbytes = sizeof(buf) - 1;

	// debug message
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_sq: sq_handler()");

	// clear our output buffer for this new request

	// AddHandler application/x-httpd-php .php .php5 .phtml
	// AddHandler application/x-httpd-php-source .phps
	if (strcmp(r->handler, "application/x-httpd-sq")) {
		return DECLINED;
	}

	// set a default header here
	ap_set_content_type(r, "text/html");

	if( r->header_only ) {
		return OK;
	}

	if( r->finfo.filetype == 0 ) {
		return DECLINED;
	}

	// debug message
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_sq: args: %s", r->args);

	// start squirrel VM
	HSQUIRRELVM v = sq_open(1024);

	// set some callback functions
	sq_setcompilererrorhandler(v, &compile_error_handler);
	sq_setprintfunc(v, &printfunc, &errorfunc);

	// push the request_rec* onto the registry table
	// (so that we can refer to it later)
	sq_pushregistrytable(v);
	sq_pushstring(v, "request_rec", -1);
	sq_pushuserpointer(v, r);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1);

	// setup globals
	sq_pushroottable(v);
	sq_pushstring(v, "query", -1);
	sq_pushstring(v, r->args, -1);
	sq_newslot(v, -3, SQFalse);

	populate_args(v, r);
	populate_cookie(v, r);
	populate_server(v, r);
	sq_pop(v, 1);

	// open requested file
	if( apr_file_open(&fd, r->filename, APR_READ, APR_OS_DEFAULT, r->pool) != APR_SUCCESS ) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, OK, r, "mod_sq: can't read `%s'", r->filename);
		// let some other handler decide what the problem is
		return DECLINED;
	}

	// read file
	if( (result = apr_file_read(fd, (char *) buf, &nbytes)) != APR_SUCCESS ) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, result, r, "mod_sq: read failed: %s", r->filename);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	// compile the squirrel file (from buffer)
	if( SQ_FAILED(sq_compilebuffer(v, buf, r->finfo.size, r->filename, SQTrue)) ) {
		// return OK here, because the comipler's error handler will output
		// the compile error message
		return OK;
	}

	// debug message
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "loading standard libraries");

	// register all of the standard libraries
	sq_pushroottable(v);
	sqstd_register_bloblib(v);
	sqstd_register_iolib(v);
	sqstd_register_systemlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);

	// sets error handlers
	sqstd_seterrorhandlers(v);

	// load our functions
	register_global_func(v, nreverse, "nreverse", "ts");
	register_global_func(v, header, "header", "tsi|o");
	register_global_func(v, file_put_contents, "file_put_contents", "tss");
	register_global_func(v, file_get_contents, "file_get_contents", "ts");
	register_global_func(v, file_exists, "file_exists", "ts");
	register_global_func(v, sq_unlink, "unlink", "ts");

	// populate the regular vargv global
	int args = populate_vargv(v, r->args);

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "arg count: %d", args);
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "args: %s", r->args);

	// call the compiled squirrel script
	if( SQ_FAILED(sq_call(v, args, SQFalse, SQTrue)) ) {
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "nut failed to run");
		const SQChar* errMsg = "Unknown error.";
		sq_getlasterror(v);

		if( sq_gettype(v, -1) == OT_STRING ) {
			sq_getstring(v, -1, &errMsg);
		}

		ap_rprintf(r, "ERROR: %s\n", errMsg);
		return 0;
	}

	// I am not sure why I put this here, but here it is...
	if (sq_getvmstate(v) == SQ_VMSTATE_SUSPENDED) {
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "VM has been suspended");
	}

	// all done
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, OK, r, "finished");
	return OK;
}

/**
 * Register mod_sq's hooks
 */
static void register_hooks(apr_pool_t *p) {
	ap_hook_handler(sq_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/**
 * Tell Apache how we roll
 */
module AP_MODULE_DECLARE_DATA squirrel_module = {
	STANDARD20_MODULE_STUFF,
	NULL, NULL, NULL, NULL, NULL,
	register_hooks,
};

