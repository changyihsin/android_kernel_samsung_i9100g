/***************************************************************************
* 
*   SiI9244 ? MHL Transmitter Driver
*
* Copyright (C) (2011, Silicon Image Inc)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2.
*
* This program is distributed “as is” WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*****************************************************************************/
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
//#include <plat/pm.h>
#include <asm/irq.h>
#include <linux/delay.h>
//#include <plat/gpio-cfg.h>
//#include <mach/regs-gpio.h>
//#include <mach/regs-clock.h>

#include <linux/syscalls.h> 
#include <linux/fcntl.h> 
#include <asm/uaccess.h> 
#include <linux/types.h>

#include "Common_Def.h"
#include "si_apiCbus.h"

//extern void rcp_cbus_uevent(void);	//MHL v1 //NAGSM_Android_SEL_Kernel_Aakash_20101126

#if (IS_CBUS == 1)
//------------------------------------------------------------------------------
// Module variables
//------------------------------------------------------------------------------

typedef struct
{
	byte rcpKeyCode;          //!< RCP CBUS Key Code
	char   rcpName[30];
} SI_Rc5RcpConversion_t;

SI_Rc5RcpConversion_t RcpSourceToSink[] =
{
	{ MHD_RCP_CMD_SELECT,			"Select"},
	{ MHD_RCP_CMD_UP,				"Up"},
	{ MHD_RCP_CMD_DOWN,				"Down"},
	{ MHD_RCP_CMD_LEFT,				"Left"},
	{ MHD_RCP_CMD_RIGHT, 			"Right"},
	//{ MHD_RCP_CMD_RIGHT_UP,		"Right Up"},
	//{ MHD_RCP_CMD_RIGHT_DOWN,		"Right Down"},
	//{ MHD_RCP_CMD_LEFT_UP,		"Left Up"},
	//{ MHD_RCP_CMD_LEFT_DOWN,		"Left Down"},
	{ MHD_RCP_CMD_ROOT_MENU,		"Root Menu"},
	//{ MHD_RCP_CMD_SETUP_MENU,		"Setup Menu"},
	//{ MHD_RCP_CMD_CONTENTS_MENU,	"Contents Menu"},
	//{ MHD_RCP_CMD_FAVORITE_MENU,	"Favorite Menu"},
	{ MHD_RCP_CMD_EXIT,				"Exit"},
	// 0x0E - 0x1F Reserved 
	{ MHD_RCP_CMD_NUM_0,			"Num 0"},
	{ MHD_RCP_CMD_NUM_1,			"Num 1"},
	{ MHD_RCP_CMD_NUM_2,			"Num 2"},
	{ MHD_RCP_CMD_NUM_3,			"Num 3"},
	{ MHD_RCP_CMD_NUM_4,			"Num 4"},
	{ MHD_RCP_CMD_NUM_5,			"Num 5"},
	{ MHD_RCP_CMD_NUM_6,			"Num 6"},
	{ MHD_RCP_CMD_NUM_7,			"Num 7"},
	{ MHD_RCP_CMD_NUM_8,			"Num 8"},
	{ MHD_RCP_CMD_NUM_9,			"Num 9"},
	//{ MHD_RCP_CMD_DOT,				"Dot"},
	{ MHD_RCP_CMD_ENTER,			"Enter"},
	{ MHD_RCP_CMD_CLEAR,			"Clear"},
	// 0x2D - 0x2F Reserved 
	{ MHD_RCP_CMD_CH_UP,			"Channel Up"},
	{ MHD_RCP_CMD_CH_DOWN,			"Channel Down"},
	{ MHD_RCP_CMD_PRE_CH,			"Previous Channel"},
	{ MHD_RCP_CMD_SOUND_SELECT,		"Sound Select"},
	//{ MHD_RCP_CMD_INPUT_SELECT,	"Input Select"},
	//{ MHD_RCP_CMD_SHOW_INFO,		"Show Info"},
	//{ MHD_RCP_CMD_HELP,			"Help"},
	//{ MHD_RCP_CMD_PAGE_UP,		"Page Up"},
	//{ MHD_RCP_CMD_PAGE_DOWN,		"Page Down"},
	// 0x39 - 0x40 Reserved 
	{ MHD_RCP_CMD_VOL_UP,			"Volume Up"},
	{ MHD_RCP_CMD_VOL_DOWN,			"Volume Down"},
	{ MHD_RCP_CMD_MUTE,				"Mute"},
	{ MHD_RCP_CMD_PLAY,				"Play"},
	{ MHD_RCP_CMD_STOP,			 	"Stop"},
	{ MHD_RCP_CMD_PAUSE,			"Pause"},
	{ MHD_RCP_CMD_RECORD,			"Record"},
	{ MHD_RCP_CMD_REWIND,			"Rewind"},
	{ MHD_RCP_CMD_FAST_FWD,			"Fast Fwd"},
	{ MHD_RCP_CMD_EJECT,			"Eject"},
	{ MHD_RCP_CMD_FWD,				"Forward"},
	{ MHD_RCP_CMD_BKWD,				"Backward"},
	// 0X4D - 0x4F Reserved
	//{ MHD_RCP_CMD_ANGLE,			"Angle"},
	//{ MHD_RCP_CMD_SUBPICTURE,		"Subpicture"},
	// 0x52 - 0x5F Reserved
	{ MHD_RCP_CMD_PLAY_FUNC,		"Play Function"},
	{ MHD_RCP_CMD_PAUSE_PLAY_FUNC,	"Pause Play Function"},
	{ MHD_RCP_CMD_RECORD_FUNC,		"Record Function"},
	{ MHD_RCP_CMD_PAUSE_REC_FUNC,	"Pause Record Function"},
	{ MHD_RCP_CMD_STOP_FUNC,		"Stop Function"},
	{ MHD_RCP_CMD_MUTE_FUNC,		"Mute Function"},
	{ MHD_RCP_CMD_UN_MUTE_FUNC,		"Un-Mute Function"},
	//{ MHD_RCP_CMD_TUNE_FUNC,		"Tune Function"},
	//{ MHD_RCP_CMD_MEDIA_FUNC,		"Media Function"},
	// 0x69 - 0x70 Reserved
	//{ MHD_RCP_CMD_F1,				"F1"},
	//{ MHD_RCP_CMD_F2,				"F2"},
	//{ MHD_RCP_CMD_F3,				"F3"},
	//{ MHD_RCP_CMD_F4,				"F4"},
	//{ MHD_RCP_CMD_F5,				"F5"},
	// 0x76 - 0x7D Reserved
	//{ MHD_RCP_CMD_VS,				"Vendor Specific"}
	// 0x7F Reserved
};

//------------------------------------------------------------------------------
// Function:    CbusRc5toRcpConvert
// Description: Translate RC5 command to CBUS RCP command
// Parameters:  keyData: key code from the remote controller
// Returns:     RCP code equivalent to passed RC5 key code or 0xFF if not in list.
//------------------------------------------------------------------------------

static byte CbusRc5toRcpConvert ( byte keyCode )
{
	byte i;
	byte retVal = 0xFF;
	byte length = sizeof(RcpSourceToSink)/sizeof(SI_Rc5RcpConversion_t);

	for ( i = 0; i < length ; i++ )
	{
		if ( keyCode == RcpSourceToSink[i].rcpKeyCode )
		{
			retVal = RcpSourceToSink[i].rcpKeyCode;
#ifdef RCP_DEBUG
			printk("CPCBUS:: Send ----> %s\n",
				RcpSourceToSink[i].rcpName );
#endif
			break;
		}
	}

	/* Return the new code or 0xFF if not found.    */

	return( ( i == length ) ? 0xFF : retVal );
}

//------------------------------------------------------------------------------
// Function:    CpCbusSendRcpMessage
// Description: Convert input port number to CBUS channel and send the
//              passed RC5 key code as a CBUS RCP key code.
// Parameters:  port    - Port Processor input port index
//              keyCode - Remote control button code.
// Returns:     true if successful, false if not MHD port or other failure.
//------------------------------------------------------------------------------

Bool CpCbusSendRcpMessage ( byte channel, byte keyCode )
{
	Bool  success;

	success = FALSE;
	for ( ;; )
	{
		printk( "CPCBUS:: Sending RCP Msg:: %02X keycode"
				" to channel %d CBUS\n\n",
				(int)keyCode, (int)channel );
		if (channel == 0xFF) {
			printk( "\n::::::: Bad channel -- " );
			break;
		}				      

		keyCode = CbusRc5toRcpConvert( keyCode );
		if (keyCode == 0xFF) {
			printk( "\n::::::: Bad KeyCode -- " );
			break;
		}

		success = SI_CbusMscMsgSubCmdSend( channel,
						MHD_MSC_MSG_RCP,
						keyCode );
		break;
	}

	if ( !success )
	{
		printk("Unable to send command :::::::\n");
	}

	return success;
}

//------------------------------------------------------------------------------
// Function:    CpCbusSendRapMessage
// Parameters:  port    - Port Processor input port index
//              actionCode - Action code.
// Returns:     true if successful, false if not MHD port or other failure.
//------------------------------------------------------------------------------

Bool CpCbusSendRapMessage ( byte channel, byte actCode )
{
	Bool  success;

	success = FALSE;
	for ( ;; )
	{
		printk( "CPCBUS:: Sending RAP Msg:: %02X action code"
			" to channel %d CBUS\n\n",
			(int)actCode, (int)channel );

		if ( channel == 0xFF) {
			printk("\n::::::: Bad channel -- ");
			break;
		}

		if ((actCode == MHD_RAP_CMD_POLL) ||
		(actCode == MHD_RAP_CMD_CHG_QUIET) ||
		(actCode != MHD_RAP_CMD_CHG_ACTIVE_PWR)) {
			success = SI_CbusMscMsgSubCmdSend(channel,
						MHD_MSC_MSG_RAP,
						actCode );
			break;
		} else {
			printk("\n::::::: Bad action code -- ");
			break;
		}
	}

	if (!success) {
		printk( "Unable to send action command :::::::\n" );
	}

	return success;
}

//------------------------------------------------------------------------------
// Function:    CpProcessRcpMessage
// Description: Process the passed RCP message.
// Returns:     The RCPK status code.
//------------------------------------------------------------------------------

static byte CpProcessRcpMessage ( byte channel, byte rcpData )
{
	byte rcpkStatus = MHD_MSC_MSG_NO_ERROR;

	printk("RCP Key Code: 0x%02X, channel: 0x%02X\n",
			(int)rcpData, (int)channel );

	switch (rcpData) {
	case MHD_RCP_CMD_SELECT:
		printk("\nSelect received\n\n");
		break;
	case MHD_RCP_CMD_UP:
		printk("\nUp received\n\n");
		break;
	case MHD_RCP_CMD_DOWN:
		printk("\nDown received\n\n");
		break;
	case MHD_RCP_CMD_LEFT:
		printk("\nLeft received\n\n");
		break;
	case MHD_RCP_CMD_RIGHT:
		printk("\nRight received\n\n");
		break;
	case MHD_RCP_CMD_RIGHT_UP:
		printk("\n MHD_RCP_CMD_RIGHT_UP\n\n");
		break;
	case MHD_RCP_CMD_RIGHT_DOWN:
		printk("\n MHD_RCP_CMD_RIGHT_DOWN \n\n");
		break;
	case MHD_RCP_CMD_LEFT_UP:
		printk("\n MHD_RCP_CMD_LEFT_UP\n\n");
		break;
	case MHD_RCP_CMD_LEFT_DOWN:
		printk("\n MHD_RCP_CMD_LEFT_DOWN\n\n");
		break;      
	case MHD_RCP_CMD_ROOT_MENU:
		printk("\nRoot Menu received\n\n");
		break;
	case MHD_RCP_CMD_SETUP_MENU:
		printk("\n MHD_RCP_CMD_SETUP_MENU\n\n");
		break;      

	case MHD_RCP_CMD_CONTENTS_MENU:
		printk("\n MHD_RCP_CMD_CONTENTS_MENU\n\n");
		break;      

	case MHD_RCP_CMD_FAVORITE_MENU:
		printk("\n MHD_RCP_CMD_FAVORITE_MENU\n\n");
		break;            

	case MHD_RCP_CMD_EXIT:
		printk("\nExit received\n\n");
		break;

	case MHD_RCP_CMD_NUM_0:
		printk("\nNumber 0 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_1:
		printk("\nNumber 1 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_2:
		printk("\nNumber 2 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_3:
		printk("\nNumber 3 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_4:
		printk("\nNumber 4 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_5:
		printk("\nNumber 5 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_6:
		printk("\nNumber 6 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_7:
		printk("\nNumber 7 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_8:
		printk("\nNumber 8 received\n\n");
		break;

	case MHD_RCP_CMD_NUM_9:
		printk("\nNumber 9 received\n\n");
		break;

	case MHD_RCP_CMD_DOT:
		printk("\n MHD_RCP_CMD_DOT\n\n");
		break;          

	case MHD_RCP_CMD_ENTER:
		printk("\nEnter received\n\n");
		break;

	case MHD_RCP_CMD_CLEAR:
		printk("\nClear received\n\n");
		break;

	case MHD_RCP_CMD_CH_UP:
		printk("\n MHD_RCP_CMD_CH_UP\n\n");
		break; 

	case MHD_RCP_CMD_CH_DOWN:
		printk("\n MHD_RCP_CMD_CH_DOWN\n\n");
		break;       

	case MHD_RCP_CMD_PRE_CH:
		printk("\n MHD_RCP_CMD_PRE_CH\n\n");
		break;           

	case MHD_RCP_CMD_SOUND_SELECT:
		printk("\nSound Select received\n\n");
		break;

	case MHD_RCP_CMD_INPUT_SELECT:
		printk("\n MHD_RCP_CMD_INPUT_SELECT\n\n");
		break;    

	case MHD_RCP_CMD_SHOW_INFO:
		printk("\n MHD_RCP_CMD_SHOW_INFO\n\n");
		break;     

	case MHD_RCP_CMD_HELP:
		printk("\n MHD_RCP_CMD_HELP\n\n");
		break;   

	case MHD_RCP_CMD_PAGE_UP:
		printk("\n MHD_RCP_CMD_PAGE_UP\n\n");
		break;  

	case MHD_RCP_CMD_PAGE_DOWN:
		printk("\n MHD_RCP_CMD_PAGE_DOWN\n\n");
		break;             

	case MHD_RCP_CMD_VOL_UP:
		printk("\n MHD_RCP_CMD_VOL_UP\n\n");
		break;             

	case MHD_RCP_CMD_VOL_DOWN:
		printk("\n MHD_RCP_CMD_VOL_DOWN\n\n");
		break;             

	case MHD_RCP_CMD_MUTE:
		printk("\n MHD_RCP_CMD_MUTE\n\n");
		break;             

	case MHD_RCP_CMD_PLAY:
		printk("\nPlay received\n\n");
		break;

	case MHD_RCP_CMD_STOP:
		printk("\n MHD_RCP_CMD_STOP\n\n");
		break;   

	case MHD_RCP_CMD_PAUSE:
		printk("\nPause received\n\n");
		break;

	case MHD_RCP_CMD_RECORD:
		printk("\n MHD_RCP_CMD_RECORD\n\n");
		break;   

	case MHD_RCP_CMD_FAST_FWD:
		printk("\nFastfwd received\n\n");
		break;

	case MHD_RCP_CMD_REWIND:
		printk("\nRewind received\n\n");
		break;

	case MHD_RCP_CMD_EJECT:
		printk("\nEject received\n\n");
		break;

	case MHD_RCP_CMD_FWD:
		printk("\nForward received\n\n");
		break;

	case MHD_RCP_CMD_BKWD:
		printk("\nBackward received\n\n");
		break;

	case MHD_RCP_CMD_PLAY_FUNC:
		printk("\nPlay Function received\n\n");
		break;

	case MHD_RCP_CMD_PAUSE_PLAY_FUNC:
		printk("\nPause_Play Function received\n\n");
		break;

	case MHD_RCP_CMD_STOP_FUNC:
		printk("\nStop Function received\n\n");
		break;

	default:
		rcpkStatus = MHD_MSC_MSG_INEFFECTIVE_KEY_CODE;
		break;
	}

	if ( rcpkStatus == MHD_MSC_MSG_INEFFECTIVE_KEY_CODE ) {
		printk("\nKeyCode not recognized or supported.\n\n");
	}

	return rcpkStatus;
}

//------------------------------------------------------------------------------
// Function:    CpProcessRapMessage
// Description: Process the passed RAP message.
// Returns:     The RAPK status code.
//------------------------------------------------------------------------------

static byte CpProcessRapMessage ( byte channel, byte rcpData )
{
	byte rapkStatus = MHD_MSC_MSG_NO_ERROR;

	printk("RAP Key Code: 0x%02X, channel: 0x%02X\n",
			(int)rcpData, (int)channel );

	switch (rcpData) {
	case MHD_RAP_CMD_POLL:
		printk("\nPOLL received\n\n");
		break;

	case MHD_RAP_CMD_CHG_ACTIVE_PWR:
		printk( "\nCHANGE TO ACTIVE POWER STATE"
				" received\n\n");
		break;        

	case MHD_RAP_CMD_CHG_QUIET:
		printk( "\nCHANGE TO QUIET STATE"
				" received\n\n");
		//TPI_GoToD3();  //???
		break;

	default:
		rapkStatus = MHD_MSC_MSG_RAP_UNRECOGNIZED_ACT_CODE;
		break;
	}

	if ( rapkStatus == MHD_MSC_MSG_RAP_UNRECOGNIZED_ACT_CODE ) {
		printk("\nKeyCode not recognized !! \n\n" );
	}

	return rapkStatus;
}

//------------------------------------------------------------------------------
// Function:    CbusConnectionCheck
// Description: Display any change in CBUS connection state and enable
//              CBUS heartbeat if channel has been connected.
// Parameters:  channel - CBUS channel to check
//------------------------------------------------------------------------------

static void CbusConnectionCheck (byte channel)
{
	static byte busConnected[ MHD_MAX_CHANNELS ] = {0};

	/* If CBUS connection state has changed for this channel,   */
	/* update channel state and hardware.                       */

	if (busConnected[channel] != SI_CbusChannelConnected(channel)) {
		busConnected[ channel ] = SI_CbusChannelConnected( channel );

		// heartbeat has been disabled in all products
		// SI_CbusHeartBeat( channel, busConnected[ channel ] );

		printk("CPCBUS:: ***Channel: %d,  CBUS %s ****\n",
				(int)channel ,
				busConnected[ channel ] ?
				"Connected" : "Unconnected");
	}
}

//------------------------------------------------------------------------------
// Function:    CpCbusProcessPrivateMessage
// Description: Get the result of the last message sent and use it appropriately
//              or process a request from the connected device.
// Parameters:  channel - CBUS channel that has message data for us.
//------------------------------------------------------------------------------

static void CpCbusProcessPrivateMessage(byte channel)
{
	byte     status;
	cbus_req_t  *pCmdRequest;

	pCmdRequest = SI_CbusRequestData( channel );
	//CbusRcpData = pCmdRequest->offsetData;

	switch ( pCmdRequest->command ) {
	case MHD_MSC_MSG_RCP:
		printk("MHD_MSC_MSG_RCP 1 \n");
	/* Acknowledge receipt of command and process it.  Note that    */
	/* we could send the ack before processing anything, because it */
	/* is an indicator that the command was properly received, not  */
	/* that it was executed, however, we use one function to parse  */
	/* the command for errors AND for processing. The only thing we */
	/* must do is make sure that the processing does not exceed the */
	/* ACK response time limit.                                     */

	/* MHL v1					*/
	/* NAGSM_Android_SEL_Kernel_Aakash_20101130	*/
	/* Inform Kernel only on MHD_MSC_MSG_RCP	*/
		//CbusRcpData = pCmdRequest->offsetData;

		//rcp_cbus_uevent();
		//MHL v1 //NAGSM_Android_SEL_Kernel_Aakash_20101126

		status = CpProcessRcpMessage(channel, pCmdRequest->offsetData);
		SI_CbusRcpMessageAck(channel, status, pCmdRequest->offsetData);
		break;

	case MHD_MSC_MSG_RCPK:
		printk("MHD_MSC_MSG_RCPK 1\n");          
		break;

	case MHD_MSC_MSG_RCPE:  
		printk("MHD_MSC_MSG_RCPE 1\n");            
		break;

	case MHD_MSC_MSG_RAP:
		printk("MHD_MSC_MSG_RAP 1\n");            
		status = CpProcessRapMessage(channel, pCmdRequest->offsetData );
			SI_CbusRapMessageAck(channel, status);
		break;

	case MHD_MSC_MSG_RAPK:  
		break;
	}
}

//------------------------------------------------------------------------------
// Function:    CpCbusHandler
// Description: Polls the send/receive state of the CBUS hardware.
//------------------------------------------------------------------------------

void CpCbusHandler(void)
{
	byte channel, status;

	/* Monitor all CBUS channels.   */

	for (channel = 0; channel < MHD_MAX_CHANNELS; channel++) {
		/* Update CBUS status.  */
		SI_CbusUpdateBusStatus( channel );
		CbusConnectionCheck( channel );

		/* Monitor CBUS interrupts. */
		status = SI_CbusHandler( channel );
		if ( status == STATUS_SUCCESS ) {
			/* Get status of current request, if any.   */

			status = SI_CbusRequestStatus( channel );
			switch ( status ) {
			case CBUS_REQ_IDLE: 
				printk("CBUS_REQ_IDLE\n");
				break;

			case CBUS_REQ_PENDING:
				printk("CBUS_REQ_PENDING\n");              
				break;

			case CBUS_REQ_SENT:
				printk("CBUS_REQ_SENT\n");              
				break;

			case CBUS_REQ_RECEIVED:
				printk("CBUS_REQ_RECEIVED\n");  
			/* Received a message or message response.  */
			/* Go do what is appropriate.               */

				CpCbusProcessPrivateMessage( channel );
				break;

			default:
				break;
			}
		} else if ( status == ERROR_NO_HEARTBEAT ) {
			printk("Lost CBUS channel %d heartbeat\n",
						(int)channel);
		} else if ( status == ERROR_NACK_FROM_PEER ) {
			printk ("NACK received from peer, cmd should be"
				" sent again.\n");
		} else {
		/* Lee: Only thing that comes here is interrupt timeout */
		/*  -- is this bad? */
		}
	}
}

//------------------------------------------------------------------------------
// Function:    CpCbusInitialize
// Description: Initialize the CBUS subsystem and enabled the default channels
//------------------------------------------------------------------------------

void CpCbusInitialize(void)
{
	/* Initialize the basic hardware.   */
	SI_CbusInitialize();
}
#endif
