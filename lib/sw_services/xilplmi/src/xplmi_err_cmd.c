/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xplmi_err_cmd.c
*
* This file contains error management commands code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   02/12/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_err.h"
#include "xplmi_modules.h"
#include "xplmi.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This function is reserved to get the supported features for this
 * module.
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS always for now
 *****************************************************************************/
static int XPlmi_CmdEmReserved(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function sets the error action as prescribed by the command.
 * command payload parameters are
 *		* Error Node ID
 *		* Error Action
 *			0 - Invalid
 *			1 - POR
 *			2 - SRST
 *			3 - Custom(Not supported)
 *			4 - ErrOut
 *			5 - Subsystem Shutdown
 *			6 - Subsystem Restart
 *			7 - PSM correctable
 *			8 - PSM non-correctable
 *			9 - None
 *		* Error ID Mask
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS on successful execution
 *****************************************************************************/
static int XPlmi_CmdEmSetAction(XPlmi_Cmd * Cmd)
{
	u32 NodeId = Cmd->Payload[0];
	u32 ErrorAction = Cmd->Payload[1];
	u32 ErrorMask = Cmd->Payload[2];
	int Status;

	XPlmi_Printf(DEBUG_DETAILED,
	    "%s: NodeId: 0x%0x,  ErrorAction: 0x%0x, ErrorMask: 0x%0x\n\r",
		 __func__, NodeId, ErrorAction, ErrorMask);

	/* Do not allow CUSTOM error action as it is not supported */
	if ((XPLMI_EM_ACTION_CUSTOM == ErrorAction) ||
		(ErrorAction >= XPLMI_EM_ACTION_MAX) ||
		(XPLMI_EM_ACTION_INVALID == ErrorAction)) {
		XPlmi_Printf(DEBUG_GENERAL,
				"Error: XPlmi_CmdEmSetAction: Invalid/unsupported error "
				"action %d received for error 0x%x", ErrorAction, ErrorMask);
		Status = XPLMI_INVALID_ERROR_ACTION;
		goto END;
	}

	/*
	 * Allow error action setting for PSM errors only if LPD is initialized
	 */
	if ((ErrorMask >= XPLMI_NODEIDX_ERROR_PS_SW_CR) &&
		(ErrorMask < XPLMI_NODEIDX_ERROR_PSMERR2_MAX) &&
		((LpdInitialized & LPD_INITIALIZED) != LPD_INITIALIZED)) {
		XPlmi_Printf(DEBUG_GENERAL, "LPD is not initialized to configure "
				"PSM errors and actions\n\r");
		Status = XPLMI_LPD_UNINITIALIZED;
		goto END;

	}

	Status = XPlmi_EmSetAction(NodeId, ErrorMask, (u8)ErrorAction, NULL);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief contains the array of PLM error commands
 *
 *****************************************************************************/
static XPlmi_ModuleCmd XPlmi_ErrCmds[] =
{
	XPLMI_MODULE_COMMAND(XPlmi_CmdEmReserved),
	XPLMI_MODULE_COMMAND(XPlmi_CmdEmSetAction),
};

/*****************************************************************************/
/**
 * @brief Contains the module ID and PLM error commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_ErrModule;

/*****************************************************************************/
/**
 * @brief This function registers the PLM error commands to the PLMI
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
void XPlmi_ErrModuleInit(void)
{

	XPlmi_ErrModule.Id = XPLMI_MODULE_ERROR_ID;
	XPlmi_ErrModule.CmdAry = XPlmi_ErrCmds;
	XPlmi_ErrModule.CmdCnt = sizeof XPlmi_ErrCmds / sizeof *XPlmi_ErrCmds;

	XPlmi_ModuleRegister(&XPlmi_ErrModule);
}
