/**
 * This file tests mod_sq's file io functions
 */

print(@"<!DOCTYPE html>
<html>
<head>
	<title>Welcome to MOD_SQ</title>
	<style type='text/css'>
		body { font-family: sans-serif; }
		span { font-family: monospace; }
	</style>
</head>
<body>

<h1>Welcome to MOD_SQ</h1>
")

// random filename
local filename = "/tmp/sq"

// test for the existence of the file
if( file_exists(filename) ) {
	print("<p><span>" + filename + "</span> does not exist exists</p>\n\n")
}

// test writing to the file
if( file_put_contents(filename, "testing 123\nwhat about now") ) {
	print("<p>Wrote something to <span>" + filename + "</span></p>\n\n")
}

// test getting the contents of the file
if( file_exists(filename) ) {
	local file = file_get_contents(filename)
	print("<p>The file exists and it contains: <pre>" + file + "</pre></p>\n\n")
}

print(@"
</body>
</html>
")
