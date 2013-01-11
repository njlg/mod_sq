/**
 * This file (and two functions) show how you can implement
 * Squirrel sessions (similar to PHP sessions).
 */

/**
 * Creates a session cookie
 *
 * Generates a 32-bit random alphanumeric string and
 * sets a 'session' cookie to that value.
 *
 * @access public
 * @return void
 */
function session_start() {
	// seed the random number generate with time
	srand(time())

	local char = ""
	local alphanumeric = ""
	local switcher = 0

	// generate a stirng of 32 alphanumbers
	for(local i=0; i < 32; i++) {
		switcher = 1 + rand() % 2
		if( switcher  > 1 ) {
			// select a random number
			char = 48 + rand() % 9
		}
		else {
			// select a random lowercase letter
			char = 97 + rand() % 26
		}

		alphanumeric += format("%c", char)
	}

	// set the session cookie
	header("Set-Cookie: session=" + alphanumeric)
}

/**
 * Gets or sets the session id
 *
 * Without any parameters, session_id() with either return the
 * 'session' cookie value, or null if there is no 'session'
 * cookie. If passed a parameter, it will set the 'session'
 * cookie to that value.
 *
 * @param null|string id The new session id to set
 * @return void|null|string
 */
function session_id(id=null) {
	if( id == null && "session" in _COOKIE ) {
		return _COOKIE["session"]
	}
	else if( id == null ) {
		return null
	}
	else {
		header("Set-Cookie: session=" + id)
	}
}

