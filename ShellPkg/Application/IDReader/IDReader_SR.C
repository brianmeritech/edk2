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

// #define COMM_PORT_AX99100
#define COM_PORT_BMC_2F8

#ifndef EFI
#define	FALSE						0
#define	TRUE						1
#endif	// DOS

/*******************************************************************************
 *	Date		Version		Comment
 *	21/02/23	V0.1.0		Initial Version
 *	21/03/31	V0.2.0		Change BUS Number
 *	21/04/01	V0.2.1		Find AX9100 Bus Number
 *	21/04/05	V0.2.2		Change delay time to read board information
 *	21/08/18	V0.3.0		Change COMM PORT AX99100 -> BMC 2F8
 *	21-08-25	V0.5.1		Add BMC 2F8 port Init routine
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						5
#define VERSION_MINOR2						1

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

#define SIZE_CMD_ARGS						16
#define SIZE_CMD_PACKET						8
#define SIZE_MAX_ID_SN						16
#define STX									0x02
#define ETX									0x03

#define CMD_CHECK_CONNECTION				0x41
#define CMD_GET_BOARD_ID					0x72
#define CMD_SET_BOARD_ID					0x73
#define CMD_GET_BOARD_SN					0x74
#define CMD_SET_BOARD_SN					0x75

#define BOARD_SN							1
#define BOARD_ID							2

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

#ifdef COM_PORT_BMC_2F8
void lpc_suart_init(int lcn, int port, int irq);
#endif

int checkConnection(void);
int readBoardInfo(int type);
int writeBoardInfo(int type, char *idstr);
int sendAndReceiveCommand(int port);
void initTxBuffer(void);
void initRxBuffer(void);
void displayHelpMsg(void);
void sendUartBuffer(int port, unsigned char *tbuf, int tcnt);
int receiveUartBuffer(int port, int rxLen, int timeOut);
int checkRxDataValidity(int rxLen);
void initUartPort(int port);
void clearUartReceiveBufferRegister(int port);
void waitUartTransferBufferReady(int port);
void convertToUpperCase(char *src, char *dest);
int convertStringToInt(char const *str);

//--- Define of global variables ----------------------------------------------
unsigned char g_rxbuf[SIZE_CMD_PACKET];
unsigned char g_txbuf[SIZE_CMD_PACKET];

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

/******************************************************************************
 *	SUB FUNCTIONS
 *****************************************************************************/
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

#if 0
/**
 * @brief  read eeprom data
 * @note
 * @param  bd_id: board id, skt_bd=1, cpu_bd=2, pch_bd=3
 * @retval
 */
int readBoardEEPROM(int bd_id)
{
	int retFunc;
	int offset;
	int i;
	FILE *fp;

	if (bd_id == 1) {
		if ( (fp = fopen("EP_D_SKT.TXT", "w")) == NULL ) {
			printf("  EP_D_SKT.TXT fopen() error\n\n");
			return FALSE;
		}
		fprintf(fp, "MEMORY SOCKET BOARD, EEPROM Data\n\n");
	}
	else if (bd_id == 2) {
		if ( (fp = fopen("EP_D_CPU.TXT", "w")) == NULL ) {
			printf("  EP_D_CPU.TXT fopen() error\n\n");
			return FALSE;
		}
		fprintf(fp, "CPU BOARD, EEPROM Data\n\n");
	}
	else if (bd_id == 3) {
		if ( (fp = fopen("EP_D_PCH.TXT", "w")) == NULL ) {
			printf("  EP_D_PCH.TXT fopen() error\n\n");
			return FALSE;
		}
		fprintf(fp, "PCH BOARD, EEPROM Data\n\n");
	}

	for (offset = 0; offset < 0x10; offset++) {
		initTxBuffer();						// Make default TX buffer
		g_txbuf[0] = GET_EEPD_CMD;			// 0x74, Get EEPROM DATA
		g_txbuf[1] = (unsigned char)bd_id;	// EEPROM selector & address
		g_txbuf[2] = (unsigned char)(offset * 0x10);
		retFunc = sendAndReceiveCommand();

		if (retFunc == FALSE) {
			printf("  Offset 0x%x Error!\n\n");
			return retFunc;
		}

		printf("  %02X : ", offset * 0x10);
		for (i=0; i<16; i++) {
			printf("%02X ", g_rxbuf[i+3]);
		}
		printf("\n");

		fprintf(fp, "  %02X : ", offset * 0x10);
		for (i=0; i<16; i++) {
			fprintf(fp, "%02X ", g_rxbuf[i+3]);
		}
		fprintf(fp, "\n");

		delay(1000);
	}

	if (bd_id == 1)
		printf("  EP_D_SKT.TXT file create Ok!\n\n");
	else if (bd_id == 2)
		printf("  EP_D_CPU.TXT file create Ok!\n\n");
	else if (bd_id == 3)
		printf("  EP_D_PCH.TXT file create Ok!\n\n");

	fclose(fp);
	return retFunc;
}
#endif

/**
 * @brief  read board Info
 * @note
 * @retval None
 */
int readBoardInfo(int type)
{
	int i, j;
	int rc = 0;
	FILE *fp;

	char strID[SIZE_MAX_ID_SN + 1];

	if (type == BOARD_ID) {
		if ((fp = fopen("EP_R_ID.TXT", "w")) == NULL) {
			printf("  EP_R_ID.TXT fopen() error\n\n");
			return 0;
		}
	}
	else if (type == BOARD_SN) {
		if ((fp = fopen("EP_R_SN.TXT", "w")) == NULL) {
			printf("  EP_R_SN.TXT fopen() error\n\n");
			return 0;
		}
	}

	for (i=0; i<SIZE_MAX_ID_SN+1; i++) {
		strID[i] = '\0';
	}

	for (i=0; i<4; i++) {
		initTxBuffer();
		if (type == BOARD_ID) {
			g_txbuf[1] = CMD_GET_BOARD_ID;
		}
		else if (type == BOARD_SN) {
			g_txbuf[1] = CMD_GET_BOARD_SN;
		}
		g_txbuf[2] = (unsigned char)i;

		rc = sendAndReceiveCommand(COMM_PORT);
		if (rc) {
			for (j=0; j<4; j++) {
				if (g_rxbuf[3+j] >= 0x20 && g_rxbuf[3+j] <= 0x7F) {
					strID[i*4 + j] = g_rxbuf[3+j];
				}
			}
		}
		else {
			if (type == BOARD_ID)
				printf("  [ERROR] Read Board ID Error, index=%d\n", i);
			else if (type == BOARD_SN)
				printf("  [ERROR] Read Board SN Error, index=%d\n", i);
			return 0;
		}

		delay(10);
	}

	if (type == BOARD_ID) {
		printf("  ID : [%s]\n", strID);
		fprintf(fp, "%s", strID);
		printf("  EP_R_ID.TXT file create Ok!\n\n");
	}
	else if (type == BOARD_SN) {
		printf("  SN : [%s]\n", strID);
		fprintf(fp, "%s", strID);
		printf("  EP_R_SN.TXT file create Ok!\n\n");
	}

	fclose(fp);

	return 1;
}

/**
 * @brief  Write Board SN or ID
 * @note
 * @param  type:
 * @param  *idstr:
 * @retval
 */
int writeBoardInfo(int type, char *idstr)
{
	int i, j;
	int rc = 0;

	for (i=0; i<4; i++) {
		initTxBuffer();
		if (type == BOARD_ID)
			g_txbuf[1] = CMD_SET_BOARD_ID;
		else if (type == BOARD_SN)
			g_txbuf[1] = CMD_SET_BOARD_SN;
		g_txbuf[2] = (unsigned char)i;

		for (j=0; j<4; j++) {
			if (idstr[i*4 + j] == 0)
				break;
			g_txbuf[j+3] = idstr[i*4 + j];
		}

		rc = sendAndReceiveCommand(COMM_PORT);
		if (rc == 0) {
			if (type == BOARD_ID)
				printf("  [ERROR] Write ID error. index=%d\n", i);
			else if (type == BOARD_SN)
				printf("  [ERROR] Write SN error. index=%d\n", i);
			break;
		}
		delay(100);
	}

	if (rc) {
		if (type == BOARD_ID)
			printf("  Write ID [%s] OK\n", idstr);
		else if (type == BOARD_SN)
			printf("  Write SN [%s]OK\n", idstr);
	}

	return rc;
}

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
		delay(10);

		initRxBuffer();
		rxCnt = receiveUartBuffer(port, SIZE_CMD_PACKET, 1000);
		if (rxCnt < SIZE_CMD_PACKET) {
			printf("  [ERROR] Receive Size Error. nReceiveData: %d\n", rxCnt);
		}

		if (checkRxDataValidity(rxCnt)) {
			return 1;
		}

		printf("  Retry [%d]\n");
		delay(100);
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
 * @brief  display usage help message
 * @note
 * @retval None
 */
void displayHelpMsg(void)
{
	printf("  usage : IDREADER [-R/W] [-SN/-ID] <ID String>\n");
	printf("          IDREADER -CC     (Check connection)\n");
	printf("          IDREADER -R -SN  (Read Bd ID -> EP_R_ID.TXT)\n");
	printf("          IDREADER -R -ID  (Read Bd SN -> EP_R_SN.TXT)\n");
	printf("          IDREADER -W -SN  Serial Number (Write Bd SN )\n");
	printf("          IDREADER -W -ID  ID            (Write Bd ID )\n");
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

	// printf("    [TX] ");
	i = 0;
	while (i < tcnt) {
		do {
			regData = IoRead8(port + 5);
		}
		while (!(regData & (1<<5)));

		// printf("%02X ", tbuf[i]);
		outportb(port, tbuf[i]);
		delay(1);
		i++;
	}
	// printf("\n");
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
void clearUartReceiveBufferRegister(int port)
{
	int i;
	unsigned char c;
	unsigned char ch;

	for (i = 0; i < 100; i++) {
		c = inportb(port + 5);		// Check to see if char has been received.
		if (c & 1) {				// If so, then get char
			ch = (unsigned char)inportb(port);
			c = ch;					// Dummy code for delete warning
			break;
		}
		delay(1);
	}
}

/**
 * @brief  Wait for Transfer Buffer Empty
 * @note
 * @param  port: UART base address
 * @retval None
 */
void waitUartTransferBufferReady(int port)
{
	int i;
	unsigned char c;

	for (i = 0; i < 100; i++) {
		c = inportb(port + 5);		// Read line status register
		if (c & 0x20) {
			break;    				// THR(Transmitter holding register) empty?
		}
		delay(1);
	}
}

/**
 * @brief  convert string to upper case
 * @note
 * @param  *str: string
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

/**
 * @brief  convert decimal string to int value
 * @note
 * @param  *str: decimal string
 * @retval converted value
 */
int convertStringToInt(char const *str)
{
	int val = 0;
	int isPositive = 1;

	if (str == NULL) return 0;

	if (*(str) == '-') {			// Negative?
		isPositive = -1;
		str++;
	}

	while (*str) {
		if (*str >= '0' && *str <= '9') {
			val = (val * 10) + (*str) - '0';
		}
		str++;
	}

	return val * isPositive;
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
	char strCmd1[SIZE_CMD_ARGS] = {0,};
	char strCmd2[SIZE_CMD_ARGS] = {0,};
	#ifdef COMM_PORT_AX99100
	UINT16 vendorID = 0;
	UINT16 devID = 0;
	UINT8 busID;
	#endif

	convertDate();
	printf("IDReader for PCT3.0-DDR5 V%d.%d.%d [%d.%d.%d]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MINOR2, year, month, date);

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

	#ifdef OPTION_MSG
	printf("  argc = %d\n", argc);
	for (i = 0; i < argc; i++) {
		printf("    argv[%d] = [%s]\n", i, argv[i]);
	}
	#endif

	if ((argc == 1) || (argc > 4)) {
		displayHelpMsg();
		exit(1);
		return 0;
	}

	convertToUpperCase(argv[1], strCmd1);

	#ifdef COM_PORT_BMC_2F8
	lpc_suart_init(AST2600_LDN_UART2, PORT2, 3);
	#endif
	initUartPort(COMM_PORT);

	if (!strcmp(strCmd1, "-CC")) {
		errorFlag = checkConnection();
	}
	// Read EEPROM ID
	else if (!strcmp(strCmd1, "-R")) {
		if (argc < 3) {
			printf("  [ERROR] Not enough command options.\n");
			errorFlag = 1;
		}
		else {
			convertToUpperCase(argv[2], strCmd2);
			if (!strcmp(strCmd2, "-SN")) {
				errorFlag = readBoardInfo(BOARD_SN);
			}
			else if (!strcmp(strCmd2, "-ID")) {
				errorFlag = readBoardInfo(BOARD_ID);
			}
			else {
				printf("  [ERROR] %s is not valid command.\n", strCmd2);
				errorFlag = 1;
			}
		}
	}
	// Write EEPROM ID
	else if (!strcmp(strCmd1, "-W")) {
		if (argc < 4) {
			printf("  [ERROR] Not enough command options.\n");
			errorFlag = 1;
		}
		else {
			convertToUpperCase(argv[2], strCmd2);
			if (!strcmp(strCmd2, "-SN")) {
				errorFlag = writeBoardInfo(BOARD_SN, argv[3]);
			}
			else if (!strcmp(strCmd2, "-ID")) {
				errorFlag = writeBoardInfo(BOARD_ID, argv[3]);
			}
			else {
				printf("  [ERROR] %s is not valid command.\n", strCmd2);
				errorFlag = 1;
			}
		}
	}
	else {
		printf("  [ERROR] %s is not valid command.\n", strCmd1);
		errorFlag = 1;
	}

	if (errorFlag == 1)
		exit(0);
	else
		exit(1);

	return 0;
}
