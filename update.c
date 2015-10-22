/*-
 * Copyright (c) 2010-2015 Juan Romero Pardines.
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
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fnmatch.h>
#include <libgen.h>

#include "defs.h"

struct ffdata {
	const char *repourl;
	xbps_array_t pkgfiles;
	xbps_dictionary_t resultsd;
	bool verbose;
};

static void
match_files_by_package (xbps_dictionary_t pkg_filesd,
                       xbps_dictionary_keysym_t key,
                       struct ffdata *ffd,
                       const char *pkgver)
{
	xbps_array_t array;
	const char *keyname;

	keyname = xbps_dictionary_keysym_cstring_nocopy (key);

	if (strcmp (keyname, "dirs") == 0)
		return;

	array = xbps_dictionary_get_keysym (pkg_filesd, key);
	for (unsigned int i = 0; i < xbps_array_count (array); i++) {
		xbps_object_t obj;
		const char *filestr = NULL;

		obj = xbps_array_get (array, i);
		xbps_dictionary_get_cstring_nocopy (obj, "file", &filestr);
		if (filestr == NULL)
			continue;

		if (fnmatch ("/usr/share/locale*", filestr, 0) == 0)
			continue;

		if (ffd->verbose)
			printf ("%s: %s\n", pkgver, filestr);

		xbps_array_add_cstring (ffd->pkgfiles, filestr);
	}
}

static bool
pkg_exists (xbps_dictionary_t filesd, const char *pkgname,
            const char *version, bool verbose)
{
	xbps_array_t allkeys;
	xbps_object_t obj;

	allkeys = xbps_dictionary_all_keys (filesd);
	for (unsigned int i = 0; i < xbps_array_count (allkeys); i++) {
		const char *opkgver, *opkgname, *oversion;
		obj = xbps_array_get (allkeys, i);
		opkgver = xbps_dictionary_keysym_cstring_nocopy (obj);

		opkgname = xbps_pkg_name (opkgver);
		oversion = xbps_pkg_version (opkgver);

		/* First, check if pkgnames match, then check if versions match */
		if (strcmp (pkgname, opkgname) == 0) {
			/* TODO: Do a proper version check */
			if (strcmp (version, oversion) == 0) {
				if (verbose)
					fprintf (stdout, "%s-%s: up-to-date\n", pkgname, version);
				return true;
			} else {
				/* TODO: It may not be safe to always assume it's an update */
				/* if versions don't match we'll just assume it's an
				 * update, then remove the old version and return false */
				xbps_dictionary_remove (filesd, opkgver);
			}
		}
	}
	return false;
}

static int
search_packages_cb (struct xbps_handle *xhp,
                 xbps_object_t obj,
                 const char *key,
                 void *arg,
                 bool *done)
{
	xbps_dictionary_t filesd;
	xbps_array_t files_keys;
	struct ffdata *ffd = arg;
	const char *pkgver, *pkgname, *version;
	char *bfile;

	xbps_dictionary_set_cstring_nocopy (obj, "repository", ffd->repourl);
	xbps_dictionary_get_cstring_nocopy (obj, "pkgver", &pkgver);

	pkgname = xbps_pkg_name (pkgver);
	version = xbps_pkg_version (pkgver);

	if (pkg_exists (ffd->resultsd, pkgname, version, ffd->verbose))
		return 0;

	bfile = xbps_repository_pkg_path (xhp, obj);
	assert (bfile);
	filesd = xbps_archive_fetch_plist (bfile, "/files.plist");
	if (filesd == NULL) {
		printf ("%s: couldn't fetch files.plist from %s: %s\n",
				pkgver, bfile, strerror (errno));
		return EINVAL;
	}

	ffd->pkgfiles = xbps_array_create ();

	files_keys = xbps_dictionary_all_keys (filesd);
	for (unsigned int i = 0; i < xbps_array_count (files_keys); i++) {
		match_files_by_package (filesd,
				xbps_array_get (files_keys, i), ffd, pkgver);
	}

	xbps_dictionary_set (ffd->resultsd, pkgver, ffd->pkgfiles);

	xbps_object_release (ffd->pkgfiles);
	xbps_object_release (files_keys);
	xbps_object_release (filesd);
	free (bfile);

	return 0;
}

static int
search_repo_cb (struct xbps_repo *repo, void *arg, bool *done)
{
	xbps_array_t allkeys;
	struct ffdata *ffd = arg;
	int rv;

	if (repo->idx == NULL)
		return 0;

	ffd->repourl = repo->uri;

	allkeys = xbps_dictionary_all_keys (repo->idx);
	rv = xbps_array_foreach_cb (repo->xhp, allkeys, repo->idx, search_packages_cb, ffd);
	xbps_object_release (allkeys);

	return rv;
}

int
update (struct xbps_handle *xhp, struct config *cfg)
{
	struct ffdata ffd;
	int rv;

	ffd.verbose = cfg->verbose;

	ffd.resultsd = xbps_dictionary_internalize_from_zfile (cfg->plist);
	if (ffd.resultsd == NULL)
		ffd.resultsd = xbps_dictionary_create ();

	rv = xbps_rpool_foreach (xhp, search_repo_cb, &ffd);
	if (rv != 0 && rv != ENOTSUP) {
		fprintf (stderr, "error: Failed to initialize rpool: %s\n",
				strerror (rv));
		return rv;
	}

	if (!xbps_dictionary_externalize_to_zfile (ffd.resultsd, cfg->plist)) {
		fprintf (stderr, "error: Unable to write to '%s': %s\n",
				dirname (cfg->plist), strerror (errno));
		return errno;
	}

	xbps_object_release (ffd.resultsd);

	return 0;
}
