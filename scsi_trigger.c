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
#include "scsi_fcp.h"

int fc_hba_reset(int argc, char **argv)
{
	FILE	*fp;
	int	err = 0;
	char 	reset_path[1024] = { 0 };
	char	*command = argv[2];
	char	*fc_host = argv[3];

	print_trace_enter();

	if (argc < 4 || argv[3] == NULL)
	{
		printf(" Usage:\n\n");
		printf(" Missing argument for the sub-command [%s]\n\n", argv[2]);
		printf(" scsi reset <%s> [<arg>] \n", argv[2]);
		return -EINVAL;
	}

	print_debug("argc %d, argv %s\n", argc, argv[3]);
	err = validate_subcommand(argv);
	if (err < 0)
		return err;

	if (strncmp(command, "issue_logo", 10) == 0) {
		print_info(" Issuing LOGO command for FC host %s\n", fc_host);
		sprintf(reset_path, "/sys/class/fc_host/%s/device/issue_logo",
		    fc_host);
	} else if (strncmp(command, "issue_lip", 9) == 0) {
		print_info("Issuing LIP (Loop Initialization Protocol) for FC host %s\n",
		    fc_host);
		snprintf(reset_path, sizeof(reset_path), "/sys/class/fc_host/%s/issue_lip",
		    fc_host);
	} else if (strncmp(command, "adapter", 7) == 0) {
		print_info(" Issuing Reset for %s\n", fc_host);
		snprintf(reset_path, sizeof(reset_path), "/sys/class/fc_host/%s/device/reset",
		    fc_host);
	} else {
		print_info(" Unknown Command %s \n", command);
		return -EINVAL;
	}

	print_info("Issuing %s\n", reset_path);

	fp = fopen(reset_path, "w");
	if (fp == NULL) {
		perror(reset_path);
		return -EINVAL;
	}
	fprintf(fp, "%d", 1);
	fclose(fp);

	print_info("Command %s issued Sucessfully to %s\n", command, fc_host);

	return 0;
}

void pci_bus_rescan()
{
//	struct dirent	*entry;
//	DIR		*dir;
//	FILE		*fp;
	int		fd;
	char 		d = '1';

	printf("%s: Issuing Rescan for %s\n", __func__, RESCAN_PCI_PATH);
	fd = open(RESCAN_PCI_PATH, O_WRONLY);
	write(fd, &d, 1);
	close(fd);

#if 0
	/*  NVMe Device has Controller Reset and Rescan hooks */
	/sys/block/nvme0n1/device/rescan_controller
	/sys/block/nvme0n1/device/reset_controller
#endif
}


/**
 * To Remove device, each individual device needs to be removed using
 * /sys/bus/pci/devices/0000\:b3\:00.0/remove
 *
 * Same directory has the rescan hook for just scanning that bus
 *
 * /sys/bus/pci/devices/0000\:b3\:00.0/rescan
 *
 * reset hook is also available for Resetting the PCI bus
 *
 * /sys/bus/pci/devices/0000\:b3\:00.0/reset
 *
 * /sys/bus/pci/devices/0000\:b3\:00.0/enable
 */
