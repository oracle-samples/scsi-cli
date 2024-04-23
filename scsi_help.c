// SPDX-License-Identifier: UPL-1.0
/*
 * Copyright (c) 2024, Oracle and/or its affiliates.
 *
 * The Universal Permissive License (UPL), Version 1.0
 *
 * Subject to the condition set forth below, permission is hereby granted to any
 * person obtaining a copy of this software, associated documentation and/or data
 * (collectively the "Software"), free of charge and under any and all copyright
 * rights in the Software, and any and all patent rights owned or freely
 * licensable by each licensor hereunder covering either (i) the unmodified
 * Software as contributed to or provided by such licensor, or (ii) the Larger
 * Works (as defined below), to deal in both
 *
 * (a) the Software, and
 * (b) any piece of software and/or hardware listed in the
 * lrgrwrks.txt file if one is included with the Software (each a "Larger
 * Work" to which the Software is contributed by such licensors),
 *
 * without restriction, including without limitation the rights to copy, create
 * derivative works of, display, perform, and distribute the Software and make,
 * use, sell, offer for sale, import, export, have made, and have sold the
 * Software and the Larger Work(s), and to sublicense the foregoing rights on
 * either these or other terms.
 *
 * This license is subject to the following condition:
 * The above copyright notice and either this complete permission notice or at
 * a minimum a reference to the UPL must be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "scsi.h"

static unsigned padding = 15;

void general_help(void)
{
	print_trace_enter();

	printf("\n");
	printf("%-.4s%s - %s\n", space, SCSI_TOOL_NAME, SCSI_TOOL_VERSION);
	printf("\n");

	usage();

	printf("\n");
	printf("%-.4sFollowing commands are implemented:\n", space);
	printf("%-.4s%-.64s\n", space, dash);

	list_builtins();

	printf("\n");
	printf("%-.4sSee '%s help <command>' for each command\n", space,
	    SCSI_TOOL_NAME);
	printf("\n");
}

void usage(void)
{
	print_trace_enter();

	printf("%-.4sUsage:\n", space);
	printf("%-.4s%-.6s\n", space, dash);
	printf("%-.4s%s <command> <sub-command> [<device>]\n", space, SCSI_TOOL_NAME);
	printf("\n");
	printf("%-.4sWhere:\n", space);
	printf("%-.4s%-.6s\n", space, dash);
	printf("%-.4sThe '<sub-command>' is command specific options supported by the tool\n", space);
	printf("%-.4sThe '<device>' may be either\n", space);
	printf("\n");
	printf("%-.6s- A Fibre Channel Device\n", space);
	printf("\n");
	printf("%-.6s- An iSCSI Device\n", space);
	printf("\n");
	printf("%-.6s- One of the\n", space);
	printf("%-.8s- SCSI generic device (ex: /dev/sdX)\n", space);
	printf("%-.8s- SCSI disk device (ex: /dev/sgX)\n", space);
	printf("%-.8s- NVMe Block device (ex: /dev/nvmeXnY\n", space);
	printf("%-.8s- NVMe generic Block device (ex: /dev/ngX)\n", space);
	printf("%-.8s- Device Mapper stacking device (ex: /dev/dm-X)\n", space);
	printf("%-.8s- RAID Array device (ex: /dev/mdXYZ)\n", space);

}

void print_list_help(void)
{
	print_trace_enter();

	printf(" %-*s %s", padding, "list", "List Block Devices, FC Adapters");
	printf("\n");
}

void print_stats_help(void)
{
	print_trace_enter();

	printf("%-*s %s", padding, "stats", "Show statistics for Block Devices, FC Adapters");
	printf("\n");
}

int help_cmd(int argc, char **argv)
{
	print_trace_enter();

	if (argc > 2) {
		if (!argv[2]) {
			printf("Enter one of the supported sub-command\n");
			return 0;
		}

		print_trace_enter();

		if (strncmp(argv[2], "list", 4) == 0)
			print_list_help();

		if (strncmp(argv[2], "stats", 5) == 0)
			print_stats_help();
	} else {
		general_help();
	}

	return 0;
}

void version_cmd(void)
{
	print_trace_enter();

	printf(" %s - %s\n", SCSI_TOOL_NAME, SCSI_TOOL_VERSION);
	printf("\n");
	printf(" %-*s %s\n", padding, "version", "Shows the program version");
}
