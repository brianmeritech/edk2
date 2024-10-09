//*****************************************************************************
//	VC_SR
//	Meritech SpahireRapid DDR5 Board Voltage Control Program
//	MCU : STM32F091VBT
//	EEPROM : AT24C04
//*****************************************************************************

// Options =====================================================================
#define VC_EFI
// #define DEBUG_MSG			// Debug message
// #define OPTION_MSG			// Option message
// #define USE_INTISR			// Use interrupt ISR when receive data
// #define CHECK_POWER_MODULE_TEMPERATUR
#define CPU_FAN_RPM_MONITORING
// #define SUPPORT_AC_POWER_MODE_FLAG			// Support AC Power Mode flag command

// #define COMM_PORT_AX99100
#define COM_PORT_BMC_2F8

#define SPR_CXM_MODEL_SUPPORT

// Header Files ================================================================
#include <stdio.h>
#include <stdlib.h>

#ifdef VC_EFI
	#include <Library/IoLib.h>
	#include <Library/UefiRuntimeServicesTableLib.h>
	#include <Library/UefiBootServicesTableLib.h>
	#include <string.h>
	#include <stdlib.h>
	#include <assert.h>
	#include <errno.h>
	#include <stdarg.h>
	#include <stdio.h>
#else
	#include <dos.h>
	#include <conio.h>
#endif

#ifndef VC_EFI
	#define	FALSE						0
	#define	TRUE						1
#endif

/*******************************************************************************
 *	Date		Version		Comment
 *	21/02/22	V0.1.0		Initial Version
 *	21/03/02	V0.2.0		Add Pulse Width command
 *	21/03/04	V0.2.1		Bug Fix
 *	21/03/09	V0.3.0		CHANGE LED & SET SLOT COUNT Protocol
 *	21/03/31	V0.4.0		Change BUS Number
 *	21/04/01	V0.4.1		Find AX9100 Bus Number
 *	21/05/04	V0.4.2		Bug Fix & Fix command
 *	21/08/18	V0.5.0		Change COMM PORT AX99100 -> BMC 2F8
 *	21-08-25	V0.5.1		Add BMC 2F8 port Init routine
 *	23-01-16	V0.5.2		Modify Help Message
 *	23-04-18	V0.5.3		Add FAN Speed Command
 *	23-07-13	V0.5.4		Add SPR CXL Support command
 *	23-07-20	V0.5.5		Add IO Expander Output control command
 ******************************************************************************/
#define	VERSION_MAJOR						0			// Major version
#define	VERSION_MINOR						5			// Minor version 1
#define	VERSION_BUILD						5			// Minor version 2

#define MAX_TRY_CNT							5			// Maximum retry count for All command
#define	SIZE_ARGUMENT_MAX					16			// Maximum command string count

//--- Define key
#define	EscKey								0x1B		// Esc key

//=== Define Interrupt Controller Mask
// #define	INTC1							0x21		// Interrupt Controller 1 Mask
// #define	INTC2							0xA1		// Interrupt Controller 2 Mask

//--- Define Serial(UART) Ports Base Address
//	COM1 0x3F8, COM2 0x2F8
//	COM3 0x3E8, COM4 0x2E8
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
	#define PCI_MAXLAT_OFFSET             	0x3F ///< Max Latency Register#else
#else
	#define PORT1							0x3F8		// Super IO UART0
	#define PORT2							0x2F8		// Super IO UART1
	#define PORT3							0x3E8
	#define PORT4							0x2E8
#endif

//--- Define Com Port's IRQ (Must also change PIC Setting)
//	COM1 - 0x0C, COM2 - 0x0B
//	COM3 - 0x0C, COM4 - 0x0B
// #define	INT_COM1						0x0C		// COM1 Port's IRQ here
// #define	INT_COM2						0x0B		// COM2 Port's IRQ here
// #define	INT_COM3						0x0C		// COM3 Port's IRQ here
// #define	INT_COM4						0x0B		// COM4 Port's IRQ here

//--- Define RX & TX count & tail
#define	RECV_BUFF_SIZE						1024
#define	SIZE_CMD_PACKET						8
#define STX									0x02
#define ETX									0x03

// --- Command Definition
#define CMD_CHECK_CONNECTION				0x41
#define CMD_PORT80_DATA						0x42
#define CMD_GET_FW_VERSION					0x51
#define CMD_GET_CUR_VOLTAGE					0x52
#define CMD_GET_SLEW_RATE					0x53
#define CMD_GET_BOOT_VOLTAGE				0x54
#define CMD_GET_MEM_COUNT					0x55
#define CMD_GET_FAN_RPM						0x56
#define CMD_SET_OUT_VOLTAGE					0x61
#define CMD_SET_SLEW_RATE					0x62
#define CMD_SET_BOOT_VOLTAGE				0x63
#define CMD_SET_LED							0x64
#define CMD_SET_MEM_COUNT_ACTION			0x65
#define CMD_SET_VOLTAGE_TUNE				0x66
#define CMD_SET_ADC_TUNE					0x67
#define CMD_SET_PWM_PULSE_WIDTH				0x68

//#define CMD_SEND_BEEP_TEST					0x68
#ifdef SPR_CXM_MODEL_SUPPORT
#define CMD_SET_REQ_COLD_RESET				0x6A
#define CMD_SET_CONTROL_CXL_PWR				0x6B
#define CMD_SET_IO_EXP_PORT_CONTROL			0x6C
#endif

//--- Define VDD, VPP MIN/MAX voltages
#define	MIN_VDD								4250	// Min. VDD volt = 4.25V
#define	MAX_VDD								15000	// Max. VDD volt = 15.00V
#define MIN_P3_3V							3000	// Min. P3.3 volt = 3.0V
#define MAX_P3_3V							3600	// Max. P3.3 volt = 3.6V

//--- Define Slew rate
#define	SLEW_060							1		// Slew =  60ms/0.1V
#define	SLEW_120							2		// Slew = 120ms/0.1V
#define	SLEW_200							3		// Slew = 200ms/0.1V
#define	SLEW_300							4		// Slew = 300ms/0.1V
#define	SLEW_500							5		// Slew = 500ms/0.1V

// #define	SETTINGS						(_COM_19200 | _COM_NOPARITY | _COM_STOP1 | _COM_CHR8)
#define	TO_UPPER_CASE(c)					(((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c))

#ifdef COMM_PORT_AX99100
	#define COMM_PORT						0x1020
#else
	#define COMM_PORT						(PORT2)
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

//-----------------------------------------------------------------------------
#ifdef VC_EFI
#else
int pBuffIn1 = 0;
int pBuffOut1 = 0;
int pBuffIn2 = 0;
int pBuffOut2 = 0;
#endif

int phead = 0;
int ptail = 0;
int	actPort;
unsigned char rxBuffer[SIZE_CMD_PACKET];
unsigned char txBuffer[SIZE_CMD_PACKET];

#ifndef VC_EFI
void interrupt(*oldPort1_ISR)();
void interrupt(*oldPort2_ISR)();
#else
// int idInitFunc();
#endif

char *STR_SLEW_RATE[5] = {"60ms/0.1V", "120ms/0.1V", "200ms/0.1V", "300ms/0.1V", "500ms/0.1V"};
char *strVoltCh[5] = {"P12V-AB", "P12V-CD", "P12V-EF", "P12V-GH", "P3.3V"};

#ifdef SPR_CXM_MODEL_SUPPORT
char *strIOExpPort[3] = {"NoAction", "LOW", "HIGH"};
#endif

int year = 0;
int month = 0;
int date = 0;

#ifdef COMM_PORT_AX99100
UINT8 AX9100_BusNumber = 0xFF;
#endif

//-----------------------------------------------------------------------------
#ifdef VC_EFI
unsigned char inportb(int port);
unsigned char outportb(int port, unsigned char value);
void delay(int msDelay);
void udelay(int usDelay);
#else
void interrupt newPort1_ISR();
void interrupt newPort2_ISR();
#endif

#ifdef COM_PORT_BMC_2F8
void lpc_suart_init(int lcn, int port, int irq);
#endif

void displayHelpMsg(void);
int checkConnection(void);
int getFirmwareVersion(void);
int getSlewRate(void);
int getFanRPM(char *strFan);
int setSlewRate(char *strSlewRate);
int convertLedStatusString(char *strStat, unsigned char *status);
int setLEDStatus(char *strStat);
int getOutputVoltage(char *strVrChannel);
int getBootVoltage(char *strVrChannel);
int setOutputVoltage(char *strVrChannel, char *strVoltage);
int setBootVoltage(char *strVrChannel, char *strBootVoltage);
int setVoltageTuningValue(char *strVrChannel, char *strTuning);
// int setFNDTuning(void);
int getIDFunc(void);
// int setBeepTest(void);
int getSlotCount(char *strSlotNumber);
int convertMemorySlotActionString(char *strStat, unsigned char *status);
int setSlotCountAction(char *strSlotAction);

#ifdef SPR_CXM_MODEL_SUPPORT
int setReqColdReset(char *strCXLPwr);
int setCXLPwrControl(char *strCXLPwr);
int setIOExpanderOutput(char *strCh, char *strP2, char *strP3);
#endif

void sendUartBuffer(int port, unsigned char *tbuf, int tcnt);
int receiveUartBuffer(int port, int rxLen, int timeOut);
int checkRxDataValidity(int rxLen);
int sendAndReceiveCommand(int port);
void initTxBuffer(void);
void initRxBuffer(void);
void initUartPort(int port);
void clearUartReceiveBufferRegister(int port);
void convertToUpperCase(char *src, char *dest);
int convertStringToInt(char const *str);
int convertStringToHex(char const *str);

//-----------------------------------------------------------------------------
#ifdef VC_EFI
unsigned char inportb(int port)
{
	return IoRead8(port);
}

unsigned char outportb(int port, unsigned char value)
{
	IoWrite8(port, value);
	return 0;
}

void delay(int msDelay)
{
	int usDelay = msDelay * 1000;
	gBS->Stall(usDelay);
}

void udelay(int usDelay)
{
	gBS->Stall(usDelay);
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
#else

//*** INTERRUPT Service ROUTINE ***********************************************

//#############################################################################
//	Func:	New Port1 ISR
//#############################################################################
void interrupt newPort1_ISR()	// Interrupt Service Routine (ISR) for PORT1
{
	int c;

	do {
		c = inportb(PORT1 + 5);
		if (c & 1) {
			uBuffer1[pBuffIn1] = inportb(PORT1);
			pBuffIn1++;
			if (pBuffIn1 >= RECV_BUFF_SIZE) {
				pBuffIn1 = 0;
			}
		}
	}
	while (c & 1);
	outportb(0x20, 0x20);		// Send EOI
}

//#############################################################################
//	Func:	New Port2 ISR
//#############################################################################
void interrupt newPort2_ISR()	// Interrupt Service Routine (ISR) for PORT2
{
	int c;

	do {
		c = inportb(PORT2 + 5);
		if (c & 1) {
			uBuffer2[pBuffIn2] = inportb(PORT2);
			pBuffIn2++;
			if (pBuffIn2 >= RECV_BUFF_SIZE) {
				pBuffIn2 = 0;
			}
		}
	}
	while (c & 1);
	outportb(0x20, 0x20);		// Send EOI
}
#endif

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

/**
 * @brief  display help message
 * @note
 * @retval None
 */
void displayHelpMsg(void)
{
	printf("  usage : VC [command] [ch] [mVolt]\n");
	printf("  examples:\n");
	printf("    VC -CC                     (Check Connection)\n");
	printf("    VC -GR                     (Get Firmware Version)\n");
	printf("    VC -GF [FAN_NUM]           (Get CPU FAN RPM)\n");
	// printf("    VC -GS                     (Get Slew Rate)\n");
	printf("    VC -ID                     (Execute LED Off, Create VOLTDEV.TXT & SPCLED.TXT)\n");
	printf("    VC -GV [V-CH]              (Get output Voltage)\n");
	printf("    VC -GB [V-CH]              (Get Boot Voltage)\n");
	// printf("    VC -SR [SLEW]              (Set Slew Rate)\n");
	printf("    VC -SL [LED-STAT]          (Set LED Control)\n");
	printf("    VC -GC [M-SLOT]            (Get Memory Slot Count)\n");
	printf("    VC -SC [M-ACTION]          (Set Memory Slot Count Action)\n");
	printf("    VC -SV [V-CH] [VOLTAGE]    (Set output Voltage)\n");
	printf("    VC -BV [V-CH] [VOLTAGE]    (Set Boot Voltage)\n");
	// printf("    VC -FT                     (Set FND Tuning)\n");
	// printf("    VC -ST [V-CH] [TUNE-VALUE] (Set Tuning Voltage)\n");
	printf("    VC -FS [FAN] [FAN_SPEED]   (Set FAN Speed Control)\n");
	#ifdef SPR_CXM_MODEL_SUPPORT
	printf("    VC -CR [CXL_PWR]           (Request Cold Reset with CXL Power Option)\n");
	printf("    VC -CP [CXL_PWR]           (Control CXL Power)\n");
	printf("    VC -IO [IO_EXP CH] [P2] [P3] (IO Expander Output)\n");
	#endif
	printf("  V-CH : 1 = P12V-AB, 2 = P12V-CD, 3 = P12V-EF, 4 = P12V-GH, 5 = P3.3V\n");
	printf("  Voltage range : %d mv <= P12V <= %d mV\n", MIN_VDD, MAX_VDD);
	printf("                  %d mV <= 3.3V <= %d mV\n", MIN_P3_3V, MAX_P3_3V);
	printf("  LED Control : P = Pass, F = Fail, B = Blank, X = No change\n");
	printf("  Memory Slot Count Action : + = Increase count, X = No Action, R = Reset Count\n");
	printf("  FAN_NUM : 1=CPU FAN 1/2, 2=CPU FAN 3/4, 3=SYS_FAN 1, 4=SYS_FAN 2\n");
	printf("  FAN : 1 = CPU FAN1, FAN2, 2 = CPU FAN 3, FAN4, 3 = CPU FAN1 ~ FAN4\n");
	printf("  FAN_SPEED : 3 ~ 10\n");
	#ifdef SPR_CXM_MODEL_SUPPORT
	printf("  CXL_PWR : 0 = CXL Power Off, 1 = CXL Power On\n");
	printf("  IO EXP CH : 0 = CH_A, 1 = CH_B, 2 = CH_C, 3 = CH_D\n");
	printf("  P2, P3 : IO Expander Pin2, Pin3 Output Value, 0 = no change, 1 = LOW, 2 = HIGH\n");
	#endif
}

/**
 * @brief  Check Connection
 * @note
 * @retval true/false
 */
int checkConnection(void)
{
	int rc;

	initTxBuffer();
	txBuffer[1] = CMD_CHECK_CONNECTION;

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Connection Ok!\n");
	else
		printf("  Connection Error!\n");

	return rc;
}

/**
 * @brief  get Firmware Version
 * @note
 * @retval true/false
 */
int getFirmwareVersion(void)
{
	int rc;

	initTxBuffer();
	txBuffer[1] = CMD_GET_FW_VERSION;
	// Main MCU Firmware와 Back I/O MCU Firmware를 구분하기 위함.
	// 최초 구현은 Main MCU Firmware Version만 요구. 차후 필요가 있을 경우 Back I/O
	// Firmware Version도 구현
	txBuffer[2] = 1;
	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc) {
		printf("  Firmware version : [%d.%d.%d] OK\n", rxBuffer[3], rxBuffer[4], rxBuffer[5]);
	}
	else
		printf("  [ERROR] Get Firmware version\n");

	return rc;
}

/**
 * @brief  get slew rate
 * @note
 * @retval true/false
 */
int getSlewRate(void)
{
	int rc;

	initTxBuffer();
	txBuffer[1] = CMD_GET_SLEW_RATE;
	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc) {
		if (rxBuffer[2] >= 1 && rxBuffer[2] <= 5)
			printf("  Slew Rate : [%s] OK\n", STR_SLEW_RATE[rxBuffer[2] - 1]);
		else
			printf("  [ERROR] Get Slew Rate\n");
	}
	else
		printf("  [ERROR] Get Slew Rate\n");

	return rc;
}

/**
 * @brief  get CPU Fan RPM
 * @note
 * @retval
 */
int getFanRPM(char *strFan)
{
	int rc;
	int rpm = 0;
	int fanIndex;

	fanIndex = convertStringToInt(strFan);
	if (fanIndex == 0 || fanIndex > 4) {
		printf("  [ERROR] Invalid Fan Index\n");
		return 0;
	}

	initTxBuffer();
	txBuffer[1] = CMD_GET_FAN_RPM;
	txBuffer[2] = (unsigned char)fanIndex;
	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc) {
		rpm = rxBuffer[3] | ((rxBuffer[4] << 8) & 0xFF00);
		printf("  FAN RPM : RPM = %d\n", rpm);
	}
	else
		printf("  [ERROR] Get FAN RPM\n");

	return rc;
}

/**
 * @brief
 * @note
 * @param  *strP80Data:
 * @retval
 */
int setPort80Data(char *strP80Data)
{
	int data;

	data = convertStringToHex(strP80Data);

	initTxBuffer();
	txBuffer[1] = CMD_PORT80_DATA;
	txBuffer[2] = (unsigned char)data;

	sendUartBuffer(COMM_PORT, txBuffer, SIZE_CMD_PACKET);

	return 1;
}

#ifdef SPR_CXM_MODEL_SUPPORT
/**
 * @brief  
 * @note   
 * @param  *strCXLPwr: 
 * @retval 
 */
int setReqColdReset(char *strCXLPwr)
{
	int rc;
	int cxl_pwr;

	cxl_pwr = convertStringToInt(strCXLPwr);

	initTxBuffer();
	txBuffer[1] = CMD_SET_REQ_COLD_RESET;
	txBuffer[2] = (unsigned char)cxl_pwr;
	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc) {
		printf("  Send request cold reset %d OK\n", cxl_pwr);
	}
	else
		printf("  [ERROR] Send request cold reset\n");

	return rc;
}

/**
 * @brief  
 * @note   
 * @param  *strCXLPwr: `
 * @retval 
 */
int setCXLPwrControl(char *strCXLPwr)
{
	int rc;
	int cxl_pwr;

	cxl_pwr = convertStringToInt(strCXLPwr);

	initTxBuffer();
	txBuffer[1] = CMD_SET_CONTROL_CXL_PWR;
	txBuffer[2] = (unsigned char)cxl_pwr;
	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc) {
		printf("  Send CXL Power Control %d OK\n", cxl_pwr);
	}
	else
		printf("  [ERROR] Send request cold reset\n");

	return rc;
}

/**
 * @brief  
 * @note   
 * @param  *strCh: 
 * @param  *strP2: 
 * @param  *strP3: 
 * @retval 
 */
int setIOExpanderOutput(char *strCh, char *strP2, char *strP3)
{
	int rc;
	int io_exp_ch;
	int p2, p3;

	io_exp_ch = convertStringToInt(strCh);
	p2 = convertStringToInt(strP2);
	p3 = convertStringToInt(strP3);

	if (io_exp_ch < 0 || io_exp_ch > 3) {
		printf("  IO Expander channel ERROR %d\n", io_exp_ch);
	}
	if (p2 < 0 || p2 >2 || p3 < 0 || p3 > 2) {
		printf("  IO Expander pin value ERROR %d %d\n", p2, p3);
	}

	initTxBuffer();
	txBuffer[1] = CMD_SET_IO_EXP_PORT_CONTROL;
	txBuffer[2] = (unsigned char)io_exp_ch;
	txBuffer[3] = (unsigned char)p2;
	txBuffer[4] = (unsigned char)p3;
	rc = sendAndReceiveCommand(COMM_PORT);

	if (rc) {
		printf("  Send IO Expander Output Control CH[%c] P2[%s] P3[%s] OK\n", 'A' + io_exp_ch, strIOExpPort[p2], strIOExpPort[p3]);
	}
	else
		printf("  [ERROR] Send IO Expander Output Control\n");

	return rc;
}
#endif

/**
 * @brief  set slew rate
 * @note
 * @param  strSlewRate: slew rate string
 * @retval
 */
int setSlewRate(char *strSlewRate)
{
	int rc;
	int valueSlewRate = 0;
	unsigned char indexSlewRate = 0;

	valueSlewRate = convertStringToInt(strSlewRate);
	switch (valueSlewRate) {
		case  60: indexSlewRate = SLEW_060; break;
		case 120: indexSlewRate = SLEW_120; break;
		case 200: indexSlewRate = SLEW_200; break;
		case 300: indexSlewRate = SLEW_300; break;
		case 500: indexSlewRate = SLEW_500; break;
		default:
			printf("  [ERROR] Invalid slew rate: %d\n", valueSlewRate);
			return 0;
	}

	initTxBuffer();
	txBuffer[1] = CMD_SET_SLEW_RATE;
	txBuffer[2] = indexSlewRate;
	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set Slew Rate [%s] OK\n", STR_SLEW_RATE[indexSlewRate-1]);
	else
		printf("  [ERROR] Set Slew rate [%s]\n", STR_SLEW_RATE[indexSlewRate-1]);

	return rc;
}

#if 0		// mskim, 21/03/08
/**
 * @brief  set LED status
 * @note
 * @param  *strStat: LED status string
 * @retval
 */
int setLEDStatus(char *strLED)
{
	int i;
	int rc;
	char strLEDStat[SIZE_ARGUMENT_MAX] = {0,};

	if (strlen(strLED) != 8) {
		printf("  Invalid parameter: %s\n", strLED);
		return 0;
	}

	convertToUpperCase(strLED, strLEDStat);
	initTxBuffer();
	txBuffer[1] = CMD_SET_LED;
	for (i=0; i<8; i++) {
		switch (strLEDStat[i]) {
			case 'P': txBuffer[2] |=  (1 << i); break;
			case 'F': txBuffer[4] |=  (1 << i); break;
		}
	}
	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set LED control %s Ok!\n\n", strLEDStat);
	else
		printf("  Set LED control %s Error!\n\n", strLEDStat);

	return rc;
}
#endif

/**
 * @brief
 * @note
 * @param  *strStat:
 * @param  *status:
 * @retval
 */
int convertLedStatusString(char *strStat, unsigned char *status)
{
	int i;
	int len;
	char strLEDUpperCase[64];

	len = strlen(strStat);
	convertToUpperCase(strStat, strLEDUpperCase);

	if (len != 8) {
		printf("  [ERROR] LED Status number is not 8 Characters\n");
		return 0;
	}

	for (i=0; i<8; i++) {
		if (strLEDUpperCase[i] == 'B') {
			status[i] = 0;
		}
		else if (strLEDUpperCase[i] == 'P') {
			status[i] = 1;
		}
		else if (strLEDUpperCase[i] == 'F') {
			status[i] = 2;
		}
	}

	return 1;
}

/**
 * @brief  set LED status
 * @note
 * @param  *strStat: LED status string
 * @retval
 */
int setLEDStatus(char *strLED)
{
	int rc;
	int index = 0;
	unsigned char ledStatus[8] = {0,};

	rc = convertLedStatusString(strLED, ledStatus);
	if (rc == 0) {
		// printf("  [LED] Status String Fault ERROR.\n");
		return rc;
	}

	for (index = 0; index < 2; index++) {
		initTxBuffer();
		txBuffer[1] = CMD_SET_LED;
		txBuffer[2] = (unsigned char)index;
		txBuffer[3] = ledStatus[index * 4];
		txBuffer[4] = ledStatus[index * 4 + 1];
		txBuffer[5] = ledStatus[index * 4 + 2];
		txBuffer[6] = ledStatus[index * 4 + 3];
		rc = sendAndReceiveCommand(COMM_PORT);
		if (rc == 0) {
			printf("  [ERROR] Set LED Communication ERROR(%d)\n", index);
		}
		else {
			delay(100);
		}
	}

	if (rc)
		printf("  Set LED Status %s Ok\n", strLED);
	else
		printf("  [ERROR] Set LED Status %s\n", strLED);

	return rc;
}

/**
 * @brief  get Output Voltage
 * @note
 * @param  strVrChannel: Voltage Channel string
 * @retval true/false
 */
int getOutputVoltage(char *strVrChannel)
{
	int rc;
	int volt = 0;
	int channelVR = 0;

	channelVR = convertStringToInt(strVrChannel);
	if (channelVR == 0 || channelVR > 5) {
		printf("  [ERROR] Get V-OUT Channel number\n");
		return 0;
	}

	initTxBuffer();
	txBuffer[1] = CMD_GET_CUR_VOLTAGE;
	txBuffer[2] = (unsigned char)channelVR;
	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc) {
		volt = rxBuffer[3] | ((rxBuffer[4] << 8) & 0xFF00);
		printf("  Get %s voltage: %d (mV)\n", strVoltCh[channelVR-1], volt);
	}
	else
		printf("  [ERROR] Get %s voltage Error\n", strVoltCh[channelVR-1]);

	return rc;
}

/**
 * @brief  get Boot Voltage
 * @note
 * @param  strVrChannel: Voltage Channel string
 * @retval
 */
int getBootVoltage(char *strVrChannel)
{
	int rc;
	int volt = 0;
	int channelVR = 0;

	channelVR = convertStringToInt(strVrChannel);
	if (channelVR == 0 || channelVR > 5) {
		printf("  [ERROR] Invalid BOOT-V Channel number.\n");
		return 0;
	}

	initTxBuffer();
	txBuffer[1] = CMD_GET_BOOT_VOLTAGE;
	txBuffer[2] = (unsigned char)channelVR;
	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc) {
		volt = rxBuffer[3] | ((rxBuffer[4] << 8) & 0xFF00);
		printf("  Get %s Boot voltage : %d (mV)\n", strVoltCh[channelVR-1], volt);
	}
	else
		printf("  [ERROR] Get %s Boot voltage\n", strVoltCh[channelVR-1]);

	return rc;
}

/**
 * @brief  set Output Voltage
 * @note
 * @param  strVrChannel: VR Channel
 * @param  strVoltage: Voltage
 * @retval
 */
int setOutputVoltage(char *strVrChannel, char *strVoltage)
{
	int rc;
	int channelVR = 0;
	int valueVoltage = 0;

	channelVR = convertStringToInt(strVrChannel);
	valueVoltage = convertStringToInt(strVoltage);
	if (channelVR >= 1 && channelVR <= 4) {
		if (valueVoltage < MIN_VDD || valueVoltage > MAX_VDD) {
			printf("  [ERROR] Set V-OUT invalid VDD voltage : %d (mV)\n", valueVoltage);
			return 0;
		}
	}
	else if (channelVR == 5) {
		if (valueVoltage < MIN_P3_3V || valueVoltage > MAX_P3_3V) {
			printf("  [ERROR] Set V-OUT invalid P3.3V voltage : %d (mV)\n", valueVoltage);
			return 0;
		}
	}
	else {
		printf("  [ERROR] Set V-OUT invalid voltage channel : %d\n", channelVR);
		return 0;
	}

	initTxBuffer();
	txBuffer[1] = CMD_SET_OUT_VOLTAGE;
	txBuffer[2] = (unsigned char)channelVR;
	txBuffer[3] = (unsigned char)valueVoltage;
	txBuffer[4] = (unsigned char)(valueVoltage >> 8);

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set %s voltage : %d (mV) OK\n", strVoltCh[channelVR-1], valueVoltage);
	else
		printf("  [ERROR] Set %s voltage : %d (mV)\n", strVoltCh[channelVR-1], valueVoltage);

	return rc;
}

/**
 * @brief  set Boot Voltage
 * @note
 * @param  strVrChannel: VR Channel
 * @param  strBootVoltage: Voltage
 * @retval
 */
int setBootVoltage(char *strVrChannel, char *strBootVoltage)
{
	int rc;
	int channelVR = 0;
	int valueBootVoltage = 0;

	channelVR = convertStringToInt(strVrChannel);
	valueBootVoltage = convertStringToInt(strBootVoltage);
	if (channelVR >= 1 && channelVR <= 4) {
		if ((valueBootVoltage != 0) && (valueBootVoltage < MIN_VDD || valueBootVoltage > MAX_VDD)) {
			printf("  [ERROR] Set BOOT-V invalid VDD boot voltage : %d (mV)\n", valueBootVoltage);
			return 0;
		}
	}
	else if (channelVR == 5) {
		if ((valueBootVoltage != 0) && (valueBootVoltage < MIN_P3_3V || valueBootVoltage > MAX_P3_3V)) {
			printf("  [ERROR] Set BOOT-V invalid P3.3V boot voltage : %d (mV)\n", valueBootVoltage);
			return 0;
		}
	}
	else {
		printf("  [ERROR] Set BOOT-V invalid voltage channel : %d\n", channelVR);
		return 0;
	}

	initTxBuffer();
	txBuffer[1] = CMD_SET_BOOT_VOLTAGE;
	txBuffer[2] = (unsigned char)channelVR;
	txBuffer[3] = (unsigned char)valueBootVoltage;
	txBuffer[4] = (unsigned char)(valueBootVoltage >> 8);

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set %s boot voltage : %d (mV) OK\n", strVoltCh[channelVR-1], valueBootVoltage);
	else
		printf("  [ERROR] Set %s boot voltage : %d (mV)\n", strVoltCh[channelVR-1], valueBootVoltage);

	return rc;
}

/**
 * @brief  set tuning voltage
 * @note
 * @param  strVrChannel: voltage channel
 * @param  strTuning: tuning value string
 * @retval true/false
 */
int setVoltageTuningValue(char *strVrChannel, char *strTuning)
{
	int rc;
	int channelVR;
	short valueTuning = 0;

	channelVR = (int)convertStringToInt(strVrChannel);
	if (channelVR == 0 || channelVR > 5) {
		printf("  [ERROR] Set V-TUNE invalid voltage channel : %d\n", channelVR);
		return 0;
	}

	// decimal string을 convertStringToInt함수를 통해 short로 잘 변환되는지 확인
	valueTuning = (short)convertStringToInt(strTuning);
	// printf("TUNING_VALUE = %d\n", valueTuning);

	// tuning value에 대한 limit를 check 하는 부분을 추가 할 것

	initTxBuffer();
	txBuffer[1] = CMD_SET_VOLTAGE_TUNE;
	txBuffer[2] = (unsigned char)channelVR;
	txBuffer[3] = (unsigned char)valueTuning;
	txBuffer[4] = (unsigned char)(valueTuning >> 8);

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set %s V-TUNE value %d (mV) OK\n", strVoltCh[channelVR-1], valueTuning);
	else
		printf("  [ERROR] Set %s V-TUNE value %d (mV)\n", strVoltCh[channelVR-1], valueTuning);

	return rc;
}

/**
 * @brief
 * @note
 * @param  *strFanChannel:
 * @param  *strFanSpeed:
 * @retval
 */
int setFanSpeed(char *strFanChannel, char *strFanSpeed)
{
	int rc = 0;
	int fanChannel = 0;
	int fanSpeed = 0;

	fanChannel = (int)convertStringToInt(strFanChannel);
	fanSpeed = (int)convertStringToInt(strFanSpeed);
	if (fanChannel == 0 || fanChannel > 3) {
		printf("  [ERROR] Set F-SPD invalid FAN Channel : %s\n", strFanChannel);
		return rc;
	}
	if (fanSpeed < 3 || fanSpeed > 10) {
		printf("  [ERROR] Set F-SPD invalid FAN Speed : %s\n", strFanSpeed);
		return rc;
	}

	initTxBuffer();
	txBuffer[1] = CMD_SET_PWM_PULSE_WIDTH;
	txBuffer[2] = (unsigned char)fanChannel;
	txBuffer[3] = (unsigned char)fanSpeed;

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set FAN %d Spped %d OK\n", fanChannel, fanSpeed);
	else
		printf("  [ERROR] Set FAN %d Spped %d\n", fanChannel, fanSpeed);

	return rc;
}

#if 0		// mskim 21/05/04
/**
 * @brief  set FND Display Tuning
 * @note
 * @retval true/false
 */
int setFNDTuning(void)
{
	int rc;

	initTxBuffer();
	txBuffer[1] = CMD_SET_ADC_TUNE;

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc) {
		printf("  Set FND Display Tuning OK!");
	}
	else
		printf("  Set FND Display Tuning Error!");

	return rc;
}
#endif

/**
 * @brief  /ID command function
 * @note
 * @retval
 */
int getIDFunc(void)
{
	int retFunc;
	FILE *fp;

	if( (fp = fopen("VOLTDEV.TXT", "w")) == NULL ) {
		printf("  [ERROR] VOLTDEV.TXT open error\n");
		return 0;
	}

	fprintf(fp, "VC S/W VERSION=%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);

	retFunc = getFirmwareVersion();
	fprintf(fp, "VC F/W VERSION=%d.%d.%d\n", rxBuffer[3], rxBuffer[4], rxBuffer[5]);
	fclose(fp);

	retFunc = setLEDStatus("BBBBBBBB");
	if( (fp = fopen("SPCLED.TXT", "w")) == NULL ) {
		printf("  [ERROR] SPCLED.TXT open error\n");
		return 0;
	}

	fprintf(fp, "SLOT1 B\n");
	fprintf(fp, "SLOT2 B\n");
	fprintf(fp, "SLOT3 B\n");
	fprintf(fp, "SLOT4 B\n");
	fprintf(fp, "SLOT5 B\n");
	fprintf(fp, "SLOT6 B\n");
	fprintf(fp, "SLOT7 B\n");
	fprintf(fp, "SLOT8 B\n");

	printf("  Set CPX_VC -SL BBBBBBBB and Create VOLTDEV.TXT & SPCLED.TXT Ok!\n\n");
	fclose(fp);

	return 1;
}

#if 0		// mskim 21/05/04
/**
 * @brief  send BEEP Test Command
 * @note
 * @retval true/false
 */
int setBeepTest(void)
{
	int rc;

	initTxBuffer();						// Make default TX buffer
	txBuffer[1] = CMD_SEND_BEEP_TEST;

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc) {
		printf("  Set Beep Test Ok!\n\n");
	}
	else
		printf("  Set Beep Test Error!\n\n");

	return rc;
}
#endif

/**
 * @brief  get memory slot count value
 * @note
 * @param  strSlotNumber: slot number, 1 ~ 6
 * @retval TRUE/FALSE
 */
int getSlotCount(char *strSlotNumber)
{
	int rc;
	int count_val = 0;
	int slot_idx = 1;
	int numberSlot = 0;

	numberSlot = convertStringToInt(strSlotNumber);
	if (numberSlot > 8) {
		printf("  [ERROR] Invalid socket number : %d\n", numberSlot);
		return 0;
	}

	if (numberSlot == 0) {
		for (slot_idx = 1; slot_idx<=8; slot_idx++) {
			initTxBuffer();
			txBuffer[1] = CMD_GET_MEM_COUNT;
			txBuffer[2] = (unsigned char)slot_idx;
			rc = sendAndReceiveCommand(COMM_PORT);
			if (rc) {
				count_val = rxBuffer[3] | (rxBuffer[4] << 8) | (rxBuffer[5] << 16) | (rxBuffer[6] << 24);
				printf("  Get Memory Socket %d Count : %d OK\n", slot_idx, count_val);
			}
			else {
				printf("  [ERROR] Get Memory Socket %d Count\n", slot_idx);
				break;
			}
			if (slot_idx == 8)
				printf("\n");
			delay(500);
		}
	}
	else {
		initTxBuffer();
		txBuffer[1] = CMD_GET_MEM_COUNT;
		txBuffer[2] = (unsigned char)numberSlot;
		rc = sendAndReceiveCommand(COMM_PORT);
		if (rc) {
			count_val = rxBuffer[3] | (rxBuffer[4] << 8) | (rxBuffer[5] << 16) | (rxBuffer[6] << 24);
			printf("  Get Memory Socket %d Count : %d OK\n", numberSlot, count_val);
		}
		else
			printf("  [ERROR] Get Memory Socket %d Count\n", numberSlot);
	}

	return rc;
}

#if 0		// mskim, 21/03/09
/**
 * @brief  set slot count action
 * @note
 * @param  *strSlotAction: action command string
 * @retval TRUE/FALSE
 */
int setSlotCountAction(char *strSlotAction)
{
	int rc;
	int i;
	char strMemCountAction[SIZE_ARGUMENT_MAX] = {0,};

	if (strlen(strSlotAction) != 8) {
		printf("  Invalid parameter: %s\n", strSlotAction);
		return 0;
	}

	convertToUpperCase(strSlotAction, strMemCountAction);
	initTxBuffer();
	txBuffer[1] = CMD_SET_MEM_COUNT_ACTION;
	for (i=0; i<8; i++) {
		if (strMemCountAction[i] == '+') {
			txBuffer[2] |= (1 << i);
		}
		else if (strMemCountAction[i] == 'R') {
			txBuffer[4] |= (1 << i);
		}
	}

	rc = sendAndReceiveCommand(COMM_PORT);
	if (rc)
		printf("  Set Memory Socket Count Action %s Ok!\n\n", strSlotAction);
	else
		printf("  Set Memory Socket Count Action Error!\n\n");

	return rc;
}
#endif

/**
 * @brief
 * @note
 * @param  *strStat:
 * @param  *status:
 * @retval
 */
int convertMemorySlotActionString(char *strStat, unsigned char *status)
{
	int i;
	int len;
	char strMemActionUpperCase[64];

	len = strlen(strStat);
	convertToUpperCase(strStat, strMemActionUpperCase);

	if (len != 8) {
		printf("  [ERROR] Set SLOT-ACT Status String is not 8 characters\n");
		return 0;
	}

	for (i=0; i<8; i++) {
		if (strMemActionUpperCase[i] == 'X') {
			status[i] = 0;		// No Action
		}
		else if (strMemActionUpperCase[i] == '+') {
			status[i] = 1;		// Increase
		}
		else if (strMemActionUpperCase[i] == 'R') {
			status[i] = 2;		// Reset
		}
	}

	return 1;
}

/**
 * @brief  set slot count action
 * @note
 * @param  *strSlotAction: action command string
 * @retval TRUE/FALSE
 */
int setSlotCountAction(char *strSlotAction)
{
	int rc;
	int index;
	unsigned char slotStatus[8]	= {0,};

	rc = convertMemorySlotActionString(strSlotAction, slotStatus);
	if (rc == 0) {
		// printf("  [ERROR] SLOT String Convert Error.\n");
		return 0;
	}

	for (index = 0; index < 2; index++) {
		initTxBuffer();
		txBuffer[1] = CMD_SET_MEM_COUNT_ACTION;
		txBuffer[2] = (unsigned char)index;
		txBuffer[3] = slotStatus[index * 4];
		txBuffer[4] = slotStatus[index * 4 + 1];
		txBuffer[5] = slotStatus[index * 4 + 2];
		txBuffer[6] = slotStatus[index * 4 + 3];
		rc = sendAndReceiveCommand(COMM_PORT);
		if (rc == 0) {
			printf("  [ERROR] Memory Slot Action Communication ERROR : %d\n", index);
			return 0;
		}
		else {
			delay(100);
		}
	}

	if (rc)
		printf("  Set Memory Socket Count Action %s OK\n", strSlotAction);
	else
		printf("  [ERROR] Set Memory Socket Count Action\n");

	return rc;
}

/**
 * @brief  check RX data validity
 * @note
 * @retval
 */
int checkRxDataValidity(int rxLen)
{
	unsigned char retCommand;
	unsigned char error = 0;
	// int i;

	retCommand = txBuffer[1] & 0xBF;
	retCommand |= 0x80;

	if (rxBuffer[0] != STX) {
		// printf("  Command Start Error [%02X]\n", rxBuffer[0]);
		error = 1;
	}

	if (error == 0) {
		if (rxBuffer[SIZE_CMD_PACKET - 1] != ETX) {
			// printf("  Command End Error [%02X]\n", rxBuffer[SIZE_CMD_PACKET - 1]);
			error = 1;
		}
	}

	if (error == 0) {
		if (rxBuffer[1] != retCommand) {
			// printf("  Return Command Error [%02X]\n", rxBuffer[1]);
			error = 1;
		}
	}

	if (error == 0) {
		if (rxBuffer[1] >= 0xE0) {
			// printf("  Error Code Returned [%02X]\n", rxBuffer[1]);
			error = 1;
		}
	}

	if (error) {
		// printf("    RX : ");
		// for (i=0; i<SIZE_CMD_PACKET; i++) {
		// 	printf("%02X ", rxBuffer[i]);
		// }
		// printf("\n");
		return 0;
	}

	return 1;
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
		sendUartBuffer(port, txBuffer, SIZE_CMD_PACKET);
		delay(150);

		initRxBuffer();
		rxCnt = receiveUartBuffer(port, SIZE_CMD_PACKET, 1000);
		if (rxCnt < SIZE_CMD_PACKET) {
			printf("  [ERROR] Receive Size Error. nReceiveData: %d\n", rxCnt);
		}

		if (checkRxDataValidity(rxCnt) == 1) {
			rc = 1;
			break;
		}

		printf("  Retry [%d]\n", i);
		delay(500);
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
	//--- Make default send buffer
	txBuffer[0] = STX;
	txBuffer[1] = 0;
	txBuffer[2] = 0;
	txBuffer[3] = 0;
	txBuffer[4] = 0;
	txBuffer[5] = 0;
	txBuffer[6] = 0;
	txBuffer[7] = ETX;
}

/**
 * @brief  initialize receive buffer
 * @note
 * @retval None
 */
void initRxBuffer(void)
{
	rxBuffer[0] = 0;
	rxBuffer[1] = 0;
	rxBuffer[2] = 0;
	rxBuffer[3] = 0;
	rxBuffer[4] = 0;
	rxBuffer[5] = 0;
	rxBuffer[6] = 0;
	rxBuffer[7] = 0;
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
			rxBuffer[rxCnt] = inportb(port);
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
		if (c & 1) {				// If so, then get Char
			ch = inportb(port);
			c = ch;					// Dummy code for delete warning
			break;
		}
		delay(1);					// Delay 1ms
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
 * @brief  convert decimal string to int value
 * @note
 * @param  *str: decimal string
 * @retval converted value
 */
int convertStringToHex(char const *str)
{
	int val = 0;

	if (str == NULL) return 0;

	while (*str) {
		if (*str >= '0' && *str <= '9') {
			val = (val * 0x10) + (*str) - '0';
		}
		else if (*str >= 'a' && *str <= 'f') {
			val = (val * 0x10) + (*str) - 'a' + 0x0A;
		}
		else if (*str >= 'A' && *str <= 'F') {
			val = (val * 0x10) + (*str) - 'A' + 0x0A;
		}
		str++;
	}

	return val;
}

/**
 * @brief  main()
 * @note
 * @param  argc:
 * @param  **argv:
 * @retval
 */
int	main(int argc, char **argv)
{
	int rc = 0;
	char optionCmd[SIZE_ARGUMENT_MAX] = {0,};
	#ifdef COMM_PORT_AX99100
	UINT16 vendorID = 0;
	UINT16 devID = 0;
	UINT8 busID;
	#endif

	convertDate();
	printf("Voltage Control Program for PCT3.0-DDR5 V%d.%d.%d [%d.%d.%d]\n",
			VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, year, month, date);

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

	//--- Check argument option ---------------------------------
	if (argc == 1) {
		displayHelpMsg();
		printf("\n");
		exit(1);
		return 0;
	}

	//--- Check Command line option ---------------------------------
	convertToUpperCase(argv[1], optionCmd);
	if (argc == 2) {
		if (!strcmp(optionCmd, "-H")) {
			displayHelpMsg();
			rc = 1;
		}
		else if (!strcmp(optionCmd, "-CC")) {
			rc = checkConnection();
		}
		else if (!strcmp(optionCmd, "-GR")) {
			rc = getFirmwareVersion();
		}
		// else if (!strcmp(optionCmd, "-GS")) {
		// 	rc = getSlewRate();
		// }
		// else if (!strcmp(optionCmd, "-FT")) {
		// 	rc = setFNDTuning();
		// }
		else if (!strcmp(optionCmd, "-ID")) {
			rc = getIDFunc();
		}
		else {
			printf("  [ERROR] %s is not valid option.\n", optionCmd);
		}
	}
	else if (argc == 3) {
		// Set LED Status
		if (!strcmp(optionCmd, "-SL")) {
			rc = setLEDStatus(argv[2]);
		}
		// else if (!strcmp(optionCmd, "-SR")) {
		// 	rc = setSlewRate(argv[2]);
		// }
		// Get Output voltage
		else if (!strcmp(optionCmd, "-GV")) {
			rc = getOutputVoltage(argv[2]);
		}
		// Get Boot voltage
		else if (!strcmp(optionCmd, "-GB")) {
			rc = getBootVoltage(argv[2]);
		}
		// Get Memory Slot Count
		else if (!strcmp(optionCmd, "-GC")) {
			rc = getSlotCount(argv[2]);
		}
		// Set Memory Slot Count Action
		else if (!strcmp(optionCmd, "-SC")) {
			rc = setSlotCountAction(argv[2]);
		}
		else if (!strcmp(optionCmd, "-GF")) {
			rc = getFanRPM(argv[2]);
		}
		else if (!strcmp(optionCmd, "-P80")) {
			rc = setPort80Data(argv[2]);
		}
		#ifdef SPR_CXM_MODEL_SUPPORT
		else if (!strcmp(optionCmd, "-CR")) {
			rc = setReqColdReset(argv[2]);
		}
		else if (!strcmp(optionCmd, "-CP")) {
			rc = setCXLPwrControl(argv[2]);
		}
		#endif
		else {
			printf("  [ERROR] %s is not valid option.\n", optionCmd);
		}
	}
	else if (argc == 4) {
		// Set Output Voltage
		if (!strcmp(optionCmd, "-SV")) {
			rc = setOutputVoltage(argv[2], argv[3]);
		}
		// Set Boot Voltage
		else if (!strcmp(optionCmd, "-BV")) {
			rc = setBootVoltage(argv[2], argv[3]);
		}
		// Set Tuning voltage
		// else if (!strcmp(optionCmd, "-ST")) {
		// 	rc = setVoltageTuningValue(argv[2], argv[3]);
		// }
		// Set Fan Speed
		else if (!strcmp(optionCmd, "-FS")) {
		 	rc = setFanSpeed(argv[2], argv[3]);
		}
		else {
			printf("  [ERROR] %s is not valid option.\n", optionCmd);
		}
	}
	#ifdef SPR_CXM_MODEL_SUPPORT
	else if (argc == 5) {
		if (!strcmp(optionCmd, "-IO")) {
			rc = setIOExpanderOutput(argv[2], argv[3], argv[4]);
		}
	}
	#endif
	else {
		displayHelpMsg();
		rc = 1;
	}

	printf("\n");

	if (rc)
		exit(0);
	else
		exit(1);

	return 0;
}