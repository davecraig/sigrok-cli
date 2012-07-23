/*
 * This file is part of the sigrok project.
 *
 * Copyright (C) 2011 Bert Vermeulen <bert@biot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <glib.h>
#include <libsigrok/libsigrok.h>
#include "sigrok-cli.h"

char **parse_probestring(int max_probes, const char *probestring)
{
	int tmp, b, e, i;
	char **tokens, **range, **probelist, *name, str[8];
	gboolean error;

	error = FALSE;
	range = NULL;
	if (!(probelist = g_try_malloc0(max_probes * sizeof(char *)))) {
		/* TODO: Handle errors. */
	}
	tokens = g_strsplit(probestring, ",", max_probes);

	for (i = 0; tokens[i]; i++) {
		if (strchr(tokens[i], '-')) {
			/* A range of probes in the form 1-5. */
			range = g_strsplit(tokens[i], "-", 2);
			if (!range[0] || !range[1] || range[2]) {
				/* Need exactly two arguments. */
				g_critical("Invalid probe syntax '%s'.", tokens[i]);
				error = TRUE;
				break;
			}

			b = strtol(range[0], NULL, 10);
			e = strtol(range[1], NULL, 10);
			if (b < 0 || e >= max_probes || b >= e) {
				g_critical("Invalid probe range '%s'.", tokens[i]);
				error = TRUE;
				break;
			}

			while (b <= e) {
				snprintf(str, 7, "%d", b);
				probelist[b] = g_strdup(str);
				b++;
			}
		} else {
			tmp = strtol(tokens[i], NULL, 10);
			if (tmp < 0 || tmp >= max_probes) {
				g_critical("Invalid probe %d.", tmp);
				error = TRUE;
				break;
			}

			if ((name = strchr(tokens[i], '='))) {
				probelist[tmp] = g_strdup(++name);
				if (strlen(probelist[tmp]) > SR_MAX_PROBENAME_LEN)
					probelist[tmp][SR_MAX_PROBENAME_LEN] = 0;
			} else {
				snprintf(str, 7, "%d", tmp);
				probelist[tmp] = g_strdup(str);
			}
		}
	}

	if (error) {
		for (i = 0; i < max_probes; i++)
			if (probelist[i])
				g_free(probelist[i]);
		g_free(probelist);
		probelist = NULL;
	}

	g_strfreev(tokens);
	if (range)
		g_strfreev(range);

	return probelist;
}

GHashTable *parse_generic_arg(const char *arg, gboolean sep_first)
{
	GHashTable *hash;
	int i;
	char **elements, *e;

	if (!arg || !arg[0])
		return NULL;

	i = 0;
	hash = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, g_free);
	elements = g_strsplit(arg, ":", 0);
	if (sep_first)
		g_hash_table_insert(hash, g_strdup("sigrok_key"),
				g_strdup(elements[i++]));
	for (; elements[i]; i++) {
		e = strchr(elements[i], '=');
		if (!e)
			g_hash_table_insert(hash, g_strdup(elements[i]), NULL);
		else {
			*e++ = '\0';
			g_hash_table_insert(hash, g_strdup(elements[i]), g_strdup(e));
		}
	}
	g_strfreev(elements);

	return hash;
}

char *strcanon(const char *str)
{
	int p0, p1;
	char *s;

	/* Returns newly allocated string. */
	s = g_ascii_strdown(str, -1);
	for (p0 = p1 = 0; str[p0]; p0++) {
		if ((s[p0] >= 'a' && s[p0] <= 'z')
				|| (s[p0] >= '0' && s[p0] <= '9'))
			s[p1++] = s[p0];
	}
	s[p1] = '\0';

	return s;
}

int canon_cmp(const char *str1, const char *str2)
{
	int ret;
	char *s1, *s2;

	s1 = strcanon(str1);
	s2 = strcanon(str2);
	ret = g_ascii_strcasecmp(s1, s2);
	g_free(s2);
	g_free(s1);

	return ret;
}
