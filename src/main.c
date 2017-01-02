//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>

/* A simple TCP server - this goes along with the following:
 *  http://spectrum.alioth.net/doc/index.php/Spectranet:_Tutorial_3 */

#include <stdio.h>
#include <string.h>
#include "M4libsocks/socket.h"		/* socket, connect, send, recv etc. */
//#include <sys/types.h>		/* types, such as socklen_t */
#include "M4libsocks/netdb.h"		/* gethostbyname */

void Do_NET()
{
	int sockfd, bytes;
	struct sockaddr_in remoteaddr;
	char *remotehost="www.google.com";
	char *txdata="GET / HTTP/1.0\r\n\r\n";
	char rxdata[1024];
	struct hostent *he;

	/* Look up the host that we want to connect to. */
	/* Note that hostent pointer is statically allocated, don't try
	   to free it! */
	printf("Looking up %s...\r\n",remotehost);
	he=gethostbyname(remotehost);
	if(!he)
	{
		printf("Failed to look up remote host\r\n");
		return;
	}
	

        /* Create the socket */
        /* The first argument, AF_INET is the family. The Spectranet only
           supports AF_INET at present. SOCK_STREAM in this context is
           for a TCP connection. */
	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("Couldn't open the socket - rc=%d\r\n",sockfd);
		return;
	}

	/* Set up the sockaddr_in structure. Note that once we've
	   copied the address, we don't have to worry about gethostbyname
           overwriting the static struct hostent that it keeps */
	printf("Connecting...\r\n");
	remoteaddr.sin_port=htons(80);		/* port 80 - http */
	remoteaddr.sin_addr.s_addr=he->h_addr;	/* ip address */
	if(connect(sockfd, &remoteaddr, sizeof(remoteaddr)) < 0)
	{
		sockclose(sockfd);
		printf("Connect failed!\r\n");
		return;
	}

	/* Send 'GET / HTTP/1.0\r\n\r\n' to the remote host */
	bytes=send(sockfd, txdata, strlen(txdata), 0);
	if(bytes < 0)
	{
		printf("Send failed\r\n");
		sockclose(sockfd);
		return;
	}

	/* Get the response - use 1 byte less than the buffer so
           we can guarantee to be able to null terminate it for printing */
	bytes=recv(sockfd, rxdata, sizeof(rxdata)-1, 0);
	if(bytes < 0)
	{
		printf("recv failed\r\n");
		sockclose(sockfd);
		return;
	}

	/* make sure the data has a NULL on the end, so we can print it */
	*(rxdata+bytes)=0;
	printf("Got %d bytes:\r\n%s\r\n", bytes, rxdata);

	/* Close the socket - we're done */
	sockclose(sockfd);
}





void main(void) {

   u8* pvmem;  // Pointer to video memory

   cpct_setVideoMode(1);

   // Clear Screen
   cpct_memset(CPCT_VMEM_START, 0, 0x4000);


  Do_NET();

   // Loop forever

   while (1);
}



