/*
 * main.c
 *
 * Copyright 2017 Naveen Karuthedath <naveen.karuthedath@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void getDate(char *date)
{
	time_t lt;
	struct tm * tmTmp;

	time(&lt);
	tmTmp = gmtime(&lt);
	if(date)
		strftime(date,40,"%a, %d %b %Y %X GMT",tmTmp);
}

void main()
{
	char *date = (char*)malloc(40);

	getDate(date);
	printf("Date: %s\n",date);

	if(date)
		free(date);
}
