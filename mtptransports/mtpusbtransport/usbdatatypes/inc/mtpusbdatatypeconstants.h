// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file
 @internalComponent
*/

#ifndef MTPUSBDATATYPECONSTANTS_H
#define MTPUSBDATATYPECONSTANTS_H

#include <mtp/mtpdatatypeconstants.h>

/**
USB MTP device class data type identifiers. 
*/
enum TMTPUsbTypeIds
    {
    EMTPUsbTypeContainer                    = (EMTPFirstTransportDataType),
    EMTPUsbTypeContainerHeader              = (EMTPFirstTransportDataType + 1),
    EMTPUsbTypeControlRequestCancelData     = (EMTPFirstTransportDataType + 2),
    EMTPUsbTypeControlRequestDeviceStatus   = (EMTPFirstTransportDataType + 3),
    EMTPUsbTypeControlRequestMODFunction    = (EMTPFirstTransportDataType + 4),
    EMTPUsbTypeControlRequestMODHeader      = (EMTPFirstTransportDataType + 5),
    EMTPUsbTypeControlRequestSetup          = (EMTPFirstTransportDataType + 6),
    EMTPUsbTypeEventContainer               = (EMTPFirstTransportDataType + 7),
    EMTPUsbTypeParameterPayloadBlock        = (EMTPFirstTransportDataType + 8),
    };

#endif // MTPUSBDATATYPECONSTANTS_H
