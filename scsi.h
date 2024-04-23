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

#ifndef SCSI_H
#define SCSI_H

#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <inttypes.h>

#include <locale.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>
#include <dirent.h>
#include <libgen.h>
#include <signal.h>
#include <limits.h>
#include <ctype.h>

#include <linux/fs.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#include <sys/random.h>

#include "utils.h"
#include "scsi_cmds.h"

/* Enable Debugging for the tool */
#define debug		0

/* Enable Funtion tracing for the tool */
#define trace		0

#define	SCSI_TOOL_NAME		"scsi-cli"
#define SCSI_TOOL_VERSION	"v0.1"

#define MAX_SYSFS_PATH_LEN	120

#define SYSFS_FC_HOST_PATH	"/sys/class/fc_host"
#define SYSFS_FC_RPRT_PATH	"/sys/class/fc_remote_ports"
#define SYSFS_ISCSI_HOST_PATH	"/sys/class/iscsi_host"
#define SYSFS_ISCSI_SESS_PATH	"/sys/class/iscsi_session"
#define SYSFS_ISCSI_CONN_PATH	"/sys/class/iscsi_connection"
#define SYSFS_BLOCK_PATH	"/sys/block"
#define SYSFS_SCSI_DEV_PATH	"/sys/class/scsi_device"
#define SYSFS_SCSI_GEN_PATH	"/sys/class/scsi_generic"
#define SYSFS_SCSI_HOST_PATH	"/sys/class/scsi_host"
#define SYSFS_SCSI_DISK_PATH	"/sys/class/scsi_disk"

#define PCI_BUS_PATH		"/sys/bys/pci"
#define RESCAN_PCI_PATH		"/sys/bus/pci/rescan"

#define DEV_DIR_PATH		"/dev"
#define DEV_NAME_LEN		1024

#define CMD_NAME_LEN		32
#define CMD_DESC_LEN		1024

enum scsi_device_type {
	DIRECT_ACCESS_BLOCK_DEVICE	= 0,
	SEQ_ACCESS_DEVICE		= 1,
	PRINTER_DEVICE			= 2,
	PROCESSOR_DEVICE		= 3,
	WRITE_ONCE_DEVICE		= 4,
	CD_DVD_ROM_DEVICE		= 5,
	SCANNER_DEVICE			= 6,
	OPTICAL_MEMORY_DEVICE		= 7,
	MEDIUM_CHANGER_DEVICE		= 8,
	COMMUNICATION_DEVICE		= 9,
	STORAGE_ARRAY_CNTROLLER		= 12, /* 0x0C */
	ENCLOSURE_DEVICE		= 13, /* 0x0D */
	SIM_DIRECT_ACCESS_DEVICE	= 14, /* 0x0E */
	OPTICAL_CARD_RW_DEVICE		= 15, /* 0x0F */
	OBJECT_BASED_STORAGE_DEVICE	= 17, /* 0x11 */
	AUTOMATION_INTERFACE		= 18, /* 0x12 */
	SECURITY_MANAGER_DEVICE		= 19, /* 0x13 */
	HOST_MANAGED_ZBD		= 20, /* 0x14 */
	WELL_KNOWN_LU			= 30, /* 0x1E */
	UNKNOWN_DEVICE			= 31  /* 0x1F */
};

enum device_type {
	NVME_DEV = 1,
	ATA_DEV,
	SCSI_DEV,
	FC_DEV,
	SAS_DEV,
	DM_DEV,
	MD_DEV,
	GENERIC_DEV,
	CDROM_DEV,
	TAPE_DEV,
	ENCLOSURE_DEV
};

static const char dash[224] = {[0 ... 223] = '-'};
static const char space[32] = {[0 ... 31] = ' '};

#define	KB_SIZE 	1024

#define LOGICAL_SECTOR_SIZE	512
#define PHYSICAL_SECTOR_SIZE	4096

/*
#define SCSI_DEVICES	1024

static unsigned int scsi_devices = SCSI_DEVICES * sizeof(struct list_head);
static struct list_head *scsi_device_list;
*/

/*  To store collected disk statistics */
/*  Array members represents index 0 for read and index 1 for write */
struct disk_stats {
	uint64_t 	ios[2];
	uint64_t	merges[2];
	uint64_t	sectors[2];
	uint64_t	ticks[2];
	uint64_t	io_ticks;
	uint64_t	time_in_queue;
	uint64_t	msec;
};

struct disk_queue_data {
	/* Disk details from /sys/block/sdX/queue/ dir */
	u64	chunk_sectors;
	u64	fua;
	u64	hw_sector_size;
	u64	io_poll;
	u64	io_poll_delay;
	u64	iostats;
	u64	io_timeout;

	u64	wbt_lat_usec;
	u64	dax;
	u64	add_random;
	u64	virt_boundary_mask;
	u64	stable_writes;
	u64	rotational;
	u64	rq_affinity;
	
	char	*scheduler;
	char	*write_cache;

	u64	physical_block_size;
	u64	logical_block_size;
	u64	minimum_io_size;
	u64	optimal_io_size;

	u64	discard_zeroes_data;
	u64	discard_max_hw_bytes;
	u64	discard_granularity;
	u64	discard_max_bytes;

	u64	nomerges;
	u64	nr_requests;
	u64	nr_zones;
	u64	zone_append_max_bytes;
	u64	zone_write_granularity;
	u64	zoned;

	u64	read_ahead_kb;
	u64	write_same_max_bytes;
	u64	write_zeroes_max_bytes;

	u64	max_discard_segments;
	u64	max_hw_sectors_kb;
	u64	max_integrity_segments;
	u64	max_sectors_kb;
	u64	max_segments;
	u64	max_segment_size;

	/*  data from /sys/block/sdX/queue/iosched dirctory */
	u64	async_depth;
	u64	fifo_batch;
	u64	front_merges;
	u64	read_expire;
	u64	write_expire;
	u64	writes_starved;
};

struct scsi_device_info {
	struct list_head	scsi_list;

	char			*alias_name;		/* user given name */

	/* To capture Disk Statistics */
	struct disk_stats	dstat;

	/*  information for each discovered disk */
	uint32_t	major;
	uint32_t	minor;

	/* Slot location for the device, eg. [0:0:0:0] */
	int	host;
	int	bus;
	int	target;
	int	lun;

	int	device_type; /* Encode Device type for printing */
	char 	disk_type[32]; /* Type of Disk */
	char	*disk_path; /*path for this device */
	char	*disk_name;	/*System disk name */
	char	*sysfs_root; /*sysfs disk path */
	char	*pci_id;	/* To extract Vendor Id, device id */
	char	*driver_name; /* Driver used for this device */

	/* Disk details from /sys/block/sdX dir */
	char	*model;
	char	*rev;
	char 	*wwid;	/* for the disk which has UUID */
	char	*queue_type;
	char	*state;
	char 	*vendor;

	struct disk_queue_data q_data;

	int	capability;
	int	ext_range;
	int	range;

	/* Disk details from /sys/block/sdX/device dir */
	int	timeout;
	int	eh_timeout;
	int	queue_depth;
	int	cdl_enabled;
	int	cdl_supported;

	u64	max_sectors;
	u64	alignment_offset;
	u64	discard_alignment;
	u64	size;

	u64	iodone_cnt;
	u64	ioerr_cnt;
	u64	iorequest_cnt;
	u64	iotmo_cnt;
	u64	iocounterbits;

	int	evt_capacity_change_reported;
	int	evt_inquiry_change_reported;
	int	evt_lun_change_reported;
	int	evt_media_change;
	int	evt_mode_parameter_change_reported;
	int	evt_soft_threshold_reached;

	char	*zoned_cap;	/* for Zoned device */

	int	fua;
	int	protection_mode;
	int	protection_type;

	int	max_retries;
	int	max_write_same_blocks;
	int	max_medium_access_timeouts;

	/*  NVMe specific disk details */
	char 	nsid[64];	/* NameSpace ID */
	char 	nguid[64];	/* NGUID */
	char 	uuid[64];	/* for the disk which has UUID */
	int	cntlid;		/* Controller ID */
	int	kato;		/* Keep Alive Time Out */
	char 	subsysnqn[256]; /* Subsystem NQN */
	char	*serial;	/* Serial Number */
	int	sqsize;		/* Submission Queue size */
	int	numa_node;	/*  NUMA Node */
	char	*dctype;	/* Discovery Controller Type */
	char	*cntrltype;	/* Controller Type */
	char	*transport;	/* NVMe Transport  */
	u64	nuse;		/* Namespace Utilization */

	/*
	 * NVMe device pci bus infomation at sysfs
	 * location /sys/block/nvmeXnY/device/address (0000:94:00.0)
	 */
	char	*pci_address;
};

struct scsi_device_list {
	struct list_head	scsi_device_list;

	char			*scsi_tool_name;	/* SCSI CLI tool name */
	char			*scsi_tool_version;	/* SCSI CLI tool Version */
	struct scsi_device_info	*disk_info;		/* SCSI Disk info */
	struct fc_device_info	*fc_info;		/* FCP information */
	struct iscsi_dev_info	*iscsi_info;		/* iSCSI information */
};

enum scsi_driver_module {
	sd_mod = 1,
	scsi_debug,
	scsi_transport_fc,
	scsi_transport_iscsi,
	t10_pi
};

enum nvme_driver_module {
	nvme = 1,
	nvme_core,
	nvme_fc,
	nvme_tcp,
	nvme_fabrics,
	nvmet,
	nvmet_fc
};

/* Helper function for memory allocation and free */
struct fc_device_info *alloc_fc_dev(void);
struct scsi_device_info *alloc_scsi_dev(void);
struct iscsi_dev_info *alloc_iscsi_dev(void);
void put_scsi_dev(struct scsi_device_info *);
void put_fc_dev(struct fc_device_info *);
void put_iscsi_dev(struct iscsi_dev_info *);

/* Various functions to display command handling help */
void usage(void);
int validate_usage(void);
void version_cmd();
void general_help();
int help_cmd(int, char **);
void handle_cmd_errors(char **, struct scsi_device_list *);

/* functions for command validation */
void list_builtins();
int validate_command(char *);
int validate_subcommand(char **);

/* Functions to get sysfs information */
int validate_sysfs_path(char *);
int remove_space(char *);
int remove_newline(char *);
int remove_int(char *);
char *open_sysfs_stats_file(char *);

/* Functions to display various list options  */
int list_enclosure(struct scsi_device_info *);
int list_controllers(struct scsi_device_info *);
int list_block_devs(struct scsi_device_info *);
int list_generic_devs(struct scsi_device_info *);
int list_tape_devs(struct scsi_device_info *);
int list_cdrom_devs(struct scsi_device_info *);
int list_multipath_devs(struct scsi_device_info *);

/* functions for collection disk information */
int validate_enclosure_type(struct scsi_device_info *);
int show_enclosure_details(char **, struct scsi_device_list *);
int show_disk_details(char **, struct scsi_device_list *);
int get_disk_stats(struct scsi_device_info *, char *);
int get_disk_vendor_model(struct scsi_device_info *);
int get_disk_type(struct scsi_device_info *d_info);
int get_disk_queue_data(struct scsi_device_info *);
int get_single_nvme_disk_details(char *, struct scsi_device_info *);
int get_single_scsi_disk_details(char *, struct scsi_device_info *);
int get_device_numbers(char *, struct scsi_device_info *);
int get_device_count(char *);
int get_hctl_info(struct scsi_device_info *);
char *get_device_entry(char *);
int get_enclosure_details(struct scsi_device_info *);
int get_disk_error_count(char **, struct scsi_device_info *);
int scsi_scan_disk_dev(char **, struct scsi_device_info *);
int set_disk_offline(char **, struct scsi_device_info *);
int set_disk_online(char **, struct scsi_device_info *);
int set_disk_alias(char **, struct scsi_device_info *);
#endif
