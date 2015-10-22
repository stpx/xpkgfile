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

#include <stdlib.h>
#include <fnmatch.h>

#include "defs.h"

void
match_files_by_pattern (xbps_dictionary_t filesd,
                        const char *pkgver,
                        void *arg)
{
	xbps_array_t pkgfiles;
	struct config *cfg = arg;

	pkgfiles = xbps_dictionary_get (filesd, pkgver);
	for (unsigned int i = 0; i < xbps_array_count (pkgfiles); i++) {
		char *filestr = NULL;
		xbps_array_get_cstring (pkgfiles, i, &filestr);

		if (filestr == NULL)
			continue;

		if (fnmatch (cfg->pattern, filestr, FNM_PERIOD) == 0)
			printf ("\033[0;1m%s:\033[0m %s\n", pkgver, filestr);

		free (filestr);
	}
}
