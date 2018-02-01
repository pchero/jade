/*
 * user_handler.c
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */


#include "slog.h"
#include "common.h"

#include "user_handler.h"

bool init_user_handler(void)
{
	slog(LOG_INFO, "Fired init_user_handler.");

	return true;
}

void term_user_handler(void)
{
	slog(LOG_INFO, "Fired term_user_handler.");
	return;
}

bool reload_user_handler(void)
{
	slog(LOG_INFO, "Fired reload_user");

	term_user_handler();
	init_user_handler();

	return true;
}
