#include "socket.h"
#include "netdb.h"

#include "M4io.h"

//char rxbuffer[2098];
//char txbuffer[2098];


///////////////////////////////////////////////////
//M4 ASM externs 
extern void M4selectROM ();
extern void M4storeROM ();
extern void M4restoreROM ();
extern void M4findROM ();

extern int M4getsocket ();
extern int M4connect (int sockfd);
extern int M4send (int sockfd);
extern int M4recv (int sockfd, int len);
extern int M4close ();

extern char cmdsocket[];
extern char cmdconnect[];
extern char cmdsend[];
extern char cmdclose[];
extern char cmdrecv[];

extern int clsocket;



extern char m4_rom_name[8];
extern unsigned char m4_rom_num[1];
extern char buf[255];
///////////////////////////////////////////////////




int socket(int domain, int type, int protocol)
{
	int result=0;
	M4storeROM();
	M4selectROM();
	//Build msg string
	cmdsocket[3]=domain; //Domain
	cmdsocket[4]=type; //Type
	cmdsocket[5]=protocol; //Protocol

	result=M4getsocket();

	M4restoreROM();
	return result;
}

int  sockclose(int fd)
{
	int result=0;
	clsocket=fd;

	result = M4close();

	return result;
}

int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
return 0;
}

int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
	int result=0;
	unsigned char inx;

	M4storeROM();
	M4selectROM();

	//Build msg string
	cmdconnect[3]=sockfd; // csocket
	for (inx=0;inx<6;inx++)
		{
		cmdconnect[4+inx]=(unsigned char) serv_addr[inx]; // ip_addr & port
		}
	result=M4connect(sockfd);

	M4restoreROM();
	return result;
	//return;

/*
_cmdconnect:	.db	9
		.dw	C_NETCONNECT
_csocket:	.db	#0
_ip_addr:	.db	#0,#0,#0,#0		; ip addr
		.dw	#0x1234		; port
*/

}

int send(int sockfd, void *buf, int len, int flags)
{
return;
}

int recv(int sockfd, void *buf, int len, int flags)
{
	M4storeROM();
	M4selectROM();
	M4recv(sockfd,len);
	M4restoreROM();


/*

*/





return;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
return;
}

int listen(int sockfd, int backlog)
{
return;
}

/*
void ASM_defines(void)
{
__asm

DATAPORT	equ &FE00
ACKPORT		equ &FC00
C_NETSOCKET	equ &4331
C_NETCONNECT	equ &4332
C_NETCLOSE	equ &4333
C_NETSEND	equ &4334
C_NETRECV	equ &4335
C_NETHOSTIP	equ &4336

__endasm;
}
*/



void ASM_code(void)
{
__asm


			;ROM paging manipulation
			;
			;Search M4, store CPC paging state, restore state, etc


_M4selectROM::
M4selectROM:
	ld	a,#_m4_rom_num
	cp	#0xFF
	jr	nz, M4findROM ; Dont have yet M4 ROM
	call 	#0xB90C
	ret

_M4storeROM::
M4storeROM:
	call 	#0xB912
	ld   	(#_rom_state),a
	ret

_M4restoreROM::
M4restoreROM:
	ld	a,#_rom_state
	call	#0xB90C
	ret



			;
			; Find M4 ROM location
			;

_M4findROM::
M4findROM:
			ld	iy,#_m4_rom_name	; rom identification line
			ld	d,#127		; start looking for from (counting downwards)

romloop:		push	de
					;ld	bc,#0xDF00
					;out	(c),d		; select rom
			ld	c,d
			call	#0xB90F		; system/interrupt friendly
			ld	a,(#0xC000)
			cp	#1
			jr	nz, not_this_rom

			; get rsxcommand_table

			ld	a,(#0xC004)
			ld	l,a
			ld	a,(#0xC005)
			ld	h,a
			push	iy
			pop	de
cmp_loop:
			ld	a,(de)
			xor	(hl)			; hl points at rom name
			jr	nz, not_this_rom
			ld	a,(de)
			inc	hl
			inc	de
			and	#0x80
			jr	z,cmp_loop

			; rom found, store the rom number

			pop	de			;  rom number
			ld 	a,d
			ld	(#_m4_rom_num),a
			ret

not_this_rom:
			pop	de
			dec	d
			jr	nz,romloop
			ld	a,#255		; not found!
			ret



M4getstatus:

;in:	a = socket
;out	ix = current socket status
;	iy = response buffer
			; multiply by 16 and add to socket status buffer
			
			sla	a
			sla	a
			sla	a
			sla	a
			
			ld	hl,#R_SOCKINFO	; get sock info
			ld	e,(hl)
			inc	hl
			ld	d,(hl)
			ld	l,a
			ld	h,#0
			add	hl,de	; sockinfo + (socket*4)
			push	hl
			pop	ix		; ix ptr to current socket status

			ld	hl,#R_RESP	; get response buffer address
			ld	e,(hl)
			inc	hl
			ld	d,(hl)
			push	de
			pop	iy

			ret


_M4getsocket::
M4getsocket:
			; get a socket

			ld	a,#0

			call M4getstatus
			
			ld	hl,#_cmdsocket
			call	sendcmd
			ld	a,3(iy)  
			cp	#255
			;ret	z   ;DANGER
			jr	nz,getsocket_ok			
			ld	a,#255

			; return socket			

getsocket_ok:
			ld	h,#0
			ld	l,a
			ret




			;
			; connect to server
			;

_M4connect::
M4connect:
			ld hl,#2
			add	hl,sp
			ld	a,(hl)

			call M4getstatus

			ld	hl,#_cmdconnect
			call	sendcmd
			ld	a,3(iy)
			cp	#255
			jp	z,M4close
wait_connect:
			ld	a,(ix)			; get socket status  (0 ==IDLE (OK), 1 == connect in progress, 2 == send in progress)
			cp	#1				; connect in progress?
			jr	z,wait_connect
			cp	#0
			jr	z,connect_ok
			;NEED RETURN ERROR AND EVAL IT
			;call	disp_error	
			jp	M4close
			ld	h,#0
			ld	l,#0xFF
			ret
connect_ok:
			ld	h,#0
			ld	l,a
			ret

			;
			; Close M4 socket
			;
_M4close::
M4close:
			ld	hl,#_cmdclose
			call	sendcmd
			ret

			;
			; Send command to M4
			;
_M4send::
M4send:
			ld	hl,#_cmdsend
			call	sendcmd
			ret

sendcmd:

			ld	bc,#0xFE00
			ld	d,(hl)
			inc	d
sendloop:		inc	b
			outi
			dec	d
			jr	nz,sendloop
			ld	bc,#0xFC00
			out	(c),c
			ret




_M4recv::
M4recv:

			; recv tcp data
			; in
			; bc = receive size
			; out
			; a = receive status
			; bc = received size 

			
recv:		; connection still active

			ld hl,#2
			add	hl,sp
			ld	a,(hl)
			ld hl,#4
			add	hl,sp
			ld	b,(hl)
			inc	hl
			ld	c,(hl)

			call M4getstatus

			ld	a,(ix)			; 
			cp	#3				; socket status  (3 == remote closed connection)
			ret	z
			; check if anything in buffer ?
			ld	a,2(ix)
			cp	#0
			jr	nz,recv_cont
			ld	a,3(ix)
			cp	#0
			jr	nz,recv_cont
			ld	bc,#0
			ld	a,#1	
			ret
recv_cont:			
			; set receive size
			ld	a,c
			ld	(#_rsize),a
			ld	a,b
			ld	(#_rsize+1),a
			
			ld	hl,#_cmdrecv
			call	sendcmd
			
			ld	bc,#0
			ld	a,3(iy)
			cp	#0				; all good ?
			jr	z,recv_ok
			push	af
			;call	disp_error  ;WARNING
			pop	af
			ld	bc,#0
			ret

recv_ok:			
			ld	c,4(iy)
			ld	b,5(iy)
			ret


__endasm;
}

void ASM_data(void)
{

__asm
_cmdsocket:	.db	5
		.dw	C_NETSOCKET
		.db	#0x0,#0x0,#0x6		; domain, type, protocol (TCP/ip)

_cmdconnect:	.db	9
		.dw	C_NETCONNECT
_csocket:	.db	#0
_ip_addr:	.db	#0,#0,#0,#0		; ip addr
		.dw	#0x1234		; port
_cmdsend:	.db	#0			; we can ignore this byte (part of early design)	
		.dw	C_NETSEND
_sendsock:	.db	#0
_sendsize:	.dw	#0			; size
_sendtext:	.ds	#255

_cmdclose:	.db	#3
		.dw	C_NETCLOSE
_clsocket:	.db	#0

_cmdrecv:	.db	#5
		.dw	C_NETRECV		; recv
_rsocket:	.db	#0			; socket
_rsize:		.dw	#2048			; size
_rtext:		.ds	#255

_m4_rom_name:	.str 	"M4 BOAR"
		.db	#0xC4		; D | &80
_m4_rom_num:	.db	#0xFF
_buf:		.ds	#255
_rom_state:	.db	#0xFF

__endasm;
}





//extern struct hostent *gethostbyname(char *name)
struct hostent *gethostbyname(char *name)
{

return 0;  //return ERROR
}

/*
			; TCP Echo client example for M4 Board
			; Requires firmware v1.0.9b7 upwards
			; Duke 2016
			
			org	&9000
			nolist
DATAPORT		equ &FE00
ACKPORT		equ &FC00			
C_NETSOCKET	equ &4331
C_NETCONNECT	equ &4332
C_NETCLOSE	equ &4333
C_NETSEND		equ &4334
C_NETRECV		equ &4335
C_NETHOSTIP	equ &4336
			
start:		ld	a,2			
			call	&bc0e		; set mode 2
get_server_ip:	ld	hl,msgserverip
			call	disptextz
			ld	hl,buf
			call	get_textinput
			
			cp	&FC			; ESC?
			ret	z
			xor	a
			cp	c
			jr	z, get_server_ip
		
			; convert ascii IP to binary, no checking for non decimal chars format must be x.x.x.x
			
			ld	hl,buf	
			call	ascii2dec
			ld	(ip_addr+3),a
			call	ascii2dec
			ld	(ip_addr+2),a
			call	ascii2dec
			ld	(ip_addr+1),a
			call	ascii2dec
			ld	(ip_addr),a
			

			push	iy
			push	ix

			ld	a,(m4_rom_num)
			cp	&FF
			call	z,find_m4_rom	; find rom (only first run)
							; should add version check too and make sure its v1.0.9
			cp	&FF
			call	nz,tcpclient
			
			pop	ix
			pop	iy
			ret
tcpclient:	ld	hl,&FF02	; get response buffer address
			ld	e,(hl)
			inc	hl
			ld	d,(hl)
			push	de
			pop	iy
			
			; get a socket
			
			ld	hl,cmdsocket
			call	sendcmd
			ld	a,(iy+3)
			cp	255
			ret	z
			
			; store socket in predefined packets
			
			ld	(csocket),a
			ld	(clsocket),a
			ld	(rsocket),a
			ld	(sendsock),a
			
			
	
			; multiply by 16 and add to socket status buffer
			
			sla	a
			sla	a
			sla	a
			sla	a
			
			ld	hl,&FF06	; get sock info
			ld	e,(hl)
			inc	hl
			ld	d,(hl)
			ld	l,a
			ld	h,0
			add	hl,de	; sockinfo + (socket*4)
			push	hl
			pop	ix		; ix ptr to current socket status
			
			; connect to server
			
			ld	hl,cmdconnect
			call	sendcmd
			ld	a,(iy+3)
			cp	255
			jp	z,exit_close
wait_connect:
			ld	a,(ix)			; get socket status  (0 ==IDLE (OK), 1 == connect in progress, 2 == send in progress)
			cp	1				; connect in progress?
			jr	z,wait_connect
			cp	0
			jr	z,connect_ok
			call	disp_error	
			jp	exit_close
connect_ok:	ld	hl,msgconnect
			call	disptextz

mainloop:		ld	hl,sendtext
			call	get_textinput			
			cp	&FC			; ESC?
			jp	z, exit_close	
			xor	a
			cp	c
			jr	z, mainloop	; nothing in buffer!
			ld	(sendsize),bc
			call	crlf
wait_send:	ld	a,(ix)
			cp	2			; send in progress?
			jr	z,wait_send	; Could do other stuff here!
			cp	0
			call	nz,disp_error	
			
			ld	hl,5
			add	hl,bc
			ex	de,hl
			ld	hl,cmdsend
			ld	(hl),e
			call	sendcmd

						
			; reset size
			ld	hl,0
			ld	(sendsize),hl

wait_recv:			
			ld	bc,255	
			call	recv
			cp	&FF
			jr	z, exit_close	
			cp	3
			jr	z, exit_close
			xor	a
			cp	c
			jr	nz, got_msg
			cp	b
			jr	z,wait_recv	; keep looping, want a reply. Could do other stuff here!
got_msg:	
					
			; disp received msg
			ld	a,&3E
			call	&bb5a
			push	iy
			pop	hl
		
			ld	de,&6
			add	hl,de		; received text pointer
			call	disptext
			call	crlf
			jp	mainloop
			


exit_close:
			ld	hl,cmdclose
			call	sendcmd			
			ret
			
			; recv tcp data
			; in
			; bc = receive size
			; out
			; a = receive status
			; bc = received size 

			
recv:		; connection still active
			ld	a,(ix)			; 
			cp	3				; socket status  (3 == remote closed connection)
			ret	z
			; check if anything in buffer ?
			ld	a,(ix+2)
			cp	0
			jr	nz,recv_cont
			ld	a,(ix+3)
			cp	0
			jr	nz,recv_cont
			ld	bc,0
			ld	a,1	
			ret
recv_cont:			
			; set receive size
			ld	a,c
			ld	(rsize),a
			ld	a,b
			ld	(rsize+1),a
			
			ld	hl,cmdrecv
			call	sendcmd
			
			ld	bc,0
			ld	a,(iy+3)
			cp	0				; all good ?
			jr	z,recv_ok
			push	af
			call	disp_error
			pop	af
			ld	bc,0
			ret

recv_ok:			
			ld	c,(iy+4)
			ld	b,(iy+5)
			ret
			
			
			;
			; Find M4 ROM location
			;
				
find_m4_rom:
			ld	iy,m4_rom_name	; rom identification line
			ld	d,127		; start looking for from (counting downwards)
			
romloop:		push	de
			;ld	bc,&DF00
			;out	(c),d		; select rom
			ld	c,d
			call	&B90F		; system/interrupt friendly
			ld	a,(&C000)
			cp	1
			jr	nz, not_this_rom
			
			; get rsxcommand_table
			
			ld	a,(&C004)
			ld	l,a
			ld	a,(&C005)
			ld	h,a
			push	iy
			pop	de
cmp_loop:
			ld	a,(de)
			xor	(hl)			; hl points at rom name
			jr	nz, not_this_rom
			ld	a,(de)
			inc	hl
			inc	de
			and	&80
			jr	z,cmp_loop
			
			; rom found, store the rom number
			
			pop	de			;  rom number
			ld 	a,d
			ld	(m4_rom_num),a
			ret
			
not_this_rom:
			pop	de
			dec	d
			jr	nz,romloop
			ld	a,255		; not found!
			ret
			
			;
			; Send command to M4
			;
sendcmd:
			ld	bc,&FE00
			ld	d,(hl)
			inc	d
sendloop:		inc	b
			outi
			dec	d
			jr	nz,sendloop
			ld	bc,&FC00
			out	(c),c
			ret
					
			; display text
			; HL = text
			; BC = length

disptext:		xor	a
			cp	c
			jr	nz, not_dispend
			cp	b
			ret	z
not_dispend:
			ld 	a,(hl)
			push	bc
			call	&BB5A
			pop	bc
			inc	hl
			dec	bc
			jr	disptext

			; display text zero terminated
			; HL = text
disptextz:	ld 	a,(hl)
			or	a
			ret	z
			call	&BB5A
			inc	hl
			jr	disptextz

			;
			; Display error code in ascii (hex)
			;
	
			; a = error code
disp_error:
			push	af
			ld	hl,msgsenderror
			ld	bc,9
			call	disptext
			pop	bc
			ld	a,b
			srl	a
			srl	a
			srl	a
			srl	a
			add	a,&90
			daa
			adc	a,&40
			daa
			call	&bb5a
			ld	a,b
			and	&0f
			add	a,&90
			daa
			adc	a,&40
			daa
			call	&bb5a
			ld	a,10
			call	&bb5a
			ld	a,13
			call	&bb5a
			ret
			
			;
			; Get input text line.
			;
			; in
			; hl = dest buf
			; return
			; bc = out size
get_textinput:		
			ld	bc,0
			call	&bb81	
inputloop:
re:			call	&bd19
			call	&bb09
			jr	nc,re

			cp	&7F
			jr	nz, not_delkey
			ld	a,c
			cp	0
			jr	z, inputloop
			push	hl
			push	bc
			call	&bb78
			dec	h
			push	hl
			call	&bb75
			ld	a,32
			call	&bb5a
			pop	hl
			call	&bb75
			pop	bc
			pop	hl
			dec	hl
			dec	bc
			jr	inputloop
not_delkey:	
			cp	13
			jr	z, enterkey
			cp	&FC
			ret	z
			cp	32
			jr	c, inputloop
			cp	&7e
			jr	nc, inputloop
			ld	(hl),a
			inc	hl
			inc	bc
			push	hl
			push	bc
			call	&bb5a
			call	&bb78
			push	hl
			ld	a,32
			call	&bb5a
			pop	hl
			call	&bb75
			pop	bc
			pop	hl
			jp	inputloop
enterkey:		ld	(hl),0
			ret
crlf:		ld	a,10
			call	&bb5a
			ld	a,13
			jp	&bb5a

ascii2dec:	ld	d,0
loop2e:		ld	a,(hl)
			cp	0
			jr	z,found2e
			cp	&2e
			jr	z,found2e
			; convert to decimal
			cp	&41	; a ?
			jr	nc,less_than_a
			sub	&30	; - '0'
			jr	next_dec
less_than_a:	sub	&37	; - ('A'-10)
next_dec:		ld	(hl),a
			inc	hl
			inc	d
			dec	bc
			xor	a
			cp	c
			ret	z
			jr	loop2e
found2e:
			push	hl
			call	dec2bin
			pop	hl
			inc	hl
			ret
dec2bin:		dec	hl
			ld	a,(hl)
			dec	hl
			dec	d
			ret	z
			ld	b,(hl)
			inc	b
			dec	b
			jr	z,skipmul10
mul10:		add	10
			djnz	mul10
skipmul10:	dec	d
			ret	z
			dec	hl
			ld	b,(hl)
			inc	b
			dec	b
			ret	z
mul100:		add	100
			djnz	mul100
			ret
			
msgconnclosed:	db	10,13,"Remote closed connection....",10,13,0
msgsenderror:	db	10,13,"ERROR: ",0
msgconnect:	db	10,13,"Connected!",10,13,"Type text to server",10,13,0
msgserverip:	db	10,13,"Input TCP server IP:",10,13,0

cmdsocket:	db	5
			dw	C_NETSOCKET
			db	&0,&0,&6		; domain, type, protocol (TCP/ip)

cmdconnect:	db	9	
			dw	C_NETCONNECT
csocket:		db	&0
ip_addr:		db	0,0,0,0		; ip addr
			dw	&1234		; port
cmdsend:		db	0			; we can ignore this byte (part of early design)	
			dw	C_NETSEND
sendsock:		db	0
sendsize:		dw	0			; size
sendtext:		ds	255
			
cmdclose:		db	&03
			dw	C_NETCLOSE
clsocket:		db	&0

cmdrecv:		db	5
			dw	C_NETRECV		; recv
rsocket:		db	&0			; socket
rsize:		dw	2048			; size
			
m4_rom_name:	db "M4 BOAR",&C4		; D | &80
m4_rom_num:	db	&FF
buf:			ds	255	



*/

/*

M4findrom
M4connect
M4send
*/

