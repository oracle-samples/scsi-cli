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
#include "scsi_print.h"

struct device_type_name {
	int scsi_dev_type;
	const char *dev_name;
} dev_type_str[] = {
	{ DIRECT_ACCESS_BLOCK_DEVICE, 	"disk" },
	{ SEQ_ACCESS_DEVICE, 		"Seqential" },
	{ PRINTER_DEVICE,		"Printer" },
	{ PROCESSOR_DEVICE,		"Processor" },
	{ WRITE_ONCE_DEVICE,		"Write-Once" },
	{ CD_DVD_ROM_DEVICE,		"CD/DVD-ROM" },
	{ SCANNER_DEVICE,		"Scanner" },
	{ OPTICAL_MEMORY_DEVICE,	"Optical" },
	{ MEDIUM_CHANGER_DEVICE,	"Medium Changer" },
	{ COMMUNICATION_DEVICE,		"Communication" },
	{ STORAGE_ARRAY_CNTROLLER,	"Storage Array" },	/* 0x0C */
	{ ENCLOSURE_DEVICE,		"Enclosure" },		/* 0x0D */
	{ SIM_DIRECT_ACCESS_DEVICE,	"Sim Direct Access" },	/* 0x0E */
	{ OPTICAL_CARD_RW_DEVICE,	"Optical Card RW" },	/* 0x0F */
	{ OBJECT_BASED_STORAGE_DEVICE,	"Object Based Storage"},/* 0x11 */
	{ AUTOMATION_INTERFACE,		"Automation" },		/* 0x12 */
	{ SECURITY_MANAGER_DEVICE,	"security Managed" },	/* 0x13 */
	{ HOST_MANAGED_ZBD,		"Host-Managed ZBD" },	/* 0x14 */
	{ WELL_KNOWN_LU,		"Well Known LU" },	/* 0x1E */
	{ UNKNOWN_DEVICE,		"Unknown" },		/* 0x1F */
};

static const char *dev_type_to_dev_name(int dev_type)
{
	size_t i;
	struct  device_type_name *d;

	for (i = 0; i < ARRAY_SIZE(dev_type_str); i++) {
		d = dev_type_str + i;
		if (dev_type == d->scsi_dev_type)
			return d->dev_name;
	}
	return "Unknow Device";
}

int get_disk_stats(struct scsi_device_info *s_info_p, char *disk_name)
{
	char	line[256];
	char	path[256];
	char	*p;
	FILE	*f;
	int	ret = 0;

	print_trace_enter();

	snprintf(path, sizeof(path), "/sys/block/%s/stat", disk_name);

	print_debug("Path: %s, disk: %s \n", path, disk_name);

	f = fopen(path, "r");
	if (f == NULL) {
		printf("file open %s returned NULL\n", path);
		return -EIO;
	}

	p = fgets(line, sizeof(line), f);
	if (!p) {
		fclose(f);
		return -EIO;
	}

	print_debug("Open Path %s,\n Stats \n%s\n", path, p);

	ret = sscanf(p, "%"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" "
			"%"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" "
			" %"SCNu64" %"SCNu64"\n",
			&s_info_p->dstat.ios[0], &s_info_p->dstat.merges[0],
			&s_info_p->dstat.sectors[0], &s_info_p->dstat.ticks[0],
			&s_info_p->dstat.ios[1], &s_info_p->dstat.merges[1],
			&s_info_p->dstat.sectors[1], &s_info_p->dstat.ticks[1],
			&s_info_p->dstat.io_ticks, &s_info_p->dstat.time_in_queue);
        fclose(f);

        return ret != 10;
}

int get_disk_vendor_model(struct scsi_device_info *s_info)
{
	char	temp_str[32] = { 0 };
	char	path[1024];
	int 	err = 0;

	print_trace_enter();

	print_debug("Disk Path %s, Disk Name %s", s_info->disk_path,
		s_info->disk_name);

	/* /sys/block/sda/device/vendor */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "vendor");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	s_info->vendor = strdup(temp_str);
	print_debug("%s: %s \n", path, s_info->vendor);

	/* /sys/block/sda/device/model */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "model");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	s_info->model = strdup(temp_str);

	/* /sys/block/sda/device/rev */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "rev");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	s_info->rev = strdup(temp_str);
	print_debug("%s: Path %s, Value %s (%s) \n", __func__, path,
		s_info->rev, temp_str);

	return err;
}

int get_device_mapper_model(struct scsi_device_info *s_info)
{
	char	temp_str[32] = { 0 };
	char	path[1024];
	int 	err = 0;

	print_trace_enter();

	print_debug("Disk Path %s, Disk Name %s", s_info->disk_path,
		s_info->disk_name);

	/* /sys/block/dm-11/dm/name */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/dm/%s", s_info->disk_path, "name");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	s_info->model = strdup(temp_str);

	return err;
}

int get_nvme_device_model(struct scsi_device_info *s_info)
{
	char	temp_str[32] = { 0 };
	char	path[1024];
	int 	err = 0;

	print_trace_enter();

	print_debug("Disk Path %s, Disk Name %s", s_info->disk_path,
		s_info->disk_name);

	/*  /sys/block/nvme0n1/device/model */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "model");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	s_info->model = strdup(temp_str);

	/*  /sys/block/nvme0n1/device/firmware_rev */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "firmware_rev");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	s_info->rev = strdup(temp_str);

	return err;
}

int validate_enclosure_type(struct scsi_device_info *d_info)
{
	size_t 	i;
	struct  device_type_name *d;
	char	temp_path[32] = { 0 };
	char	temp_val[8] = { 0 };
	char    *end;

	print_trace_enter();

	/* Extract device Type from a give disk path */
	snprintf(temp_path, sizeof(temp_path), "%s/device/type", d_info->disk_path);
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(temp_path));
	d_info->device_type = strtol(temp_val, &end, 0);
	print_debug("Path: %s, Value %s", temp_path, temp_val);

	/* Translate device type from number to a human readable string */
	for (i = 0; i < ARRAY_SIZE(dev_type_str); i++) {
		d = dev_type_str + i;

		print_debug("Search for Device Type: %d \n",
			d_info->device_type);
		if (d_info->device_type == d->scsi_dev_type) {
			snprintf(d_info->disk_type, sizeof(d_info->disk_type),
				dev_type_to_dev_name(d_info->device_type));
			print_debug("Found %s \n", d_info->disk_type);
			return 0;
		}
	}

	return -ENODEV;
}

int get_enclosure_details(struct scsi_device_info *s_info)
{
	int 	count = 0;
	char	path[1024];
	char	temp_val[8] = { 0 };
	char	*end;

	print_debug("disk_path %s, disk_name %s\n", s_info->disk_path,
		s_info->disk_name);

	/* Get Hard-disk connected to this Controller */
	snprintf(path, sizeof(path), "%s/%s", s_info->disk_path, "components");
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(path));
	count = strtoull(temp_val, &end, 0);
	print_debug("Path: %s, Value %s", path, temp_val);

	if (count < 0)
		return -EIO;

	/* /sys/class/enclosure/0\:0\:8\:0/device/model */
	memset(temp_val, 0, sizeof(temp_val));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "model");
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(path));
	s_info->model = strdup(temp_val);

	/* /sys/block/sda/device/vendor */
	memset(temp_val, 0, sizeof(temp_val));
	snprintf(path, sizeof(path), "%s/device/%s", s_info->disk_path, "vendor");
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(path));
	s_info->vendor = strdup(temp_val);

	return 0;
}

/*
 * list all the enclosure device
 */
int list_enclosure(struct scsi_device_info *s_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		scsi_sysfs_path[64];
	char		enclosure_path[128];
	int		err = 0;

	print_trace_enter();

	snprintf(scsi_sysfs_path, sizeof(scsi_sysfs_path), "/sys/class/enclosure");

	dir = opendir(scsi_sysfs_path);
	if (unlikely(!dir)) {
		print_debug("\n No Enclosure device configured \n");
		return -ENODEV;
	}

	print_command_label("Enclosure");

	print_enclosure_header();

	for (entry = readdir(dir); entry; entry = readdir(dir)) {
		if (entry->d_type != DT_DIR) {
			s_info = alloc_scsi_dev();
			if (!s_info)
				return -ENOSPC;

			err = sscanf(entry->d_name, "%d:%d:%d:%d", &s_info->host,
				&s_info->bus, &s_info->target, &s_info->lun);

			print_debug("%s/%s - %d: %d: %d: %d\n", scsi_sysfs_path,
				entry->d_name, s_info->host,
				s_info->bus, s_info->target, s_info->lun);

			s_info->sysfs_root = strdup(scsi_sysfs_path);
			s_info->disk_name = strdup(entry->d_name);

			snprintf(enclosure_path, sizeof(enclosure_path), "%s/%s",
			    s_info->sysfs_root, s_info->disk_name);
			s_info->disk_path = strdup(enclosure_path);

			err = validate_enclosure_type(s_info);
			if (err < 0) {
				put_scsi_dev(s_info);
				return err;
			}

			get_enclosure_details(s_info);

			print_enclosure_info(s_info);

			put_scsi_dev(s_info);

			if (err == 4 || err == 0)
				break;
		}
	}
	closedir(dir);

	return err;
}

int list_block_devs(struct scsi_device_info *d_info)
{
	struct dirent		*entry;
	DIR			*dir;
	char			disk_path[512];

	print_trace_enter();

	dir = opendir(SYSFS_BLOCK_PATH);
	if (unlikely(!dir)) {
		print_info("\n No Block device configured \n");
		return -ENODEV;
	}

	print_command_label("block");

	print_disk_header();

	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "%s/%s",
		    SYSFS_BLOCK_PATH, entry->d_name);

		if (!strncmp(entry->d_name, "sd", 2)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENODEV;

			d_info->disk_path = strdup(disk_path);
			d_info->disk_name = strdup(entry->d_name);
			get_disk_type(d_info);
			snprintf(d_info->disk_type, sizeof(d_info->disk_type),
			    dev_type_to_dev_name(d_info->device_type));
			get_device_numbers(entry->d_name, d_info);
			get_hctl_info(d_info);
			get_disk_vendor_model(d_info);
			print_disk_info(d_info);

			put_scsi_dev(d_info);
			continue;
		}
	}
	closedir(dir);

	dir = opendir(SYSFS_BLOCK_PATH);
	if (unlikely(!dir)) {
		print_info("\n No Block device configured \n");
		return -ENODEV;
	}

	print_command_label("nvme-block");
	print_nvme_disk_header();
	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "%s/%s",
		    SYSFS_BLOCK_PATH, entry->d_name);
		if (!strncmp(entry->d_name, "nvme", 4) &&
		    strncmp(entry->d_name, "nvme-fabrics", 12)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENOSPC;

			d_info->device_type = DIRECT_ACCESS_BLOCK_DEVICE;
			snprintf(d_info->disk_type, sizeof(d_info->disk_type),
			    dev_type_to_dev_name(d_info->device_type));
			d_info->disk_path = strdup(disk_path);
			get_device_numbers(entry->d_name, d_info);
			get_hctl_info(d_info);
			d_info->disk_name = strdup(entry->d_name);
			get_nvme_device_model(d_info);
			print_nvme_disk_info(d_info);

			put_scsi_dev(d_info);

			continue;
		}
	}
	closedir(dir);

	return 0;
}

int list_controllers(struct scsi_device_info *d_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		disk_path[512];

	print_trace_enter();

	dir = opendir(SYSFS_SCSI_GEN_PATH);
	if (unlikely(!dir))
		return -ENODEV;

	print_command_label("Disk Controller");

	print_disk_header();

	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "%s/%s",
		    SYSFS_SCSI_GEN_PATH, entry->d_name);
		print_trace_enter();

		d_info = alloc_scsi_dev();
		if (!d_info)
			return -ENODEV;

		d_info->disk_path = strdup(disk_path);
		d_info->disk_name = strdup(entry->d_name);
		get_disk_type(d_info);
		snprintf(d_info->disk_type, sizeof(d_info->disk_type),
		    dev_type_to_dev_name(d_info->device_type));
		get_device_numbers(entry->d_name, d_info);
		get_hctl_info(d_info);
		get_disk_vendor_model(d_info);

		if (d_info->device_type == STORAGE_ARRAY_CNTROLLER)
			print_disk_info(d_info);

		put_scsi_dev(d_info);

		continue;
	}
	closedir(dir);

	return 0;
}

int list_cdrom_devs(struct scsi_device_info *d_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		disk_path[512];

	print_trace_enter();

	dir = opendir("/dev");
	if (unlikely(!dir))
		return -ENODEV;

	print_command_label("CD-ROM Devices");

	print_disk_header();

	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "/dev/%s", entry->d_name);
		if ((!strncmp(entry->d_name, "sr", 2) &&
		    (strncmp(entry->d_name, "std", 3))) ||
		    !strncmp(entry->d_name, "scd", 3)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENOSPC;

			d_info->disk_path = strdup(disk_path);
			d_info->disk_name = strdup(entry->d_name);
			get_disk_type(d_info);
			snprintf(d_info->disk_type, sizeof(d_info->disk_type),
			    dev_type_to_dev_name(d_info->device_type));
			put_scsi_dev(d_info);
			continue;
		}
	}
	closedir(dir);

	return 0;
}

int list_tape_devs(struct scsi_device_info *d_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		disk_path[512];

	print_trace_enter();

	dir = opendir("/dev");
	if (unlikely(!dir))
		return -ENODEV;

	print_command_label("Tape Drives");

	print_disk_header();

	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "/dev/%s", entry->d_name);
		if ((!strncmp(entry->d_name, "st", 2) &&
		    (strncmp(entry->d_name, "std", 3))) ||
		    !strncmp(entry->d_name, "mt", 2)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENOSPC;

			d_info->disk_path = strdup(disk_path);
			d_info->disk_name = strdup(entry->d_name);
			get_disk_type(d_info);
			snprintf(d_info->disk_type, sizeof(d_info->disk_type),
			    dev_type_to_dev_name(d_info->device_type));
			printf("%s: Tape Device %s\n", __func__, disk_path);

			put_scsi_dev(d_info);
			continue;
		}
	}
	closedir(dir);

	return 0;
}

int list_generic_devs(struct scsi_device_info *d_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		disk_path[512];

	print_trace_enter();

	dir = opendir("/dev");
	if (unlikely(!dir))
		return -ENODEV;

	print_command_label("Generic Devices");

	print_generic_disk_header();

	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "/dev/%s", entry->d_name);
		if (!strncmp(entry->d_name, "sg", 2) ||
		    !strncmp(entry->d_name, "ng", 2)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENOSPC;

			d_info->device_type = GENERIC_DEV;
			d_info->disk_path = strdup(disk_path);
			d_info->disk_name = strdup(entry->d_name);
			get_device_numbers(entry->d_name, d_info);
			get_hctl_info(d_info);

			if (!strncmp(entry->d_name, "ng", 2))
				snprintf(d_info->disk_type, sizeof(d_info->disk_type),
				    "%s", "NVMe Generic");
			else
				snprintf(d_info->disk_type, sizeof(d_info->disk_type),
				    "%s", "SCSI Generic");

			get_disk_vendor_model(d_info);
			print_generic_disk_info(d_info);

			put_scsi_dev(d_info);
			continue;
		}
	}
	closedir(dir);

	return 0;
}

int list_multipath_devs(struct scsi_device_info *d_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		disk_path[512] = { 0 };

	print_trace_enter();

	dir = opendir(SYSFS_BLOCK_PATH);
	if (unlikely(!dir))
		return -ENODEV;

	print_command_label("Multipath Devices");

	print_mpath_disk_header();

	for_each_dir(entry, dir) {
		snprintf(disk_path, sizeof(disk_path), "%s/%s", SYSFS_BLOCK_PATH, entry->d_name);
		if (!strncmp(entry->d_name, "dm-", 2)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENOSPC;

			d_info->device_type = UNKNOWN_DEVICE;
			d_info->disk_path = strdup(disk_path);
			d_info->disk_name = strdup(entry->d_name);
			get_device_numbers(entry->d_name, d_info);
			get_hctl_info(d_info);
			get_disk_vendor_model(d_info);
			print_mpath_disk_info(d_info);

			put_scsi_dev(d_info);
			continue;
		}
		if (!strncmp(entry->d_name, "md", 2)) {
			print_trace_enter();

			d_info = alloc_scsi_dev();
			if (!d_info)
				return -ENOSPC;

			d_info->device_type = UNKNOWN_DEVICE;
			d_info->disk_path = strdup(disk_path);
			d_info->disk_name = strdup(entry->d_name);
			get_device_numbers(entry->d_name, d_info);
			get_hctl_info(d_info);
			get_disk_vendor_model(d_info);
			print_mpath_disk_info(d_info);

			put_scsi_dev(d_info);
			continue;
		}
	}
	closedir(dir);


	return 0;
}

int get_disk_type(struct scsi_device_info *d_info)
{
	char	path[1024], buf[8];
	char	*end;

	print_trace_enter();

	print_debug("Device path %s", d_info->disk_path);

	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "type");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	d_info->device_type = strtoull(buf, &end, 0);

	print_debug("Device Path: %s Device Type: %d\n",
	    d_info->disk_path, d_info->device_type);
	print_debug("Device Type %d\n", d_info->device_type);

	return 0;
}

int get_disk_queue_data(struct scsi_device_info *sdev_info)
{
	char	scsi_path[256];
	char	path[2048];
	char	buf[32];
	char	*end;
	int	err = 0;

	print_trace_enter();

	snprintf(scsi_path, sizeof(scsi_path), "%s/%s", sdev_info->disk_path, "queue");

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "chunk_sectors");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.chunk_sectors = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fua");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.fua = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "hw_sector_size");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.hw_sector_size = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "io_poll");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.io_poll = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "io_poll_delay");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.io_poll_delay = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "iostats");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.iostats = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "io_timeout");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.io_timeout = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "physical_block_size");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.physical_block_size = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "logical_block_size");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.logical_block_size = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "minimum_io_size");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.minimum_io_size = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "optimal_io_size");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.optimal_io_size = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "discard_zeroes_data");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.discard_zeroes_data = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "discard_max_hw_bytes");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.discard_max_hw_bytes = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "discard_max_bytes");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.discard_max_bytes = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "nr_zones");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.nr_zones = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "nomerges");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.nomerges = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "nr_requests");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.nr_requests = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "zone_append_max_bytes");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.zone_append_max_bytes = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "zone_write_granularity");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.zone_write_granularity = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "zoned");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.zoned = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "read_ahead_kb");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.read_ahead_kb = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "write_same_max_bytes");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.write_same_max_bytes = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "write_zeroes_max_bytes");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.write_zeroes_max_bytes = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "max_discard_segments");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.max_discard_segments = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "max_hw_sectors_kb");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.max_hw_sectors_kb = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "max_integrity_segments");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.max_integrity_segments = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "max_sectors_kb");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.max_sectors_kb = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "max_segments");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.max_segments = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "max_segment_size");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.max_segment_size = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "rq_affinity");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.rq_affinity = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "stable_writes");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.stable_writes = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "rotational");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.rotational = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "add_random");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.add_random = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "virt_boundary_mask");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.virt_boundary_mask = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "wbt_lat_usec");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.wbt_lat_usec = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "dax");
	snprintf(buf, sizeof(buf), open_sysfs_stats_file(path));
	sdev_info->q_data.dax = strtoull(buf, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "write_cache");
	snprintf(buf, sizeof(buf), "%s", open_sysfs_stats_file(path));
	sdev_info->q_data.write_cache = strdup(buf);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "scheduler");
	snprintf(buf, sizeof(buf), "%s", open_sysfs_stats_file(path));
	sdev_info->q_data.scheduler = strdup(buf);

	return err;
}

int get_single_nvme_disk_details(char *disk_name, struct scsi_device_info *d_info)
{
	char		path[1024], disk_path[256];
	char		*end;
	int		err = 0;
	char		temp_str[64] = { 0 };

	print_trace_enter();

	print_debug("Disk Name: %s\n", disk_name);

	snprintf(disk_path, sizeof(disk_path), "%s/%s", SYSFS_BLOCK_PATH, disk_name);
	d_info->disk_path = strdup(disk_path);
	d_info->disk_name = strdup(disk_name);

	print_debug("disk_path %s, disk_name %s\n", d_info->disk_path,
	    d_info->disk_name);

	/*  /sys/block/nvme0n1/device/model */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "model");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->model = strdup(temp_str);

	/*  /sys/block/nvme0n1/device/firmware_rev */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "firmware_rev");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->rev = strdup(temp_str);

	/* /sys/block/nvme0n1/device/queue_count */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "queue_count");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->queue_depth = strtoul(temp_str, &end, 0);

	/*  /sys/block/nvme0n1/device/state */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "state");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->state = strdup(temp_str);

	/* /sys/block/nvme0n1/device/address */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "address");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->pci_address = strdup(temp_str);
	print_debug("%s: %s\n", path, d_info->pci_address);

	/* /sys/block/nvme0n1/device/cntlid */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "cntlid");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->cntlid = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/device/serial */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "serial");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->serial = strdup(temp_str);

	/* /sys/block/nvme0n1/device/dctype */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "dctype");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->dctype = strdup(temp_str);

	/* /sys/block/nvme0n1/device/transport */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "transport");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->transport = strdup(temp_str);

	/* /sys/block/nvme0n1/device/cntrltype */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "cntrltype");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->cntrltype = strdup(temp_str);

	/* /sys/block/nvme0n1/device/kato */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "kato");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->kato = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/device/nuse */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "nuse");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->nuse = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/device/sqsize */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "sqsize");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->sqsize = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/nguid */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "nguid");
	snprintf(d_info->nguid, 64, open_sysfs_stats_file(path));

	/* /sys/block/nvme0n1/nsid */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "nsid");
	snprintf(d_info->nsid, sizeof(temp_str), open_sysfs_stats_file(path));

	/* /sys/block/nvme0n1/uuid */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "uuid");
	snprintf(d_info->uuid, sizeof(temp_str), open_sysfs_stats_file(path));

	/* /sys/block/nvme0n1/alignment_offset */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "alignment_offset");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->alignment_offset = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/discard_alignment */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "discard_alignment");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->discard_alignment = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/size  */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "size");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->size = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/capability */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "capability");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->capability = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/ext_range */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "ext_range");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->ext_range = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/range */
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "range");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->range = strtoull(temp_str, &end, 0);

	/* /sys/block/nvme0n1/wwid */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "wwid");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->wwid = strdup(temp_str);

	get_disk_queue_data(d_info);

	print_nvme_disk_details(d_info);

	return err;
}

int get_single_scsi_disk_details(char *disk_name, struct scsi_device_info *d_info)
{
	char		path[1024], temp_disk_path[128];
	char		temp_str[64] = { 0 };
	char		*end;
	int		err = 0;

	print_trace_enter();

	print_debug("Disk Name: %s\n", disk_name);

	snprintf(temp_disk_path, sizeof(temp_disk_path), "%s/%s", SYSFS_BLOCK_PATH, disk_name);
	d_info->disk_path = strdup(temp_disk_path);
	d_info->disk_name = strdup(disk_name);

	snprintf(d_info->disk_type, sizeof(d_info->disk_type), "disk");

	print_debug("disk_path %s, disk_name %s\n", d_info->disk_path,
	    d_info->disk_name);

	/* /sys/block/sda/device/model */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "model");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->model = strdup(temp_str);

	/* /sys/block/sda/device/timeout */
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "timeout");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->timeout = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/rev */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "rev");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->rev = strdup(temp_str);

	/* /sys/block/sda/device/dh_state */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "dh_state");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->state = strdup(temp_str);

	/* /sys/block/sda/device/iotmo_cnt */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "iotmo_cnt");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->iotmo_cnt = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/iodone_cnt */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "iodone_cnt");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->iodone_cnt = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/ioerr_cnt */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "ioerr_cnt");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->ioerr_cnt = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/iorequest_cnt */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "iorequest_cnt");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->iorequest_cnt = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/iocounterbits */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "iocounterbits");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->iocounterbits = strtoull(temp_str, &end, 0);

	/*  Scsi Device change Event Notifications */

	/* /sys/block/sda/device/evt_capacity_change_reported */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path,
	    "evt_capacity_change_reported");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->evt_capacity_change_reported = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/evt_inquiry_change_reported */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path,
	    "evt_inquiry_change_reported");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->evt_inquiry_change_reported = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/evt_lun_change_reported */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path,
	    "evt_lun_change_reported");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->evt_lun_change_reported = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/evt_media_change */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path,
	    "evt_media_change");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->evt_media_change = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/evt_mode_parameter_change_reported */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path,
	    "evt_mode_parameter_change_reported");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->evt_mode_parameter_change_reported =
	    strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/evt_soft_threshold_reached */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path,
	    "evt_soft_threshold_reached");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->evt_soft_threshold_reached = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/queue_type */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "queue_type");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->queue_type = strdup(temp_str);

	/* /sys/block/sda/device/queue_depth */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "queue_depth");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->queue_depth = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/alignment_offset */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "alignment_offset");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->alignment_offset = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/discard_alignment */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "discard_alignment");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->discard_alignment = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/size  */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "size");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->size = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/capability */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "capability");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->capability = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/ext_range */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "ext_range");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->ext_range = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/range */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/%s", d_info->disk_path, "range");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->range = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/wwid */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "wwid");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->wwid = strdup(temp_str);

	/* /sys/block/sda/device/max_sectors */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "max_sectors");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->max_sectors = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/eh_timeout */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "eh_timeout");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->eh_timeout = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/cdl_enabled */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "cdl_enabled");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->cdl_enabled = strtoull(temp_str, &end, 0);

	/* /sys/block/sda/device/cdl_supported */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "cdl_supported");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->cdl_supported = strtoull(temp_str, &end, 0);

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/FUA */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/protection_mode */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/protection_type */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/provisioning_mode */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/max_retries */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/max_write_same_blocks */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/max_medium_access_timeouts */

	/* /sys/block/sdb/device/scsi_disk/0:2:1:0/zoned_cap */

	/* /sys/block/sda/device/vendor */
	memset(temp_str, 0, sizeof(temp_str));
	snprintf(path, sizeof(path), "%s/device/%s", d_info->disk_path, "vendor");
	snprintf(temp_str, sizeof(temp_str), open_sysfs_stats_file(path));
	d_info->vendor = strdup(temp_str);
	print_debug("%s: %s \n", path, d_info->vendor);

	get_disk_queue_data(d_info);

	print_scsi_disk_details(d_info);

	return err;
}

int show_enclosure_details(char *argv[], struct scsi_device_list *sdev)
{
	struct scsi_device_info *disk_info = sdev->disk_info;

	print_trace_enter();

	if (!strcmp(argv[2], "enclosure"))
		return -EINVAL;

	if (disk_info->disk_name)
		printf("%s: %s\n", __func__, disk_info->disk_name);

	return 0;
}

int show_disk_details(char *argv[], struct scsi_device_list *sdev)
{
	struct dirent		*entry;
	DIR			*dir;
	char			disk_name[16] = { 0 };
	int			count = 0;
	int			is_nvme = 0;

	print_trace_enter();

	dir = opendir(SYSFS_BLOCK_PATH);
	if (unlikely(!dir))
		return -ENODEV;

	print_debug("Input %s: %s\n", argv[2], argv[3]);

	snprintf(disk_name, sizeof(disk_name), argv[3]);

	if (!strncmp(disk_name, "nvme", 4))
		is_nvme = 1;

	print_debug("Show Details for %s, Len %ld is_nvme: %d\n",
	    disk_name, strlen(disk_name), is_nvme);

	for_each_dir(entry, dir) {
		print_debug("Disk Name: %s (%s)\n", disk_name, entry->d_name);

		if (!strncmp(entry->d_name, disk_name, strlen(disk_name)) &&
		    is_nvme) {
			print_trace_enter();
			print_debug("Disk Name: %s (%s)\n", disk_name,
				    entry->d_name);
			get_single_nvme_disk_details(disk_name, sdev->disk_info);
			count++;
			continue;
		} else if (!strncmp(disk_name, entry->d_name,
			            strlen(disk_name))) {
			print_trace_enter();
			print_debug("Disk Name: %s (%s)\n", disk_name,
				    entry->d_name);
			get_single_scsi_disk_details(disk_name, sdev->disk_info);
			count++;
			continue;
		}
	}
	closedir(dir);

	return (count == 1) ? 1 : -ENODEV;
}

/**
 * Get Eror Count for a disk device
 */
int get_disk_error_count(char **argv, struct scsi_device_info *d_info)
{
	int	err = 0;
	char	disk_name[16];

	if (strlen(argv[3]) == 0)
		return -EIO;

	snprintf(disk_name, sizeof(disk_name), argv[3]);

	d_info = alloc_scsi_dev();
	if (!d_info)
		return -ENODEV;

	d_info->disk_name = strdup(disk_name);

	put_scsi_dev(d_info);

	return err;
}

/**
 * Issue Scsi scan for a disk device
 */
int scsi_scan_disk_dev(char **argv, struct scsi_device_info *d_info)
{
	int	err = 0;
	char	disk_name[16];

	if (strlen(argv[3]) == 0)
		return -EIO;

	snprintf(disk_name, sizeof(disk_name), argv[3]);

	/* Verify that the scsi deice is not a boot device */
	d_info = alloc_scsi_dev();
	if (!d_info)
		return -ENODEV;

	d_info->disk_name = strdup(disk_name);

	put_scsi_dev(d_info);

	return err;
}

/**
 * Set scsi disk device offlinee
 */
int set_disk_offline(char **argv, struct scsi_device_info *d_info)
{
	int	err = 0;
	char	disk_name[16];

	if (strlen(argv[3]) == 0)
		return -EIO;

	snprintf(disk_name, sizeof(disk_name), argv[3]);

	/* Verify that the scsi deice is not a boot device */
	d_info = alloc_scsi_dev();
	if (!d_info)
		return -ENODEV;

	d_info->disk_name = strdup(disk_name);

	put_scsi_dev(d_info);

	return err;
}

/**
 * Set scsi disk device online
 */
int set_disk_online(char **argv, struct scsi_device_info *d_info)
{
	int	err = 0;
	char	disk_name[16];

	if (strlen(argv[3]) == 0)
		return -EIO;

	snprintf(disk_name, sizeof(disk_name), argv[3]);

	/* Verify that the scsi deice is not a boot device */
	d_info = alloc_scsi_dev();
	if (!d_info)
		return -ENODEV;

	d_info->disk_name = strdup(disk_name);

	put_scsi_dev(d_info);

	return err;
}

/**
 * Set scsi disk device online
 */
int set_disk_alias(char **argv, struct scsi_device_info *d_info)
{
	int	err = 0;
	char	disk_name[16];

	if (strlen(argv[3]) == 0)
		return -EIO;

	snprintf(disk_name, sizeof(disk_name), argv[3]);

	/*
	 * Verify that the scsi disk is valid disk and the destination
	 * alias format does not have special characters
	 */

	d_info = alloc_scsi_dev();
	if (!d_info)
		return -ENODEV;

	d_info->disk_name = strdup(disk_name);

	put_scsi_dev(d_info);

	return err;
}
