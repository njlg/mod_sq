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

#include "mod_sq.h"

// read
SQInteger file_get_contents(HSQUIRRELVM v) {
	// function parameters
	const SQChar* filename;
	char* contents;

	// internal structures
	apr_file_t* file;
	apr_finfo_t finfo;
	apr_size_t nbytes;
	apr_status_t status;
	request_rec* r = getRequestRec(v);

	// for error messages
	char error[120];
	char errorMessage[120];

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "file_get_contents()");

	// grab filename param
	if( sq_gettype(v, 2) != OT_STRING ) {
		return SQ_ERROR;
	}
	else if( SQ_FAILED(sq_getstring(v, 2, &filename)) ) {
		return SQ_ERROR;
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_get_contents('%s')", filename);
	if( (status = apr_file_open(&file, filename, APR_READ, APR_OS_DEFAULT, r->pool)) != APR_SUCCESS ) {
		apr_strerror(status, error, sizeof error);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_get_contents() failed: %s", error);
		sprintf(errorMessage, "file_get_contents() failed: %s", error);
		errorfunc(v, errorMessage);
		sq_pushbool(v, SQFalse);
		return 1;
	}

	// figure out file size and allocate enough room to read it all in
	status = apr_file_info_get(&finfo, APR_FINFO_NORM, file);
	nbytes = finfo.size;
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "file_get_contents() file is %d", (int)nbytes);
	contents = apr_palloc(r->pool, finfo.size);

	if( (status = apr_file_read(file, contents, &nbytes)) != APR_SUCCESS ) {
		apr_strerror(status, error, sizeof error);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_get_contents() failed: %s", error);
		sprintf(errorMessage, "file_get_contents() read %d bytes, but failed: %s", (int)nbytes, error);
		errorfunc(v, errorMessage);
		sq_pushbool(v, SQFalse);
	}
	else {
		contents[nbytes] = '\0';
		sq_pushstring(v, contents, -1);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_get_contents() read %d bytes", (int)nbytes);
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "file_get_contents() returning 1");
	return 1;
}

// write
SQInteger file_put_contents(HSQUIRRELVM v) {
	// function parameters
	const SQChar* filename;
	const SQChar* contents;

	// internal structures
	apr_file_t* file;
	apr_size_t nbytes;
	apr_status_t status;
	request_rec* r = getRequestRec(v);

	// for error messages
	char error[120];
	char errorMessage[120];

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "file_put_contents()");

	// grab filename param
	if( sq_gettype(v, 2) != OT_STRING ) {
		return SQ_ERROR;
	}
	else if( SQ_FAILED(sq_getstring(v, 2, &filename)) ) {
		return SQ_ERROR;
	}

	// grab contents param
	if( sq_gettype(v, 3) != OT_STRING ) {
		return SQ_ERROR;
	}
	else if( SQ_FAILED(sq_getstring(v, 3, &contents)) ) {
		return SQ_ERROR;
	}

	nbytes = strlen(contents);

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_put_contents('%s', '%s') %d", filename, contents, (int)nbytes);
	if( (status = apr_file_open(&file, filename, APR_WRITE|APR_CREATE, APR_OS_DEFAULT, r->pool)) != APR_SUCCESS ) {
		apr_strerror(status, error, sizeof error);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_put_contents() failed: %s", error);
		sprintf(errorMessage, "file_put_contents() failed: %s", error);
		errorfunc(v, errorMessage);
		sq_pushbool(v, SQFalse);
	}
	else if( (status = apr_file_write(file, contents, &nbytes)) != APR_SUCCESS ) {
		apr_strerror(status, error, sizeof error);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_put_contents() failed: %s", error);
		sprintf(errorMessage, "file_put_contents() failed: %s", error);
		errorfunc(v, errorMessage);
		sq_pushbool(v, SQFalse);
	}
	else {
		sq_pushinteger(v, nbytes);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_put_contents() wrote %d bytes", (int)nbytes);
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "file_put_contents() returning 1");
	return 1;
}

// unlink
SQInteger sq_unlink(HSQUIRRELVM v) {
	const SQChar* filename;
	apr_status_t status;
	request_rec* r = getRequestRec(v);

	char errorMessage[120];
	char error[120];

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "unlink()");

	// grab function param
	if( sq_gettype(v, 2) != OT_STRING ) {
		return SQ_ERROR;
	}

	if( SQ_FAILED(sq_getstring(v, 2, &filename)) ) {
		return SQ_ERROR;
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    unlink(%s)", filename);
	status = apr_file_remove(filename, r->pool);
	if( status ) {
		apr_strerror(status, error, sizeof error);
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    unlink failed: %s", error);
		sprintf(errorMessage, "unlink failed: %s", error);
		errorfunc(v, errorMessage);
		sq_pushbool(v, SQFalse);
	}
	else {
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    unlink() worked");
		sq_pushbool(v, SQTrue);
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "unlink() returning 1");
	return 1;
}

// exists
SQInteger file_exists(HSQUIRRELVM v) {
	const SQChar* filename;
	apr_finfo_t finfo;
	apr_status_t status;
	request_rec* r = getRequestRec(v);
	
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "file_exists()");

	// grab function param
	if( sq_gettype(v, 2) != OT_STRING ) {
		return SQ_ERROR;
	}

	if( SQ_FAILED(sq_getstring(v, 2, &filename)) ) {
		return SQ_ERROR;
	}
	
	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    file_exists(%s)", filename);

	status = apr_stat(&finfo, filename, APR_FINFO_CSIZE, r->pool);
	if( status ) {
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    set false");
		sq_pushbool(v, SQFalse);
	}
	else {
		ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    set true");
		sq_pushbool(v, SQTrue);
	}

	ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "    returning 1");
	return 1;
}

