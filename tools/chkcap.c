/*
 * chkcap.c
 * Copyright (C) 2018 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <unistd.h>


static void
cleanup_cap_free (cap_t* pp)
{
    if (*pp) {
        cap_free (*pp);
        *pp = NULL;
    }
}


int
main (int argc, char *argv[])
{
    if (argc > 3) {
        fprintf (stderr, "Usage: %s [pid|- [capability]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    __attribute__((cleanup(cleanup_cap_free))) cap_t caps = NULL;

    if (argc == 1 || (argv[1][0] == '-' && argv[1][1] == '\0')) {
        caps = cap_get_proc ();
    } else {
        char *endptr = NULL;
        unsigned long pid = strtoull (argv[1], &endptr, 10);
        if (*endptr != '\0' || (pid == ULLONG_MAX && errno == ERANGE)) {
            fprintf (stderr, "%s: Invalid PID '%s'\n", argv[0], argv[1]);
            return EXIT_FAILURE;
        }
        if (!(caps = cap_get_pid ((pid_t) pid))) {
            fprintf (stderr, "%s: Cannot obtain caps for PID %lu: %s\n", argv[0], pid, strerror (errno));
            return EXIT_FAILURE;
        }
    }

    if (argc == 3) {
        // Check for a specific capability.
        cap_value_t cap_id;
        if (cap_from_name (argv[2], &cap_id)) {
            fprintf (stderr, "%s: Invalid capability name '%s'\n", argv[0], argv[2]);
            return EXIT_FAILURE;
        }

        cap_flag_value_t cap_value;
        if (cap_get_flag (caps, cap_id, CAP_PERMITTED, &cap_value)) {
            fprintf (stderr, "%s: Cannot obtain capability flag value: %s\n", argv[0], strerror (errno));
            return EXIT_FAILURE;
        }

        return cap_value ? EXIT_SUCCESS : EXIT_FAILURE;
    } else {
        // Print all process capabilities.
        bool need_final_newline = false;
        char *caps_text = cap_to_text (caps, NULL);
        for (const char *p = caps_text; *p; p++) {
            switch (*p) {
                case '=':  // Ignored.
                case ' ':
                    break;
                case ',':  // Convert to newline.
                    putchar ('\n');
                    break;
                default:   // Pass as-is.
                    need_final_newline = true;
                    putchar (*p);
                    break;
            }
        }
        if (need_final_newline) {
            putchar ('\n');
        }

        cap_free (caps_text);
        return EXIT_SUCCESS;
    }
}
