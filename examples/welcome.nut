/**
 * Tests Squirrel sessions and _SERVER global
 */
local path = _SERVER["filename"]
local pathend = path.find("welcome.nut")
path = path.slice(0, pathend)

// load session functions
dofile(path + "session.nut")

local id = session_id()
if( id == null ) {
	session_start()
	// get the session id
	id = session_id()
}

local server = ""
foreach( key,val in _SERVER ) {
	server += key + " -> " + val + "\n"
}

local html = @"<!DOCTYPE html>
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

<p>Your session id is <span>" + id + @"</span></p>

<p>These are the <strong>_SERVER</strong> values</p>
<pre>" + server + @"</pre>

</body>
</html>
"

print(html)
