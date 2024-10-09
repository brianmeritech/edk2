
#ifndef VC_SR_H_
#define VC_SR_H_

/*******************************************************************************
 *	Date		Version		Comment
 *	24/10/09	V1.0.0		Initial Version
 ******************************************************************************/
#define	VERSION_MAJOR						        1			// Major version
#define	VERSION_MINOR						        0			// Minor version 1
#define	VERSION_BUILD						        0			// Minor version 2

#define MAX_TRY_CNT							        5			// Maximum retry count for All command
#define	SIZE_ARGUMENT_MAX				        16		// Maximum command string count

#define CPU_FAN_RPM_MONITORING
#define COM_PORT_BMC_2F8                0x2F8

#define	RECV_BUFF_SIZE						      1024
#define	SIZE_CMD_PACKET						      8
#define STX									            0x02
#define ETX									            0x03

// --- Command Definition
#define CMD_CHECK_CONNECTION			      0x41
#define CMD_PORT80_DATA						      0x42
#define CMD_GET_FW_VERSION				      0x51
#define CMD_GET_CUR_VOLTAGE				      0x52
#define CMD_GET_SLEW_RATE					      0x53
#define CMD_GET_BOOT_VOLTAGE			      0x54
#define CMD_GET_MEM_COUNT					      0x55
#define CMD_GET_FAN_RPM						      0x56
#define CMD_SET_OUT_VOLTAGE				      0x61
#define CMD_SET_SLEW_RATE					      0x62
#define CMD_SET_BOOT_VOLTAGE			      0x63
#define CMD_SET_LED							        0x64
#define CMD_SET_MEM_COUNT_ACTION	      0x65
#define CMD_SET_VOLTAGE_TUNE			      0x66
#define CMD_SET_ADC_TUNE					      0x67
#define CMD_SET_PWM_PULSE_WIDTH		      0x68

//--- Define VDD, VPP MIN/MAX voltages
#define	MIN_VDD								          4250	// Min. VDD volt = 4.25V
#define	MAX_VDD								          15000	// Max. VDD volt = 15.00V
#define MIN_P3_3V							          3000	// Min. P3.3 volt = 3.0V
#define MAX_P3_3V							          3600	// Max. P3.3 volt = 3.6V

#define AST2600_CONFIG_INDEX			      0x2E
#define AST2600_CONFIG_DATA				      0x2F

#define COM1_PORT_ADDR					        0x03F8
#define COM2_PORT_ADDR					        0x02F8

#define AST2600_LDN_UART1				        0x02
#define AST2600_LDN_UART2				        0x03

#define AST2600_LDN_SEL_REGISTER		    0x07
#define AST2600_ACTIVATE_REGISTER		    0x30
#define AST2600_BASE1_HI_REGISTER		    0x60
#define AST2600_BASE1_LO_REGISTER		    0x61
#define AST2600_IRQ1_REGISTER			      0x70
#define AST2600_IRQ1TYPE_REGISTER		    0x71

#define AST2600_ACTIVATE_VALUE			    0x01
#define AST2600_DEACTIVATE_VALUE		    0x00
#define AST2600_CONFIG_MODE_ENTER_VALUE	0xA5
#define AST2600_CONFIG_MODE_EXIT_VALUE	0xAA
















#endif
