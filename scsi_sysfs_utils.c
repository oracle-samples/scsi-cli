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

int load_sysfs_path(char *path, char *buf, int len)
{
	int fd = open(path, O_RDONLY);
	int count;

	print_trace_enter();

	if (fd < 0)
		return -EINVAL;

	count = read(fd, buf, len);
	close(fd);

	if (count < 0 || count >= len)
		return -EINVAL;

	buf[count] = 0;

	if (count && buf[count-1] == '\n')
		buf[count-1] = 0;

	return 0;
}

/*
 * Check if there are any files present before we
 * go parse values
 */
int validate_sysfs_path(char *sysfs_path)
{
	int 		count = 0;
	struct dirent	**dent;

	print_trace_enter();

	print_debug("Validate Path %s\n", sysfs_path);

	count = scandir(sysfs_path, &dent, NULL, alphasort);
	if (count < 0) {
		print_info("No Entries found at %s \n", sysfs_path);
		return -ENODEV;
	}

	print_debug("%s contains %d entries\n", sysfs_path, count);

	return count;
}

char *open_sysfs_stats_file(char *path)
{
	static char line[64];
	char	*p;
	FILE    *fp;

	print_trace_enter();

	if (path == NULL) {
		print_debug("Empty Path Input");
		return NULL;
	}

	print_debug("Path %s (Path Len: %ld)\n", path, strlen(path));

	fp = fopen(path, "r");
	if (fp == NULL) {
		print_debug("Can Not Open Path");
		return NULL;
	}

	p = fgets(line, sizeof(line), fp);
	if (p == NULL) {
		fclose(fp);
		return NULL;
	}
	remove_newline(p);

	print_debug("Output value: %s (len %ld)\n", p, strlen(p));

	fclose(fp);

	return p;
}

int get_device_numbers(char *file_name, struct scsi_device_info *d_info_p)
{
	struct stat	buf;
	int 		err = 0;
	char		dev_path[256];
	
	sprintf(dev_path, "/dev/%s", file_name);
	err = open(dev_path, O_RDONLY);
	if (!err) {
		print_info("\n Invalid disk name %s/%s\n", dev_path, file_name);
		err = -EINVAL;
		goto err_out;
	}

	if (stat(dev_path, &buf)) {
		perror(dev_path);
		err = -EINVAL;
		goto err_out;
	}
	d_info_p->major = major(buf.st_rdev);
	d_info_p->minor = minor(buf.st_rdev);

	print_debug("Major %d, Minor %d",
	    d_info_p->major, d_info_p->minor);

err_out:
	close(err);

	return 0;
}

char *get_device_entry(char *path)
{
	struct dirent	*dent;
	DIR		*dir;

	dir = opendir(path);
	if (!dir)
		return NULL;

	for (dent = readdir(dir); dent; dent = readdir(dir)) {
		if (dent->d_type != DT_DIR)
			continue;

		if (!strncmp(dent->d_name, "sd", 2)) {
			print_debug("%s Entry Name %s \n", path, dent->d_name);
			closedir(dir);
			return dent->d_name;
		}
	}

	return "unknown";
}

int get_hctl_info(struct scsi_device_info *s_info_p)
{
	struct dirent	*dent;
	DIR		*dir;
	int		err = 0;
	char 		block_dev_path[256];

	/*
	 *  /sys/bus/scsi/devices/target%d:%d:%d
	 *
	 * This routine to keep to get the Host/Bus/Target/LUN path.. see if
	 * this can be used for FCP device
	 */
	snprintf(block_dev_path, sizeof(block_dev_path),
		"/sys/dev/block/%d:%d/device/scsi_device",
		s_info_p->major, s_info_p->minor);

	dir = opendir(block_dev_path);
	if (!dir)
		return -ENODEV;

	for (dent = readdir(dir); dent; dent = readdir(dir)) {
		int count;

		if (dent->d_type != DT_DIR)
			continue;

		count = sscanf(dent->d_name, "%d:%d:%d:%d",
				&s_info_p->host, &s_info_p->bus,
				&s_info_p->target, &s_info_p->lun);
		if (count == 4)
			break;
	}
	closedir(dir);

	if (!dent)
		return -ENODEV;

	print_debug("Host:%d, Bus=%d, Target=%d, Lun=%d",
	    s_info_p->host, s_info_p->bus, s_info_p->target, s_info_p->lun);

    	return err;
}

int get_device_count(char *path)
{
	struct dirent	**dent;
	int count;

	print_debug("Open Path %s", path);

	count = scandir(path, &dent, NULL, alphasort);
	if (count < 0) {
		print_debug("Failed to open %s error %d", path, count);
		return -ENODEV;
	}

	print_debug("Path %s Count %d", path, count);

	if ((count - 2) == 0) {
		print_debug("Empty directory, No device found");
		print_debug("d_ent[0]->d_type (%s) %d, d_ent[1]->d_type (%s) %d",
			dent[0]->d_name, dent[0]->d_type,
			dent[1]->d_name, dent[1]->d_type);
		return -ENODEV;
	}

	return (count - 2);
}
