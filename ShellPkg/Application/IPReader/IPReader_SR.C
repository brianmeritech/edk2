//*****************************************************************************
//	IDReader_SR
//	Meritech SpahireRapid DDR5 Board ID Reader
//	MCU : STM32F091VBT
//	EEPROM : AT24C04
//*****************************************************************************
#define EFI

#include <stdio.h>
#ifdef EFI
#include <Library/IoLib.h>
// #include <Library/UefiRuntimeServicesTableLib.h>	// *gRT
#include <Library/UefiBootServicesTableLib.h>		// *gST, gBS
#include <string.h>
#include <stdlib.h>
#else
#include <dos.h>
#include <conio.h>
#endif

//-----------------------------------------------------------------------------

//--- TBVR COMMON equations ---------------------------------------------------
// #define	DEBUG_MSG			// Debug message
//#define	OPTION_MSG			// Option message
//#define	USE_INTISR			// Use interrupt ISR when receive data

#define COM_PORT_BMC_2F8

#ifndef EFI
#define	FALSE						0
#define	TRUE						1
#endif	// DOS

/*******************************************************************************
 *	Date		Version		Comment
 *				V0.1.0		Initial Version
 *	21/02/22	V0.2.0		Add Set IP Command
 *	21/03/31	V0.3.0		Change BUS Number
 *	21/04/01	V0.3.1		Find AX9100 Bus Number
 *	21/04/23	V0.3.2		Fix Help Message
 *	21/04/26	V0.3.3		Text File Format change - Request from SEC
 *	21/08/18	V0.4.0		Change COMM PORT AX99100 -> BMC 2F8
 *	21-08-25	V0.5.1		Add BMC 2F8 port Init routine
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						5
#define VERSION_MINOR2						1
#define IPREADER_VERSION					((VERSION_MAJOR * 100) + (VERSION_MINOR * 10) + VERSION_MINOR2)

// #define COMM_PORT_AX99100

#define MAX_TRY_CNT							5

#ifdef COMM_PORT_AX99100
	#define PCIE_REG_ADDR(Bus, Device, Function, Offset)	(((Offset) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))

	#define Ax9100_B_N  					3
	#define Ax9100_D_N  					0
	#define Ax9100_F_N  					1

	#define PCI_VENDOR_ID_OFFSET          	0x00
	#define PCI_DEVICE_ID_OFFSET          	0x02
	#define PCI_COMMAND_OFFSET            	0x04
	#define PCI_PRIMARY_STATUS_OFFSET     	0x06
	#define PCI_REVISION_ID_OFFSET        	0x08
	#define PCI_CLASSCODE_OFFSET          	0x09
	#define PCI_CACHELINE_SIZE_OFFSET     	0x0C
	#define PCI_LATENCY_TIMER_OFFSET      	0x0D
	#define PCI_HEADER_TYPE_OFFSET        	0x0E
	#define PCI_BIST_OFFSET               	0x0F
	#define PCI_BASE_ADDRESSREG_OFFSET    	0x10
	#define PCI_CARDBUS_CIS_OFFSET        	0x28
	#define PCI_SVID_OFFSET               	0x2C ///< SubSystem Vendor id
	#define PCI_SUBSYSTEM_VENDOR_ID_OFFSET	0x2C
	#define PCI_SID_OFFSET                	0x2E ///< SubSystem ID
	#define PCI_SUBSYSTEM_ID_OFFSET       	0x2E
	#define PCI_EXPANSION_ROM_BASE        	0x30
	#define PCI_CAPBILITY_POINTER_OFFSET  	0x34
	#define PCI_INT_LINE_OFFSET           	0x3C ///< Interrupt Line Register
	#define PCI_INT_PIN_OFFSET            	0x3D ///< Interrupt Pin Register
	#define PCI_MAXGNT_OFFSET             	0x3E ///< Max Grant Register
	#define PCI_MAXLAT_OFFSET             	0x3F ///< Max Latency Register
#else
	//--- Define Serial(UART) Ports Base Address
	#define PORT1							0x3F8
	#define PORT2							0x2F8
	#define PORT3							0x3E8
	#define PORT4							0x2E8
#endif

#ifdef COM_PORT_BMC_2F8
	#define AST2600_CONFIG_INDEX			0x2E
	#define AST2600_CONFIG_DATA				0x2F

	#define AST2600_LDN_SEL_REGISTER		0x07
	#define AST2600_ACTIVATE_REGISTER		0x30
	#define AST2600_BASE1_HI_REGISTER		0x60
	#define AST2600_BASE1_LO_REGISTER		0x61
	#define AST2600_IRQ1_REGISTER			0x70
	#define AST2600_IRQ1TYPE_REGISTER		0x71

	#define AST2600_ACTIVATE_VALUE			0x01
	#define AST2600_DEACTIVATE_VALUE		0x00
	#define AST2600_CONFIG_MODE_ENTER_VALUE	0xA5
	#define AST2600_CONFIG_MODE_EXIT_VALUE	0xAA

	#define COM1_PORT_ADDR					0x03F8
	#define COM2_PORT_ADDR					0x02F8
	#define AST2600_MAILBOX_BASE_ADDRESS	0x0600
	/* #define MAILBOX_ADDR					0x0CC0 */

	#define AST2600_LDN_UART1				0x02
	#define AST2600_LDN_UART2				0x03
	#define AST2600_LDN_SWC					0x04
	#define AST2600_LDN_GPIO				0x07
	#define AST2600_LDN_UART3				0x0B
	#define AST2600_LDN_UART4				0x0C
	#define AST2600_LDN_ILPC2AHB			0x0D
	#define AST2600_LDN_MAILBOX				0x0E
	#define AST2600_LDN_LSAFS				0x0F
#endif

#define MAX_ARGUMENT_STRING					16

#define SIZE_CMD_ARGS						16
#define SIZE_CMD_PACKET						8
#define SIZE_MAX_ID_SN						16
#define STX									0x02
#define ETX									0x03

#define CMD_CHECK_CONNECTION				0x41
#define CMD_GET_IP_INFO						0x71
#define CMD_SET_IP_INFO						0x78

#define TYPE_IP_ADDRESS						1
#define TYPE_SUBNET_MASK					2
#define TYPE_GATEWAY						3

#define	SETTINGS  							(_COM_19200 | _COM_NOPARITY | _COM_STOP1 | _COM_CHR8)
#define	TO_UPPER_CASE(c)					(((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c))

#ifdef COMM_PORT_AX99100
	#define COMM_PORT						0x1020
#else
	#define COMM_PORT						(PORT2)
#endif

//--- Define of functions -----------------------------------------------------
#ifdef EFI
unsigned char inportb(int port);
unsigned char outportb(int port, unsigned char value);
void delay(int ms);
#endif

#if (IPREADER_VERSION >= 033)
int readIpInfo(void);
int writeIpInfo(char *strAddress);
#else
int readIpInfo(int type);
int writeIpInfo(int type, char *strAddress);
#endif

#ifdef COM_PORT_BMC_2F8
void lpc_suart_init(int lcn, int port, int irq);
#endif

int checkConnection(void);
int sendAndReceiveCommand(int port);
void initTxBuffer(void);
void initRxBuffer(void);
void sendUartBuffer(int port, unsigned char *tbuf, int tcnt);
int receiveUartBuffer(int port, int rxLen, int timeOut);
int checkRxDataValidity(int rxLen);
void initUartPort(int port);
// void clearUartReceiveBufferRegister(int port);
// void waitUartTransferBufferReady(int port);
void convertToUpperCase(char *src, char *dest);
int convertStringToInt(char const *str);

//--- Define of global variables ----------------------------------------------
unsigned char g_rxbuf[SIZE_CMD_PACKET];
unsigned char g_txbuf[SIZE_CMD_PACKET];

// unsigned char ipAddress[4] = {0,};
// unsigned char subnetMask[4] = {0};
// unsigned char gateway[4] = {0,};
int address[4] = {0,};

int year = 0;
int month = 0;
int date = 0;

#ifdef COMM_PORT_AX99100
UINT8 AX9100_BusNumber = 0xFF;
#endif

/******************************************************************************
 *	EFI only FUNCTIONs
 *****************************************************************************/
#ifdef EFI
unsigned char inportb(int port)
{
	return (unsigned char)IoRead8(port);
}

unsigned char outportb(int port, unsigned char value)
{
	IoWrite8(port, value);
	return 0;
}

void delay(int ms)
{
	int us = ms * 1000;
	gBS->Stall(us);
}

#ifdef COMM_PORT_AX99100
void PciRead16(UINT32 CfgBase, UINT8 bus, UINT8 dev, UINT8 func, UINT16 reg, UINT16 *data)
{
    UINT8 *regAddr;

    regAddr = (UINT8 *)(CfgBase | PCIE_REG_ADDR(bus, dev, func, reg));
    *data = *(volatile UINT16 *)regAddr;
}

void PciWrite16(UINT32 CfgBase, UINT8 bus, UINT8 dev, UINT8 func, UINT16 reg, UINT16 data)
{
    UINT8 *regAddr;

    regAddr = (UINT8 *)(CfgBase | PCIE_REG_ADDR(bus, dev, func, reg));

    *(volatile UINT16 *)regAddr = data;
}
#endif
#endif // EFI

#ifdef COM_PORT_BMC_2F8
void lpc_suart_init(int lcn, int port, int irq)
{
	/* Super IO Password Register: A5 Enable, 0xAA Disable */
	outportb(AST2600_CONFIG_INDEX, AST2600_CONFIG_MODE_ENTER_VALUE);
	outportb(AST2600_CONFIG_INDEX, AST2600_CONFIG_MODE_ENTER_VALUE);

	/* 2f8 port (LDN3) setup */
	/* Super IO Logical number select */
	outportb(AST2600_CONFIG_INDEX, AST2600_LDN_SEL_REGISTER);
	/* [3:0] Logical device number -> SUART2 select */
	outportb(AST2600_CONFIG_DATA, (unsigned char)lcn);

	/* [0] Enable SUART2 */
	outportb(AST2600_CONFIG_INDEX, AST2600_ACTIVATE_REGISTER);
	/* [0] disable SUART2 */
	outportb(AST2600_CONFIG_DATA, AST2600_DEACTIVATE_VALUE);

	/* set SUART2 I/O port to 0x2F8 */
	outportb(AST2600_CONFIG_INDEX, AST2600_BASE1_HI_REGISTER);
	/* [7:0] SUART1 base address bit[15:8] */
	outportb(AST2600_CONFIG_DATA, (unsigned char)((port>>8) & 0x00FF));

	/* set SUART2 I/O port to 0x2F8 */
	outportb(AST2600_CONFIG_INDEX, AST2600_BASE1_LO_REGISTER);
	/* [7:3] SUART1 base address bit[7:3] */
	outportb(AST2600_CONFIG_DATA, (unsigned char)((port>>0) & 0x00FF));

	/* set SUART1 SIRQ to 0x3 */
	outportb(AST2600_CONFIG_INDEX, AST2600_IRQ1_REGISTER);
	/* [3:0] LPC: Select ID bit[3:0] of SerIRQ for SUART2 */
	outportb(AST2600_CONFIG_DATA, (unsigned char)irq);

	/* set SUART2 SIRQ type to rising edge trigger */
	/* [1:0] Host SerIRQ interrupt type for SUART2 */
	outportb(AST2600_CONFIG_INDEX, AST2600_IRQ1TYPE_REGISTER);
	/* BIOS setting in eSPI mode */
	outportb(AST2600_CONFIG_DATA, 0x00);

	/* [0] enable SUART2 */
	outportb(AST2600_CONFIG_INDEX, AST2600_ACTIVATE_REGISTER);
	outportb(AST2600_CONFIG_DATA, AST2600_ACTIVATE_VALUE);

	/* apply SIO setting and lock */
	outportb(AST2600_CONFIG_INDEX, AST2600_CONFIG_MODE_EXIT_VALUE);
}
#endif

/******************************************************************************
 *	SUB FUNCTIONS
 *****************************************************************************/
void convertDate(void)
{
	char strDate[] = __DATE__;
	int i;
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	year = atoi(strDate+7);
	strDate[6] = 0;
	date = atoi(strDate+4);
	strDate[3] = 0;

	for (i=0; i<12; i++) {
		if (!strcmp(strDate, months[i])) {
			month = i + 1;
			break;
		}
	}
}

/**
 * @brief  convert string to upper case
 * @note
 * @param  *src: source string
 * @param  *dest: converted string
 * @retval None
 */
void convertToUpperCase(char *src, char *dest)
{
	if (src == NULL || dest == NULL)
		return;

	while (*src) {
		*dest = TO_UPPER_CASE(*src);
		src++;
		dest++;
	}
}

//=============================================================================
//	Func:	Check connect
//	Input:	None
//	Output:	TRUE/FALSE
//=============================================================================
/**
 * @brief  check connection function
 * @note
 * @retval
 */
int checkConnection(void)
{
	int rc = 0;

	initTxBuffer();
	g_txbuf[1] = CMD_CHECK_CONNECTION;			// 0x42, Check Connection (20 bytes packet)

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Connection Ok!\n\n");
	else
		printf("  Connection Error!\n\n");

	return rc;
}

#if (IPREADER_VERSION >= 033)
/**
 * @brief  read IP information from MCU
 * @note
 * @param  type: 1:IP, 2:SubnetMask, 3:GateWay
 * @retval
 */
int readIpInfo(void)
{
	int rc = 0;
	FILE *fp;

	if ((fp = fopen("ReadIp.TXT", "w")) == NULL) {
		printf("  ReadIp.TXT fopen() error\n\n");
		return 0;
	}

	initTxBuffer();
	g_txbuf[1] = CMD_GET_IP_INFO;
	g_txbuf[2] = TYPE_IP_ADDRESS;
	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc == 1) {
		address[0] = g_rxbuf[3];
		address[1] = g_rxbuf[4];
		address[2] = g_rxbuf[5];
		address[3] = g_rxbuf[6];

		printf("  BOARD IP:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
		printf("  TPC IP:%d.%d.%d.250\n", address[0], address[1], address[2]);
		printf("  TPC PORT:30001\n");
		printf("  TEMP IP:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3] + 5);
		printf("  TEMP PORT:30002\n");
		printf("  NETMASK:255.255.0.0\n");
		printf("  GATEWAY:%d.%d.1.1\n", address[0], address[1]);
		// printf("  Version:%d.%d.%d[%d.%02d.%02d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);

		fprintf(fp, "BOARD IP:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
		fprintf(fp, "TPC IP:%d.%d.%d.250\n", address[0], address[1], address[2]);
		fprintf(fp, "TPC PORT:30001\n");
		fprintf(fp, "TEMP IP:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3] + 5);
		fprintf(fp, "TEMP PORT:30002\n");
		fprintf(fp, "NETMASK:255.255.0.0\n");
		fprintf(fp, "GATEWAY:%d.%d.1.1\n", address[0], address[1]);
		fprintf(fp, "Version:%d.%d.%d[%d.%02d.%02d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);
	}
	printf("  ReadIp.TXT file create Ok!\n\n");
	fclose(fp);

	return 1;
}

/**
 * @brief  write ip information to MCU
 * @note
 * @param  type: 1:IP, 2:SubnetMask, 3:GateWay
 * @retval
 */
int writeIpInfo(char *strAddress)
{
	int i;
	int rc = 0;
	int dotCnt = 0;
	int lenAddrStr = strlen(strAddress);

	for (i=0; i<lenAddrStr; i++) {
		if (strAddress[i] == '.') {
			dotCnt++;
		}
	}

	if (dotCnt != 3) {
		printf("  [ERROR] Invalid Address number...\n");
		return 0;
	}

	for (i=0; i<lenAddrStr; i++) {
		if (!((strAddress[i] >= '0' && strAddress[i] <= '9') || strAddress[i] == '.')) {
			printf("  [ERROR] Argument ERROR\n");
			return 0;
		}
	}

	sscanf(strAddress, "%d.%d.%d.%d", &address[0], &address[1], &address[2], &address[3]);

	initTxBuffer();
	g_txbuf[1] = CMD_SET_IP_INFO;
	g_txbuf[2] = TYPE_IP_ADDRESS;
	g_txbuf[3] = (unsigned char)address[0];
	g_txbuf[4] = (unsigned char)address[1];
	g_txbuf[5] = (unsigned char)address[2];
	g_txbuf[6] = (unsigned char)address[3];

	rc = sendAndReceiveCommand(COMM_PORT);

	printf("  Write BOARD IP:%d.%d.%d.%d OK\n", address[0], address[1], address[2], address[3]);

	return rc;
}
#else
/**
 * @brief  read IP information from MCU
 * @note
 * @param  type: 1:IP, 2:SubnetMask, 3:GateWay
 * @retval
 */
int readIpInfo(int type)
{
	// int i;
	int rc = 0;
	FILE *fp;

	if (type == TYPE_IP_ADDRESS) {
		if ((fp = fopen("EP_R_IP.TXT", "w")) == NULL) {
			printf("  EP_R_IP.TXT fopen() error\n\n");
			return 0;
		}
	}
	else if (type == TYPE_SUBNET_MASK) {
		if ((fp = fopen("EP_R_SM.TXT", "w")) == NULL) {
			printf("  EP_R_SM.TXT fopen() error\n\n");
			return 0;
		}
	}
	else if (type == TYPE_GATEWAY) {
		if ((fp = fopen("EP_R_GW.TXT", "w")) == NULL) {
			printf("  EP_R_GW.TXT fopen() error\n\n");
			return 0;
		}
	}

	initTxBuffer();
	g_txbuf[1] = CMD_GET_IP_INFO;
	g_txbuf[2] = (UINT8)type;
	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc == 1) {
		address[0] = g_rxbuf[3];
		address[1] = g_rxbuf[4];
		address[2] = g_rxbuf[5];
		address[3] = g_rxbuf[6];
		if (type == TYPE_IP_ADDRESS) {
			printf("  IP:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
			fprintf(fp, "IP:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
			fprintf(fp, "Version:%d.%d.%d[%d.%02d.%02d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);
		}
		else if (type == TYPE_SUBNET_MASK) {
			printf("  SUBNETMASK:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
			fprintf(fp, "SUBNETMASK:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
			fprintf(fp, "Version:%d.%d.%d[%d.%02d.%02d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);
		}
		else if (type == TYPE_GATEWAY) {
			printf("  GATEWAY:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
			fprintf(fp, "GATEWAY:%d.%d.%d.%d\n", address[0], address[1], address[2], address[3]);
			fprintf(fp, "Version:%d.%d.%d[%d.%02d.%02d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);
		}
	}

	if (type == TYPE_IP_ADDRESS) {
		printf("  EP_R_IP.TXT file create Ok!\n\n");
	}
	else if (type == TYPE_SUBNET_MASK) {
		printf("  EP_R_SM.TXT file create Ok!\n\n");
	}
	else if (type == TYPE_GATEWAY) {
		printf("  EP_R_GW.TXT file create Ok!\n\n");
	}

	fclose(fp);

	return 1;
}

/**
 * @brief  write ip information to MCU
 * @note
 * @param  type: 1:IP, 2:SubnetMask, 3:GateWay
 * @retval
 */
int writeIpInfo(int type, char *strAddress)
{
	int i;
	int rc = 0;
	int dotCnt = 0;
	int lenAddrStr = strlen(strAddress);

	for (i=0; i<lenAddrStr; i++) {
		if (strAddress[i] == '.') {
			dotCnt++;
		}
	}

	if (dotCnt != 3) {
		printf("  [ERROR] Invalid Address number...\n");
		return 0;
	}

	for (i=0; i<lenAddrStr; i++) {
		if (!((strAddress[i] >= '0' && strAddress[i] <= '9') || strAddress[i] == '.')) {
			printf("  [ERROR] Argument ERROR\n");
			return 0;
		}
	}

	sscanf(strAddress, "%d.%d.%d.%d", &address[0], &address[1], &address[2], &address[3]);

	initTxBuffer();
	g_txbuf[1] = CMD_SET_IP_INFO;
	g_txbuf[2] = (UINT8)type;
	g_txbuf[3] = (unsigned char)address[0];
	g_txbuf[4] = (unsigned char)address[1];
	g_txbuf[5] = (unsigned char)address[2];
	g_txbuf[6] = (unsigned char)address[3];

	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc == 1) {
		if (type == TYPE_IP_ADDRESS) {
			printf("  SET IP:%d.%d.%d.%d OK\n", address[0], address[1], address[2], address[3]);
		}
		else if (type == TYPE_SUBNET_MASK) {
			printf("  SET SubnetMask:%d.%d.%d.%d OK\n", address[0], address[1], address[2], address[3]);
		}
		else if (type == TYPE_GATEWAY) {
			printf("  SET GateWay:%d.%d.%d.%d OK\n", address[0], address[1], address[2], address[3]);
		}
	}

	return rc;
}
#endif

/**
 * @brief  send command and receive return command
 * @note
 * @retval
 */
int sendAndReceiveCommand(int port)
{
	int i;
	int rc = 0;
	int rxCnt;

	for (i=0; i<MAX_TRY_CNT; i++) {
		sendUartBuffer(port, g_txbuf, SIZE_CMD_PACKET);
		delay(100);

		initRxBuffer();
		rxCnt = receiveUartBuffer(port, SIZE_CMD_PACKET, 1000);
		if (rxCnt < SIZE_CMD_PACKET) {
			printf("  [ERROR] Receive Size Error. nReceiveData: %d\n", rxCnt);
		}

		if (checkRxDataValidity(rxCnt)) {
			return 1;
		}

		printf("  Retry [%d]\n");
		delay(200);
	}

	return rc;
}

/**
 * @brief  initialize transfer buffer
 * @note
 * @retval None
 */
void initTxBuffer(void)
{
	int i;

	for (i=0; i<SIZE_CMD_PACKET; i++) {
		g_txbuf[i] = 0;
	}
	g_txbuf[0] = STX;
	g_txbuf[SIZE_CMD_PACKET - 1] = ETX;
}

/**
 * @brief  Initialize Receive Buffer
 * @note
 * @retval None
 */
void initRxBuffer(void)
{
	int i;

	for (i=0; i<SIZE_CMD_PACKET; i++) {
		g_rxbuf[i] = 0;
	}
}

/**
 * @brief  Send buffer to inputed UART port
 * @note
 * @param  port: UART base address
 * @param  *tbuf: send buffer
 * @param  tcnt: send counter
 * @retval None
 */
void sendUartBuffer(int port, unsigned char *tbuf, int tcnt)
{
	int i;
	unsigned char regData;

	i = 0;
	while (i < tcnt) {
		do {
			regData = IoRead8(port + 5);
		}
		while (!(regData & (1<<5)));

		outportb(port, tbuf[i]);
		delay(1);
		i++;
	}
}

/**
 * @brief  Receive buffer from inputed UART port
 * @note
 * @param  port: UART base address
 * @param  rxLen: number of bytes to receive
 * @param  timeOut: max try count number to wait
 * @retval received bytes
 */
int receiveUartBuffer(int port, int rxLen, int timeOut)
{
	// int i;
	int tryCnt = 0;
	int rxCnt = 0;
	unsigned char c;

	while (rxCnt < rxLen) {
		c = inportb(port + 5);		// Check to see if char has been received.
		if (c & 1) {
			g_rxbuf[rxCnt] = inportb(port);
			rxCnt++;
		}
		delay(1);

		tryCnt++;
		if (tryCnt >= timeOut) {
			return rxCnt;
		}
	}

	return rxCnt;
}

/**
 * @brief  check RX data validity
 * @note
 * @retval
 */
int checkRxDataValidity(int rxLen)
{
	unsigned char retCommand;

	retCommand = g_txbuf[1] & 0xBF;
	retCommand |= 0x80;

	if (g_rxbuf[0] != STX) {
		printf("  Command Start Error.\n");
		return 0;
	}

	if (g_rxbuf[SIZE_CMD_PACKET - 1] != ETX) {
		printf("  Command End Error.\n");
		return 0;
	}

	if (g_rxbuf[1] != retCommand) {
		printf("  Return Command Error.\n");
		return 0;
	}

	if (g_rxbuf[1] >= 0xE0) {
		printf("  Error Code Returned. (ErrCode=0x%x)\n", g_rxbuf[1]);
		return 0;
	}

	return 1;
}

/**
 * @brief  Initialize UART port
 * @note
 * @param  port: UART base address
 * @retval None
 */
void initUartPort(int port)
{
	IoWrite8(0x80, 0x08);
	IoWrite8(port+1, 0);
	IoWrite8(port+3,0x80);
	IoWrite8(port, 0x01);    // 115200 bps
	IoWrite8(port+3, 0x03);
	IoWrite8(port+2, 0xC7);
	IoWrite8(port+1, 0x00);

	delay(100);						// Delay 20ms
}

/**
 * @brief  Clear receiver buffer register
 * @note
 * @param  port: UART base address
 * @retval None
 */
// void clearUartReceiveBufferRegister(int port)
// {
// 	int i;
// 	unsigned char c;
// 	unsigned char ch;

// 	for (i = 0; i < 100; i++) {
// 		c = inportb(port + 5);		// Check to see if char has been received.
// 		if (c & 1) {				// If so, then get char
// 			ch = (unsigned char)inportb(port);
// 			c = ch;					// Dummy code for delete warning
// 			break;
// 		}
// 		delay(1);
// 	}
// }

/**
 * @brief  Wait for Transfer Buffer Empty
 * @note
 * @param  port: UART base address
 * @retval None
 */
// void waitUartTransferBufferReady(int port)
// {
// 	int i;
// 	unsigned char c;

// 	for (i = 0; i < 100; i++) {
// 		c = inportb(port + 5);		// Read line status register
// 		if (c & 0x20) {
// 			break;    				// THR(Transmitter holding register) empty?
// 		}
// 		delay(1);
// 	}
// }

void displayHelpMsg(void)
{
#if (IPREADER_VERSION >= 033)
	printf("  usage : IPReader [-R/-W] [address]\n");
	printf("  examples:\n");
	printf("    IPReader -W address     (Write BOARD IP)\n");
	printf("    IPReader -R             (Read BOARD IP)\n");
#else
	printf("  usage : IPReader [command] [address]\n");
	printf("  examples:\n");
	printf("    IPReader -SETIP address     (Set IP Address)\n");
	printf("    IPReader -SETMASK address   (Set Subnet Mask)\n");
	printf("    IPReader -SETGW address     (Set Gateway)\n");
	printf("    IPReader -GETIP             (Get IP Address)\n");
	printf("    IPReader -GETMASK           (Get Subnet Mask)\n");
	printf("    IPReader -GETGW             (Get Gateway)\n");
#endif
}

/**
 * @brief  IDReader main routine
 * @note
 * @param  argc:
 * @param  **argv:
 * @retval
 */
int	main(int argc, char **argv)
{
	int errorFlag = 0;
	char optionCmd[MAX_ARGUMENT_STRING] = {0,};
	#ifdef COMM_PORT_AX99100
	UINT16 vendorID = 0;
	UINT16 devID = 0;
	UINT8 busID;
	#endif

	convertDate();
	printf("IPReader for PCT3.0-DDR5 V%d.%d.%d [%d.%d.%d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);
	// printf("IPReader for DDR5 PCT3.0 V%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2);

	#ifdef COMM_PORT_AX99100
	for (busID = 0; busID < 0xFF; busID++) {
		vendorID = 0;
		devID = 0;
		PciRead16(0x80000000, busID, 0, 0, PCI_VENDOR_ID_OFFSET, &vendorID);
		PciRead16(0x80000000, busID, 0, 0, PCI_DEVICE_ID_OFFSET, &devID);
		// printf("  [BUS=0x%x] Vendor=0x%x, Device=0x%x\n", busID, vendorID, devID);
		if (vendorID == 0x125B && devID == 0x9100) {
			AX9100_BusNumber = busID;
			break;
		}
	}
	if (AX9100_BusNumber == 0xFF) {
		printf("  [ERROR] Can't find AX9100\n");
		return 0;
	}
	// else {
	// 	printf("  AX9100 BUS = 0x%x\n", AX9100_BusNumber);
	// }

	PciWrite16(0x80000000, AX9100_BusNumber, Ax9100_D_N, Ax9100_F_N, PCI_COMMAND_OFFSET, 0x7);
	#endif

	#ifdef COM_PORT_BMC_2F8
	lpc_suart_init(AST2600_LDN_UART2, PORT2, 3);
	#endif
	initUartPort(COMM_PORT);

	if ((argc == 1) || (argc > 3)) {
		displayHelpMsg();
		exit(1);
		return 0;
	}

	convertToUpperCase(argv[1], optionCmd);

#if (IPREADER_VERSION >= 033)
	if (argc == 2) {
		if (!strcmp(optionCmd, "-R")) {
			errorFlag = readIpInfo();
		}
	}
	else {
		if (!strcmp(optionCmd, "-W")) {
			errorFlag = writeIpInfo(argv[2]);
		}
	}
#else
	if (argc == 2) {
		if (!strcmp(optionCmd, "-GETIP")) {
			errorFlag = readIpInfo(TYPE_IP_ADDRESS);
		}
		else if (!strcmp(optionCmd, "-GETMASK")) {
			errorFlag = readIpInfo(TYPE_SUBNET_MASK);
		}
		else if (!strcmp(optionCmd, "-GETGW")) {
			errorFlag = readIpInfo(TYPE_GATEWAY);
		}
	}
	else {
		if (!strcmp(optionCmd, "-SETIP")) {
			errorFlag = writeIpInfo(TYPE_IP_ADDRESS, argv[2]);
		}
		else if (!strcmp(optionCmd, "-SETMASK")) {
			errorFlag = writeIpInfo(TYPE_SUBNET_MASK, argv[2]);
		}
		else if (!strcmp(optionCmd, "-SETGW")) {
			errorFlag = writeIpInfo(TYPE_GATEWAY, argv[2]);
		}
	}
#endif

	if (errorFlag == 1)
		exit(0);
	else
		exit(1);

	return 0;
}
