/* Test for reading SMSC on AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

#define BUFFER_SIZE 16384

extern GSM_Error ATGEN_ReplyGetSMSC(GSM_Protocol_Message *msg, GSM_StateMachine * s);

int main(int argc, char **argv)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	unsigned char buffer[BUFFER_SIZE];
	FILE *f;
	size_t len;
	GSM_StateMachine *s;
	GSM_Protocol_Message msg;
	GSM_Error error;
	GSM_SMSC SMSC;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: get-smsc-at comm.dump\n");
		return 1;
	}

	/* Open file */
	f = fopen(argv[1], "r");
	test_result(f != NULL);

	/* Read data */
	len = fread(buffer, 1, sizeof(buffer) - 1, f);
	test_result(feof(f));

	/* Zero terminate data */
	buffer[len] = 0;

	/* Close file */
	fclose(f);

	/* Configure state machine */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Initialize AT engine */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->Charset = AT_CHARSET_UCS2;

	/* Init message */
	msg.Type = 0;
	msg.Length = len;
	msg.Buffer = buffer;
	SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);

	/* Pointer to store message */
	s->Phone.Data.SMSC = &SMSC;

	/* Parse it */
	error = ATGEN_ReplyGetSMSC(&msg, s);

	/* This is normally done by ATGEN_Terminate */
	FreeLines(&Priv->Lines);
	GetLineString(NULL, NULL, 0);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	printf("%s\n", GSM_ErrorString(error));

	return (error == ERR_NONE) ? 0 : 1;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
