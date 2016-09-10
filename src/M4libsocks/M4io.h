//IO Ports

#define DATAPORT	0xFE00
#define ACKPORT		0xFC00

//M4 commands
#define C_NETSOCKET	0x4331
#define C_NETCONNECT	0x4332
#define C_NETCLOSE	0x4333
#define C_NETSEND	0x4334
#define C_NETRECV	0x4335
#define C_NETHOSTIP	0x4336



extern void M4_IOwrite();

extern void M4_IOread();
