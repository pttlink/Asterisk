/*
 * app_pi_gpio.c - based upon Asterisk -- An open source telephony toolkit.
 *
 * Controls the Raspberry PI GPIO using the pigpio library
 * https://github.com/joan2937/pigpio
 * 
 * Copyright (C) 2022, Stacy Olivas, KG7QIN
 * All rights reserved
 *
 * Licensed under the GNU GPL v3 (see below)
 * 
 *  This software is based upon and dependent upon the Asterisk - An open source telephone toolkit
 *  Copyright (C) 1999 - 2006, Digium, Inc.
 * 
 * See http://www.asterisk.org for more information about the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance; the project provides a web site, mailing lists and IRC
 * channels for your use.
 * 
 * -----------------------------------------------------------------------------------------------
 * 
 * License:
 * --------
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ------------------------------------------------------------------------
 * 
 */

/*! \file
 *
 * \brief Raspberry PI GPIO Interface module
 *
 * \author Stacy Olivas, KG7QIN
 * 
 * \ingroup applications
 */

/*** MODULEINFO
	<defaultenabled>no</defaultenabled>
	<depend>pigpio</depend>
 ***/

#include "asterisk.h"

/* Increment to match revision.  Format YYYYMMDD## */
ASTERISK_FILE_VERSION(__FILE__, "$Revision: 2022031201 $")

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/utils.h"
#include "asterisk/lock.h"

#include "asterisk/options.h"
#include "asterisk/app.h"
#include "asterisk/cli.h"

/* Module specific includes */
#include <pigpio.h>

static char *tdesc = "PIGPIO - Raspberry PI GPIO Control v0.01 - 03/12/2022";
static char *app = "PiGPIO";
static char *synopsis = "Raspberry PI GPIO Control";
static char *descrip = "This application allows you to control the Raspberry PI GPIO PINs.\n";

#define GPIOSTATUS "GPIOSTATUS"
#define MSG_GPIO_OP_ERROR "ERROR"
#define MSG_GPIO_OP_OK "OK"

/* convert string to integer */
inline static int myatoi(char *str)
{
	int	ret;

	if (str == NULL) return -1;
	/* leave this %i alone, non-base-10 input is useful here */
	if (sscanf(str,"%i",&ret) != 1) return -1;
	return ret;
}

/*! \brief sets the GPIOSTATUS channel variable */
static void set_gpio_result(struct ast_channel *chan, char *value)
{
	/* set helper variable to return result */
	pbx_builtin_setvar_helper(chan, GPIOSTATUS, value);
	return;
}

static int pigpio_do_version(int fd, int argc, char *argv[])
{
	ast_cli(fd, "%s\n", tdesc);
	return RESULT_SUCCESS;
}

static int pigpio_do_write(int fd, int argc, char *argv[])
{
	int pin, mode, status;

	if (argc < 4)
		return RESULT_SHOWUSAGE;

	pin = myatoi(argv[2]);
	if (pin < 0 || pin > 31)
	{
		ast_log(LOG_ERROR, "PIGPIO: %d is an invalid GPIO pin\n", pin);
		return RESULT_SHOWUSAGE;
	}
	mode = myatoi(argv[3]);
	if (mode == 0 || mode == 1)
	{
		status = gpioWrite(pin, mode);
		if (status == PI_BAD_GPIO || status == PI_BAD_LEVEL)
		{
			switch (status)
			{
				case PI_BAD_GPIO:
					ast_log(LOG_ERROR, "PIGPIO: PI BAD GPIO\n");
					break;
				case PI_BAD_LEVEL:
					ast_log(LOG_ERROR, "PIGPIO: PI BAD LEVEL\n");
					break;
				default:
					ast_log(LOG_ERROR, "PIGPIO: UNDEFINED ERROR\n");
					break;
			}
			return RESULT_FAILURE;
		}
		else
		{
			ast_cli(fd, "[*] PIGPIO: Write GPIO %d - mode %d \n", pin, mode);
			return RESULT_SUCCESS;
		}
	}
	ast_log(LOG_ERROR, "PIGPIO: Mode %d is not valid (must be 0 or 1)\n", mode);
	return RESULT_SHOWUSAGE;
}

static int pigpio_do_read(int fd, int argc, char *argv[])
{
	int pin, value;

	if (argc < 3)
		return RESULT_SHOWUSAGE;

	pin = myatoi(argv[2]);
	if (pin < 0 || pin > 31)
	{
		ast_log(LOG_ERROR, "PIGPIO: %d is an invalid GPIO pin\n", pin);
		return RESULT_SHOWUSAGE;
	}
	value = gpioRead(pin);
	if (value == PI_BAD_GPIO)
	{
		ast_log(LOG_ERROR, "PIGPIO: PI BAD GPIO\n");
		return RESULT_FAILURE;
	}
	ast_cli(fd, "[*] PTGPIO: Read GPIO %d - value %d \n", pin, value);
	return RESULT_SUCCESS;
}

static int pigpio_do_mode(int fd, int argc, char *argv[])
{
	int pin, value, mode, status;
	//char *cmode;

	if (argc < 3)
		return RESULT_SHOWUSAGE;

	pin = myatoi(argv[2]);
	if (pin < 0 || pin > 31)
	{
		ast_log(LOG_ERROR, "PIGPIO: %d is an invalid GPIO pin\n", pin);
		return RESULT_SHOWUSAGE;
	}
	
	mode = -1;
	if (!strcasecmp(argv[3], "INPUT"))
		mode = 0;
	else if (!strcasecmp(argv[3], "OUTPUT"))
		mode = 1;

	if (mode == 0 || mode == 1) {
		switch (mode)
		{
			case 0:
				status = gpioSetMode(pin, PI_INPUT);
				break;
			case 1:
				status = gpioSetMode(pin, PI_OUTPUT);
				break;
		}
		if (status == PI_BAD_GPIO || status == PI_BAD_MODE)
		{
			switch (status)
			{
				case PI_BAD_GPIO:
					ast_log(LOG_ERROR, "PIGPIO: PI BAD GPIO\n");
					break;
				case PI_BAD_MODE:
					ast_log(LOG_ERROR, "PIGPIO: PI BAD MODE\n");
					break;
				default:
					ast_log(LOG_ERROR, "PIGPIO: UNDEFINED ERROR\n");				
					break;
			}
			return RESULT_FAILURE;
		}
		else
		{
			ast_cli(fd, "[*] PIGPIO: Mode GPIO %d mode %s \n", pin, mode ? "OUTPUT":"INPUT");
			return RESULT_SUCCESS;
		}
	}
	ast_log(LOG_ERROR, "PIGPIO: %s is an invalid mode, must be INPUT or OUTPUT\n", argv[3]);
	return RESULT_SHOWUSAGE;
}

static int pigpio_exec(struct ast_channel *chan, void *data)
{
	char *command;
	char *gpiomode;
	int  pin, mode, value, status;
	char *s, cmode[10] = "";
	
	struct ast_module_user *u;

	ast_log(LOG_NOTICE, "PIGPIO Exec\n");
	if (ast_strlen_zero(data)) {
		ast_log(LOG_ERROR, "\n\n"
						   "PIGPIO requires arguments: write|<GPIO PIN>|<1:0>\n"
		                   "                           read|<GPIO PIN>\n"
						   "                           mode|<GPIO PIN>|<input|output>\n"
						   "\n");
		return -1;
	}

	u = ast_module_user_add(chan);

	s = ast_strdupa(data);

	/* get command */
	command = strsep(&s, "|");
	if (!command) {
		ast_log(LOG_ERROR, "PIGPIO: Command must be given\n");
		ast_module_user_remove(u);
		return -1;
	}

	/* get GPIO PIN */
	if (s) {
		pin = myatoi(strsep(&s, "|"));
		if (!pin){
			ast_log(LOG_ERROR, "PIGPIO: GPIO PIN is required\n");
			ast_module_user_remove(u);
			return -1;
		}
		if (pin < 0 || pin > 31) {
			ast_log(LOG_ERROR, "PIGPIO: GPIO PIN must be between 0 and 31\n");
			ast_module_user_remove(u);
			return -1;
		}
	}

	if (strcasecmp(command, "WRITE") == 0)
	{
		/* write GPIO PIN */
		mode = myatoi(strsep(&s, "|"));
		if(!mode) {
			ast_log(LOG_ERROR, "PIGPIO: A 0 (off/low) or 1 (on/high) value must be given\n");
			ast_module_user_remove(u);
			return -1;
		}
		if (mode == 0 || mode == 1) {
			status = gpioWrite(pin, mode);
			if (status == PI_BAD_GPIO || status == PI_BAD_LEVEL)
			{
				switch (status)
				{
					case PI_BAD_GPIO:
						ast_log(LOG_ERROR, "PIGPIO: PI BAD GPIO\n");
						break;
					case PI_BAD_LEVEL:
						ast_log(LOG_ERROR, "PIGPIO: PI BAD LEVEL\n");
						break;
					default:
						ast_log(LOG_ERROR, "PIGPIO: UNDEFINED ERROR\n");
						break;
				}
				set_gpio_result(chan, MSG_GPIO_OP_ERROR);				
				ast_module_user_remove(u);
				return -1;
			}
			else
			{
				ast_log(LOG_NOTICE, "PIGPIO: Write GPIO %d mode %d\n", pin, mode);
				set_gpio_result(chan, MSG_GPIO_OP_OK);
				ast_module_user_remove(u);
				return 0;
			}
		}
		else
		{
			ast_log(LOG_ERROR, "PIGPIO: A 0 (off/low) or 1 (on/high) value must be given\n");
			ast_module_user_remove(u);
			return -1;
		}
	}

	if (strcasecmp(command, "READ") == 0)
	{
		/* read GPIO PIN */
		value = gpioRead(pin);
		if (value == PI_BAD_GPIO)
		{
			ast_log(LOG_ERROR, "PIGPIO: PI BAD GPIO\n");
			set_gpio_result(chan, MSG_GPIO_OP_ERROR);
			ast_module_user_remove(u);
			return -1;
		}

		/* save value in GPIOSTATUS */
		sprintf(cmode, "%d", value);		
		set_gpio_result(chan, cmode);
		ast_log(LOG_NOTICE, "PTGPIO: Read GPIO %d - value %d\n", pin, value);
		ast_module_user_remove(u);
		return 0;
	}

	if (strcasecmp(command, "MODE") == 0)
	{
		/* set GPIO PIN mode */
		gpiomode = strsep(&s, "|");
		if(!gpiomode) {
			ast_log(LOG_ERROR, "PIGPIO: A mode of INPUT or OUTPUT must be given\n");
			ast_module_user_remove(u);
			return -1;
		}

		mode = -1;
		if(!strcasecmp(gpiomode, "INPUT"))
			mode = 0;
		else if (!strcasecmp(gpiomode, "OUTPUT"))
			mode = 1;

		if (mode == 0 || mode == 1) {
			switch (mode)
			{
				case 0:
					status = gpioSetMode(pin, PI_INPUT);
					break;
				case 1:
					status = gpioSetMode(pin, PI_OUTPUT);
					break;
			}
			if (status == PI_BAD_GPIO || status == PI_BAD_MODE)
			{
				switch (status)
				{
					case PI_BAD_GPIO:
						ast_log(LOG_ERROR, "PIGPIO: PI BAD GPIO\n");
						break;
					case PI_BAD_MODE:
						ast_log(LOG_ERROR, "PIGPIO: PI BAD MODE\n");
						break;
					default:
						ast_log(LOG_ERROR, "PIGPIO: UNDEFINED ERROR\n");
						break;
				}
				set_gpio_result(chan, MSG_GPIO_OP_ERROR);
				ast_module_user_remove(u);
				return -1;
			}
			else
			{
				ast_log(LOG_NOTICE, "PIGPIO: Mode GPIO %d mode %s\n", pin, mode ? "OUTPUT":"INPUT");
				set_gpio_result(chan, MSG_GPIO_OP_OK);
				ast_module_user_remove(u);
				return 0;
			}
		}
		else
		{
			ast_log(LOG_ERROR, "PIGPIO: %s is an invalid mode, must be INPUT or OUTPUT\n", gpiomode);
			ast_module_user_remove(u);
			return -1;
		}
	}

	ast_log(LOG_WARNING, "PIGPIO: Invalid command.\n");
	ast_module_user_remove(u);

	return -1;
}

static char pigpio_version_usage[] =
"Usage: pigpio version\n"
"       Displays pgpio version information.\n\n"
"       Examples:\n"
"         - pigpio version\n"
"\n";

static char pigpio_write_usage[] =
"Usage: pigpio write <GPIO PIN> [0|1]\n"
"       Write 0 (off/low) or 1 (on/high) to <GPIO PIN> of Raspberry PI.\n\n"
"       Examples:\n"
"         Command line:\n"
"         - pigpio write 2 0 - Write 0 (off/low) to GPIO PIN 2\n"
"         - pigpio write 2 1 - Write 1 (on/high) to GPIO PIN 2\n"
"\n"
"         Dialplan:\n"
"         - exten => pigpio(write|2|0) - Write 0 (off/low) to GPIO PIN 2\n"
"         - exten => ptgpio(write|2|1) - Write 1 (on/high) to GPIO PIN 2\n"
"\n";

static char pigpio_read_usage[] =
"Usage: pigpio write <GPIO PIN>\n"
"       Reads GPIO and returns 0 (off/low) or 1 (on/high) to <GPIO PIN> of Raspberry PI.\n\n"
"       Examples:\n"
"        Command line:\n"
"         - pigpio read 2 - Reads GPIO PIN 2 status\n"
"         - pigpio read 5 - Reads GPIO PIN 5 status\n"
"\n"
"        Dialplan:\n"
"         - exten => pigpio(read|2) - Read status of GPIO PIN 2\n"
"\n"
"           Note: Variable GPIOSTATUS holds the value of the most recent pigpio read call.\n"
"                 0 = off/low\n"
"                 1 = on/high\n"
"\n";

static char pigpio_mode_usage[] =
"Usage: pigpio mode <GPIO PIN> <input|output>\n"
"       Sets GPIO pin mode to INPUT or OUTPUT.\n\n"
"       Examples:\n"
"        Command line:\n"
"         - pigpio mode 2 input  - Sets GPIO PIN 2 to input mode\n"
"         - pigpio mode 2 output - Sets GPIO PIn 2 to output mode\n"
"\n"
"        Dialplan:\n"
"         - exten => pigpio(mode|2|input)  - Sets GPIO PIN 2 to input mode\n"
"         - exten => pigpio(mode|2|output) - Sets GPIO PIN 2 to output mode\n"
"\n";

/* Allows for commands to be passed via the CLI */
static struct ast_cli_entry cli_pigpio[] = {
	{ { "pigpio", "version", NULL }, 
	pigpio_do_version, "Raspberry PI GPIO Utility - Show version information", 
	pigpio_version_usage, NULL, NULL },

	{ { "pigpio", "write", NULL }, 
	pigpio_do_write, "Raspberry PI GPIO Utility - Write GPIO PIN", 
	pigpio_write_usage, NULL, NULL },

	{ { "pigpio", "read", NULL }, 
	pigpio_do_read, "Raspberry PI GPIO Utility - Read GPIO PIN", 
	pigpio_read_usage, NULL, NULL },

	{ { "pigpio", "mode", NULL }, 
	pigpio_do_mode, "Raspberry PI GPIO Utility - Set GPIO PIN Mode (input/output)", 
	pigpio_mode_usage, NULL, NULL },
};

static int reload(void)
{
	return 0;
}

static int unload_module(void)
{
	int res;
	/* Unregister cli extensions */
	ast_cli_unregister_multiple(cli_pigpio, sizeof(cli_pigpio) / sizeof(struct ast_cli_entry));
	res = ast_unregister_application(app);

	/* stop pigpio library routines */
	gpioTerminate();

	return res;	
}

static int load_module(void)
{
	/* Configure pigpio library to turn off internal signal handling */
	int gpio_status, gpio_cfg = 0;
	gpio_cfg = gpioCfgGetInternals()
	gpio_cfg |= PI_CFG_NOSIGHANDLER;
	gpioCfgSetInternals(gpio_cfg);

	/* iniitalize pigpio library */
	gpio_status = gpioInitialise();
	if (gpio_status < 0)
	{
		ast_log(LOG_ERROR, "PIGPIO: Library initialization failed.\n");
		return -1;
	}

	/* Register cli extensions */
	ast_cli_register_multiple(cli_pigpio, sizeof(cli_pigpio) / sizeof(struct ast_cli_entry));
	return ast_register_application(app, pigpio_exec, synopsis, descrip);
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "Raspberry PI GPIO Module",
		.load = load_module,
		.unload = unload_module,
		.reload = reload,
	       );