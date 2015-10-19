/*-
 * Copyright (c) 2015 Steve Prybylski.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include "defs.h"

static void
usage (bool fail)
{
	fprintf (stdout,
			"Usage: xpkgfile [OPTIONS] MODE [TARGET]\n"
			"\nOPTIONS\n"
			" -h --help                Display this help and exit\n"
			" -V --version             Display version and exit\n"
			" -v --verbose             Verbose messages\n"
			"\nMODE\n"
			" -l --list                List files of a packages\n"
			" -s --search              Search for packages containing target (default)\n"
			" -u --update              Update repo file lists\n"
			);

	exit (fail ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void
version (void)
{
	fprintf (stdout, "%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

int
main (int argc, char *argv[])
{
	const char *shortopts = "hlsuVv";
	const struct option longopts[] = {
		{"help", no_argument, NULL, 'h'},
		{"list", no_argument, NULL, 'l'},
		{"search", no_argument, NULL, 's'},
		{"update", no_argument, NULL, 'u'},
		{"verbose", no_argument, NULL, 'v'},
		{"version", no_argument, NULL, 'V'},
		{NULL, 0, NULL, 0}
	};

	struct xbps_handle xh;
	struct config cfg = {0};
	int c, rv = 0;
	bool list_mode, search_mode, update_mode, opmode;
	bool verbose, debug;

	list_mode = search_mode = update_mode = false;
	verbose = debug = false;

	memset (&xh, 0, sizeof (xh));

	while ((c = getopt_long (argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
			case 'd':
				/* cfg.debug = true; */
				debug = true;
				break;
			case 'h':
				usage (false);
				/* NOT REACHED */
			case 'l':
				list_mode = opmode = true;
				break;
			case 's':
				search_mode = opmode = true;
				break;
			case 'u':
				update_mode = opmode = true;
				break;
			case 'V':
				version ();
				exit (EXIT_SUCCESS);
			case 'v':
				/* cfg.verbose = true; */
				verbose = true;
				break;
			case '?':
			default:
				usage (true);
				/* NOT REACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (!argc && !opmode) {
		usage (true);
	} else if (!opmode) {
		/* search mode by default */
		search_mode = opmode = true;
	}

	cfg.pattern = *argv;
	cfg.verbose = verbose;
	snprintf (cfg.plist, sizeof (cfg.plist), "%s/files.plist", XPKGFILE_CACHE_PATH);

	if ((rv = xbps_init (&xh)) != 0) {
		fprintf (stderr, "error: Failed to initialize libxbps: %s\n",
				strerror (rv));
		exit (EXIT_FAILURE);
	}

	if ((list_mode || search_mode) && cfg.pattern == NULL) {
		fprintf (stderr, "error: You must specify a target.");
		exit (EXIT_FAILURE);
	}

	if (list_mode) {
		rv = search (&xh, list_files, &cfg);
	} else if (search_mode) {
		rv = search (&xh, match_files_by_pattern, &cfg);
	} else if (update_mode) {
		rv = update (&xh, &cfg);
	}

	xbps_end (&xh);
	exit (rv);
}
