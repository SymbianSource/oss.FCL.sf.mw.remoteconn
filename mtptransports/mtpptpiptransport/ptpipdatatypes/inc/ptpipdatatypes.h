// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @internalComponent
*/

#ifndef CPTPIPDATATYPES_H_
#define CPTPIPDATATYPES_H_

/**
PTPIP data types:
1. request packet & response packet
2. data packets: start data, data, end data
3. cancel packet
4. probe packet and response

The structure as per the MTP type hierarchy is as follows:

One ptpip container of the mtp compound type for the generic use ( it would be 
used for everything except the data phase commands). This contains the type and 
length as its first 8 bytes. The next part is the payload, which can be of the 
following types:
1. Request type 
2. Cancel type
3. Start data type
 
One generic ptpip container of the Mtp compound type for the data phase. This 
contains the len, type and transaction id as its first 12 bytes. The next part 
is the payload which is provided by the mtp framework. 
*/
   
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/mtpdatatypeconstants.h>

/**
PTPIP MTP device class data type identifiers. 
*/

enum TMTPPTPIPTypeIds
	{
	EPTPIPTypeRequestPayload    = (EMTPFirstTransportDataType + 0x100),   
	EPTPIPTypeResponsePayload   = (EMTPFirstTransportDataType + 0x101),
	EPTPIPTypeStartDataPayload 	= (EMTPFirstTransportDataType + 0x102),
	EPTPIPTypeGenericContainer	= (EMTPFirstTransportDataType + 0x103),
	EPTPIPTypeDataContainer		= (EMTPFirstTransportDataType + 0x104),
	EPTPIPTypeInitCmdRequest	= (EMTPFirstTransportDataType + 0x105),
	EPTPIPTypeInitCmdAck		= (EMTPFirstTransportDataType + 0x106),
	EPTPIPTypeInitEvtRequest	= (EMTPFirstTransportDataType + 0x107),
	EPTPIPTypeInitEvtAck		= (EMTPFirstTransportDataType + 0x108),
	EPTPIPTypeInitFail		    = (EMTPFirstTransportDataType + 0x109)
	
	};


#endif /*CPTPIPDATATYPES_H_*/
