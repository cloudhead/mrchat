/***************************************************************************
 *   Copyright (C) 2004 by Desty and Kapone                                *
 *   root@kapone                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "common.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define PORT 1337

enum 
{
	TCP_NORMAL_PEER,
	TCP_NORMAL_PORT,
	TCP_NORMAL_END
};
 
struct TCP_normal_attr
{
	int sock;
	struct sockaddr_in local;
	struct sockaddr_in peer;
};
