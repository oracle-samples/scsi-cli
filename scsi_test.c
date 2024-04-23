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
#include "utils.h"
#include "scsi_cmds.h"
#include "scsi_print.h"
#include "scsi_fcp.h"
#include "scsi_iscsi.h"

static unsigned padding = 15;

static struct supported_cmds cmd_str[] = {
	{ "alias",	cmd_alias,	"Create handy alias for disk, adater, controller"},
	{ "errors",	cmd_errors,	"Show errors for a device" },
	{ "list",	cmd_list,	"List all device on this host"	},
	{ "online",	cmd_online,	"Change device state to online" },
	{ "offline",	cmd_offline,	"Change device state to offline" },
	{ "reset",	cmd_reset,	"Reset a device" },
	{ "stats",	cmd_stats,	"Show statistics of a device" },
	{ "show",	cmd_show,	"Show details" },
	{ "scan",	cmd_scan,	"Scan a system for device" },
};

static struct supported_sub_cmds sub_cmd_str[] = {
	/* subcommand options for list */
	{ "list",	"cd-rom",	"List CD-ROM devices from the host" },
	{ "list",	"tape",		"List Tape devices from the host" },
	{ "list",	"medium-changer", "List Medium Changer devices from the host" },
	{ "list",	"enclosure",	"List Enclosure devices from the host" },
	{ "list",	"fc_hba",	"List Fiber Channel HBA instlled" },
	{ "list",	"iscsi",	"List iSCSI Sessions" },
	{ "list",	"disk",		"List disks from the host" },
	{ "list",	"controller",	"List disk controllers from the host" },
	{ "list",	"generic",	"List generic disk from the host" },
	{ "list",	"multipath",	"List multipath disk from the host" },

	/* subcommand options for show */
	{ "show",	"disk",		"Show details of a disk" },
	{ "show",	"fc_port",	"show details of an fc port" },
	{ "show",	"iscsi",	"show details of an iscsi host" },

	/* Subcommand options for scan */
	{ "scan",	"block",	"Rescan system for block devices" },
	{ "scan",	"fc_hba",	"Rescan system for FC adapters" },

	/* subcommand options for reset */
	{ "reset",	"adapter",	"Reset a given adapter" },
	{ "reset",	"port",		"Reset a port" },
	{ "reset",	"issue_logo",	"force device to issue logout", },
	{ "reset",	"issue_lip",	"Issue Loop Initialization Primitive" },

	/* subcommand options for stats */
	{ "stats",	"disk",		"Show statistics for block device" },
	{ "stats",	"fc_port",	"Show statistics for Fiber Channel port" },
};

/**
 * list_subcommands() will iterate through the array to find command and display
 * supported subcommands.
 */
void list_subcommands(const char *cmd)
{
	size_t  i;
	struct supported_sub_cmds *p;

	for (i = 0; i < ARRAY_SIZE(sub_cmd_str); i++) {
		p = sub_cmd_str + i;

		/* Search for the first occurance of the command */
		if (strcmp(cmd, p->cmd) == 0) {
			printf("\n%-.2sFollowing Sub-Commands are supported for '%s'\n",
			    space, cmd);
			printf("%-.2s%-.64s\n", space, dash);
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(sub_cmd_str); i++) {
		p = sub_cmd_str + i;

		/*  Print sub-command and description as a list */
		if (strcmp(cmd, p->cmd) == 0)
			printf("%-.2s%-*s%-.4s%s\n", space, padding, p->sub_cmd,
			    space, p->sub_cmd_desc);
	}
	printf("\n");
}

/**
 * list_builtins() will iterate through the array to display
 * subcommands for the issued command
 */
void list_builtins()
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(cmd_str); i++) {
		struct supported_cmds *p = cmd_str + i;
		printf("%-.4s%-*s%-.4s%s\n", space, padding, p->cmds, space, p->cmd_desc);
	}
}

/**
 * validate_command() will iterate through the array to
 * verify if the command issues is one of the supported
 * commnad by the tool
 */
int validate_command(char *cmd)
{
	size_t i;

	print_trace_enter();

	for (i = 0; i < ARRAY_SIZE(cmd_str); i++) {
		struct supported_cmds *p = cmd_str + i;

		print_debug("Searching for command '%s' ", cmd);

		if (strcmp(cmd, p->cmds) == 0) {
			print_debug("Found Command match %s (%s)",
			    cmd, p->cmds);
			return 0;
		}
	}

	print_debug("Command '%s' not found", cmd);

	return -EAGAIN;
}

/**
 * validate_subcommand() will iterate through the array to
 * verify if the sub-ommand issued is one of the supported
 * sub-commnad option
 */
int validate_subcommand(char **argv)
{
	size_t i;
	char *cmd = argv[1];
	char *sub_cmd = argv[2];

	print_trace_enter();

	print_debug("Command '%s', Sub command '%s'", cmd, sub_cmd);

	for (i = 0; i < ARRAY_SIZE(cmd_str); i++) {
		struct supported_cmds *p = cmd_str + i;
		if (strcmp(cmd, p->cmds) == 0) {
			print_debug("found Command '%s'", cmd);
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(sub_cmd_str); i++) {
		struct supported_sub_cmds *p = sub_cmd_str + i;

		print_debug("Searching for %s ", sub_cmd);
		if ((strcmp(cmd, p->cmd) == 0) &&
		    (strcmp(sub_cmd, p->sub_cmd) == 0)) {
			print_debug("Found sub-command match %s (%s)",
			    sub_cmd, p->sub_cmd);

			return 0;
		}
	}

	print_debug("sub command '%s' not supported for '%s' ", sub_cmd, cmd);

	return -ENOENT;
}

/**
 * handle_cmd_errors() will handle erros with the issued command
 */
void handle_cmd_errors(char **argv, struct scsi_device_list *s_dev)
{
	list_subcommands(argv[1]);

	list_enclosure(s_dev->disk_info);

	list_controllers(s_dev->disk_info);

	list_block_devs(s_dev->disk_info);

	list_multipath_devs(s_dev->disk_info);

	list_fc_adapters(s_dev->fc_info);

	list_iscsi_devs(s_dev->iscsi_info);
}

/**
 * cmd_alias() will create alias for your device to user defined name
 */
int cmd_alias(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;

	if (argc > 2) {
		print_info("Set '%s' for device to your favourite name", argv[2]);

		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		if (strncmp(argv[2], "disk", 4) == 0)
			err = set_disk_alias(argv, s_dev->disk_info);
		else if (strncmp(argv[2], "fc_port", 7) == 0)
			err = set_fcport_alias(argv, s_dev->fc_info);
		else if  (strncmp(argv[2], "iscsi", 5) == 0)
			err = set_iscsi_alias(argv, s_dev->iscsi_info);
		else
			print_info("Unknown Sub-command '%s' \n", argv[2]);

		if (err < 0)
			return err;
	} else {
		print_info("(%d) Command '%s' is not currently supported", argc, argv[1]);
		/* handle_cmd_errors(argv); */
	}

	return err;
}

/**
 * cmd_list() will list all the devices found in the system
 */
int cmd_list(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;

	print_trace_enter();

	if (argc > 2) {
		print_trace_enter();

		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		if (!strcmp(argv[2], "fc_hba")) {
			err = list_fc_adapters(s_dev->fc_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "iscsi")) {
			err = list_iscsi_devs(s_dev->iscsi_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "controller")) {
			err = list_controllers(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "disk")) {
			err = list_block_devs(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "multipath")) {
			err = list_multipath_devs(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "generic")) {
			err = list_generic_devs(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "enclosure")) {
			err = list_enclosure(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "cd-rom")) {
			err = list_cdrom_devs(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
		if (!strcmp(argv[2], "medium-changer")) {
			print_command_label("Medium-Changer");
		}
		if (!strcmp(argv[2], "tape")) {
			err = list_tape_devs(s_dev->disk_info);
			if (err < 0)
				goto err_out;
		}
	} else {
		handle_cmd_errors(argv, s_dev);
	}

	return err;

err_out:
	print_debug("%s: '%s' command failed with %d", argv[1], argv[2], err);

	return err;
}

/**
 * cmd_errors() will display error counts for a particular device
 */
int cmd_errors(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;

	print_trace_enter();

	if (argc > 2) {
		print_info("Get Error Details for %s", argv[3]);

		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		if (strncmp(argv[2], "disk", 4) == 0)
			err = get_disk_error_count(argv, s_dev->disk_info);
		else if (strncmp(argv[2], "fc_port", 7) == 0)
			err = get_fcport_error_count(argv, s_dev->fc_info);
		else if  (strncmp(argv[2], "iscsi", 5) == 0)
			err = get_iscsi_error_count(argv, s_dev->iscsi_info);
		else
			print_info("Unknown Sub-command '%s' \n", argv[2]);

		if (err < 0)
			return err;
	} else {
		print_info("(%d) Command '%s' is not currently supported", argc, argv[1]);
		/* handle_cmd_errors(argv); */
	}

	return 0;
}

/**
 * cmd_online() will toggle a particular device to an online state
 */
int cmd_online(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;
	print_trace_enter();

	if (argc > 2) {
		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		print_info("Call '%s' command", argv[1]);

		if (strncmp(argv[2], "disk", 4) == 0)
			err = set_disk_online(argv, s_dev->disk_info);
		else if (strncmp(argv[2], "fc_port", 7) == 0)
			err = set_fcport_online(argv, s_dev->fc_info);
		else if  (strncmp(argv[2], "iscsi", 5) == 0)
			err = set_iscsi_dev_online(argv, s_dev->iscsi_info);
		else
			print_info("Unknown Sub-command '%s' \n", argv[2]);

		if (err < 0)
			return err;
	} else {
		print_info("(%d) Command '%s' is not currently supported", argc, argv[1]);
		/* handle_cmd_errors(argv); */
	}

	return 0;
}

/**
 * cmd_offine() will toggle a particular device to an off-line state
 */
int cmd_offline(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;
	print_trace_enter();

	if (argc > 2) {
		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		print_info("Call '%s' command",argv[3]);

		if (strncmp(argv[2], "disk", 4) == 0)
			err = set_disk_offline(argv, s_dev->disk_info);
		else if (strncmp(argv[2], "fc_port", 7) == 0)
			err = set_fcport_offline(argv, s_dev->fc_info);
		else if  (strncmp(argv[2], "iscsi", 5) == 0)
			err = set_iscsi_dev_offline(argv, s_dev->iscsi_info);
		else
			print_info("Unknown Sub-command '%s' \n", argv[2]);

		if (err < 0)
			return err;
	} else {
		print_info("(%d) Command '%s' is not currently supported", argc, argv[1]);
		/* handle_cmd_errors(argv); */
	}

	return 0;
}

/**
 * cmd_reset() will reset a particular device
 */
int cmd_reset(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = -EINVAL;
	char disk_str[32] = { 0 };

	print_trace_enter();

	print_debug("argc %d", argc);

	if (argc > 2) {
		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		snprintf(disk_str, strlen(argv[3]), "%s", argv[3]);

		print_trace_enter();

		if (strncmp(argv[2], "adapter", 7) == 0) {
			print_debug("Issue Adapter Reset to %s", argv[3]);
			err = fc_hba_reset(argc, argv);
			if (err < 0)
				goto err_out;
		}
		if (strncmp(argv[2], "port", 4) == 0) {
			print_debug("Issue Port Reset to %s", argv[3]);
		}
		if (strncmp(argv[2], "issue_logo", 10) == 0) {
			/* path - /sys/class/fc_host/host10/device/issue_logo */
			print_debug("Issue FCP Port Logout for %s", argv[3]);
		}
		if (strncmp(argv[2], "issue_lip", 9) == 0) {
			/* path - /sys/class/fc_host/host10/issue_lip */
			print_debug("issue LIP for %s", argv[3]);
		}
	} else {
		handle_cmd_errors(argv, s_dev);
	}

	return 0;

err_out:
	return err;
}

/**
 * cmd_stats() will show statistical data about a device
 */
int cmd_stats(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int	len, err = -EINVAL;
	char	disk_str[32] = { 0 };

	print_trace_enter();

	if (argc > 2) {
		print_trace_enter();

		if (argv[3] == NULL) {
			print_info("Please provide %s name to display Statistics",
				argv[2]);
			return 0;
		}

		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		len = strlen(argv[3]) + 2;

		print_debug("%s: %s \n", argv[2], argv[3]);

		snprintf(disk_str, len,  "%s", argv[3]);

		if (strncmp(argv[2], "disk", 4) == 0) {
			print_trace_enter();

			s_dev->disk_info = alloc_scsi_dev();
			if (!s_dev->disk_info)
				return err;

			err = get_disk_stats(s_dev->disk_info, disk_str);
			if (err < 0) {
				print_err("Can not get statistics for %s, (err=%d)",
				    disk_str, err);
				put_scsi_dev(s_dev->disk_info);
				return 0;
			}
			print_disk_stats(&s_dev->disk_info->dstat, disk_str);

			put_scsi_dev(s_dev->disk_info);
		}

		if (strncmp(argv[2], "fc_port", 7) == 0) {
			print_trace_enter();

			s_dev->fc_info = alloc_fc_dev();

			if (!s_dev->fc_info)
				return err;

			err = get_fc_dev_stats(disk_str, s_dev->fc_info);
			if (err < 0) {
				print_err("Can not get statistics for %s, (err=%d)",
				    disk_str, err);
				put_fc_dev(s_dev->fc_info);
				return 0;
			}
			print_fc_port_stats(s_dev->fc_info);

			put_fc_dev(s_dev->fc_info);

		}
	} else {
		handle_cmd_errors(argv, s_dev);
	}

	return 0;
}

/**
 * cmd_show() will show details about a device
 */
int cmd_show(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;
	char device_str[16] = { 0 };

	print_trace_enter();

	if (argc > 2) {
		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		if (argc == 3 || argv[3] == NULL) {
			print_info("Please provide %s name to display details",
				argv[2]);
			return err;
		}

		if (!s_dev)
			return -EINVAL;

		snprintf(device_str, strlen(argv[3]), "%s", argv[3]);

		print_trace_enter();

		if (strncmp(argv[2], "disk", 4) == 0) {
			print_trace_enter();

			s_dev->disk_info = alloc_scsi_dev();
			if (!s_dev->disk_info)
				return -EINVAL;

			err = show_disk_details(argv, s_dev);
			if (err < 0)
				return err;

			put_scsi_dev(s_dev->disk_info);
		}

		if (strncmp(argv[2], "fc_port", 7) == 0) {
			print_trace_enter();

			s_dev->fc_info = alloc_fc_dev();
			if (!s_dev->fc_info)
				return -EIO;

			err = show_fc_port_details(argv, s_dev);
			if (err < 0)
				return err;

			put_fc_dev(s_dev->fc_info);
		}

		if (strncmp(argv[2], "enclosure", 9) == 0) {
			print_trace_enter();

			s_dev->disk_info = alloc_scsi_dev();
			if (!s_dev->disk_info)
				return -EINVAL;

			err = show_enclosure_details(argv, s_dev);
			if (err < 0)
				return err;

			put_scsi_dev(s_dev->disk_info);
		}

		if (strncmp(argv[2], "iscsi", 5) == 0) {
			print_trace_enter();

			s_dev->iscsi_info = alloc_iscsi_dev();
			if (!s_dev->iscsi_info)
				return -EINVAL;

			err = show_iscsi_details(argv, s_dev);
			if (err < 0)
				return err;

			put_iscsi_dev(s_dev->iscsi_info);
		}
	} else {
		print_trace_enter();
		handle_cmd_errors(argv, s_dev);
	}

	return err;
}

/**
 * cmd_scan() will issue a device scan using sysfs hook for a system
 */
int cmd_scan(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;

	print_trace_enter();

	if (argc < 2) {
		print_trace_enter();
		err = validate_subcommand(argv);
		if (err < 0)
			return err;

		if (strncmp(argv[2], "disk", 4) == 0)
			err = scsi_scan_disk_dev(argv, s_dev->disk_info);
		else if (strncmp(argv[2], "fc_port", 7) == 0)
			err = scsi_scan_fcport_dev(argv, s_dev->fc_info);
		else if  (strncmp(argv[2], "iscsi", 5) == 0)
			err = scsi_scan_iscsi_dev(argv, s_dev->iscsi_info);
		else
			print_info("Unknown Sub-command '%s' \n", argv[2]);

		if (err < 0)
			return err;
	} else {
		print_info("(%d) Command '%s' is not currently supported", argc, argv[1]);
		/* handle_cmd_errors(argv); */
	}

	return 0;
}

/**
 * parse_cmd() will check each command and call apropriate hook for a command
 * action
 */
int parse_cmd(int argc, char **argv, struct scsi_device_list *s_dev)
{
	int err = 0;
	char *cmd = argv[1];

	while(*cmd == '-')
		cmd++;

	print_trace_enter();

	print_debug("Input Command for %s : %s ",
	    argv[0], argv[1]);

	err = validate_command(cmd);
	if (err < 0)
		return err;

	if (strncmp(cmd, "list", 4) == 0)
		err = cmd_list(argc, argv, s_dev);

	if (strncmp(cmd, "show", 4) == 0)
		err = cmd_show(argc, argv, s_dev);

	if (strncmp(cmd, "scan", 4) == 0)
		err = cmd_scan(argc, argv, s_dev);

	if (strncmp(cmd, "reset", 5) == 0)
		err = cmd_reset(argc, argv, s_dev);

	if (strncmp(cmd, "alias", 4) == 0)
		err = cmd_alias(argc, argv, s_dev);

	if (strncmp(cmd, "stats", 4) == 0)
		err = cmd_stats(argc, argv, s_dev);

	if (strncmp(cmd, "online", 6) == 0)
		err = cmd_online(argc, argv, s_dev);

	if (strncmp(cmd, "offline", 7) == 0)
		err = cmd_offline(argc, argv, s_dev);

	if (strncmp(cmd, "errors", 6) == 0)
		err = cmd_errors(argc, argv, s_dev);

	if (err < 0)
		print_debug("%s: '%s' Command Failed %d ", argv[1], argv[2], err);

	return err;
}

/**
 * handle_cmd() will validate the input and call parse_cmd()
 */
int handle_cmd(int argc, char **argv)
{
	struct scsi_device_list *sdev = NULL;
	char 	*str = argv[1];
	int	ret = 0;

	print_trace_enter();

	if (!argc) {
		general_help();
		return ret;
	}

	sdev = malloc(sizeof(*sdev));
	if (!sdev) {
		ret = -EINVAL;
		goto err_out;
	}
	sdev->scsi_tool_name = strdup(SCSI_TOOL_NAME);
	sdev->scsi_tool_version = strdup(SCSI_TOOL_VERSION);

	while(*str == '-')
		str++;

	if ((strncmp(str, "help", 4) == 0) || (strncmp(str, "h", 1) == 0)) {
		ret = help_cmd(argc, argv);
		goto err_out;
	} else if ((strncmp(str, "version", 7) == 0) || (strncmp(str, "v", 1) == 0)) {
		version_cmd();
		goto err_out;
	} 

	ret = parse_cmd(argc, argv, sdev);
	if (ret < 0) {
		if (ret == -ENODEV) {
			print_info("\n %s: No Valid Device found \n", argv[0]);
		} else 	if (ret == -EAGAIN) {
			print_err("%s: Command '%s' not supported (%s) ",
			    argv[0], argv[1], strerror(ret));
		} else 	if (ret == -ENOENT) {
			print_err("%s: Sub Command '%s' not supported (%s)",
			    argv[0], argv[2], strerror(ret));
		} else {
			print_err("%s: Command %s returned error. (%s)",
			    argv[0], argv[1], strerror(ret));
		}
		ret = -EINVAL;
	} else if (ret == 0) {
		print_debug("%s: Command %s returned Sucessful",
		    argv[0], argv[1]);
	}

err_out:
	free(sdev);
	return ret;
}

int main(int argc, char **argv)
{
	int err = 0;

	print_trace_enter();

	if (argc < 2) {
		general_help();
		return 0;
	}

	err = handle_cmd(argc, argv);
	if (err == -ENOTTY) {
		general_help();
	}

	return err ? 1 : 0;
}
