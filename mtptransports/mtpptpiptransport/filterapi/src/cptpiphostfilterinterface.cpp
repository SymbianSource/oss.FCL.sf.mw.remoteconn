
// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cptpiphostfilterinterface.h"

/*
Creates an implementation of an ECOM plugin with the specified UID
*/
EXPORT_C CPTPIPHostFilterInterface* CPTPIPHostFilterInterface::NewL()
	{		
	const TUid KFilterImplUid ={0xA0004A5F};
	
	TAny* defaultFilter=NULL;
	
	TRAPD(error,defaultFilter=REComSession::CreateImplementationL(KFilterImplUid,_FOFF(CPTPIPHostFilterInterface,iID_offset)));
	if(error==KErrNone)
	{
	return (reinterpret_cast<CPTPIPHostFilterInterface*>(defaultFilter));
	}
	else 
		{
		return NULL;	
		}
	}


/*
Lists all the implementations for that Interface identified by the Interface ID
*/
EXPORT_C void CPTPIPHostFilterInterface::ListImplementations(RImplInfoPtrArray& aImplInfoArray)
	{
	const TUid KFilterInterfaceUid ={0xA0004A5E};
	TRAPD(ret, REComSession::ListImplementationsL(KFilterInterfaceUid,aImplInfoArray));
	if(ret != KErrNone)
	{
		RDebug::Print(_L("CPTPIPController::ListImplementations ERROR = %d\n") ,ret);
	}

	}


/*
Destructor
*/
EXPORT_C CPTPIPHostFilterInterface::~CPTPIPHostFilterInterface()
{

}




