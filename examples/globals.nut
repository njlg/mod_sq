/**
 * This file tests mod_sq's globals
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

if( query == null ) {
	print(@"<p>You do not see anything for <strong>vargv</strong>, <strong>query</strong>, and <strong>_ARGS</strong> because there are were no parameters or queries passed to this page. To see something show up click the following link: <a href='globals.nut?test=this&and=that&mod=squirrel'>globals.nut?test=this&and=that&mod=squirrel</a>
</p>
")
}

/**
 * mod_sq honors Squirrel's regular vargv global
 * BUT instead of it being an array, mod_sq uses
 * it as a table since we have key/value pairs
 */
if( vargv != null ) {
	print("<h1>vargv</h1>\n")
	print("<pre>")
	foreach( key,val in vargv )
		print(key + " -> " + val + "\n" )

	print(@"</pre>

")
}

/**
 * mod_sq sets the entire query string into a global
 * called 'query'. This is essentially a shortcut to
 * _SERVER['args']
 */
print("<h1>query</h1><pre>" + query + "</pre>\n")

/**
 * _ARGS is a table all passed in key/value pairs
 */
print("<h1>_ARGS</h1>\n<pre>\n")
foreach( key,val in _ARGS ) {
	print(key + " -> " + val + "\n" )
}

print(@"</pre>

")

/**
 * The _SERVER global is based on PHP's _SERVER super global
 * and is basically a dump of a lot of information that is
 * accessible via Apache
 */
print("<h1>_SERVER</h1>\n<pre>\n")
foreach( key,val in _SERVER ) {
	print(key + " -> " + val + "\n" )
}

print(@"</pre>

")

/**
 * The _COOKIE global is a table of key/value cookie
 * values that the browser passes to Apache
 */
print("<h1>_COOKIE</h1>\n<pre>\n")
foreach( key,val in _COOKIE ) {
	print(key + " -> " + val + "\n" )
}

print(@"</pre>

</body>
</html>
")

