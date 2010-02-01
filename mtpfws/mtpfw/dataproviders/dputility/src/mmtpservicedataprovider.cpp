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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/mmtpservicedataprovider.cpp

#include <mtp/mmtptype.h>

#include "mmtpservicedataprovider.h"

EXPORT_C MMTPServiceHandler* MMTPServiceDataProvider::ServiceHandler() const
	{
	return NULL;
	}

EXPORT_C TBool MMTPServiceDataProvider::IsValidServicePropCodeL(TUint16 /*aPropCode*/) const
	{
	return EFalse;
	}

EXPORT_C TUint32 MMTPServiceDataProvider::ServiceID() const
	{
	return 0;
	}

EXPORT_C TUint32 MMTPServiceDataProvider::OperationGranularity() const
	{
	// Default granularity of delete object and get object property is 50
	return 50;
	}
