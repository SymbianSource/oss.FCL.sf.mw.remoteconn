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
// Provides an interface (ECOM-plugin interface)for loading any implementation of filter.
// If multiple instances exist, which implementation is loaded is based either on UID 
// mentioned or the parameters passed that distinguish it from the other implementations.
// ListImplementations() can be called to get the list the list of implementations
// for this particular interface.
// 
//

/**
 @publishedPartner
 @released
*/

#ifndef __CPTPIPHOSTFILTERINTERFACE_H__
#define __CPTPIPHOSTFILTERINTERFACE_H__


#include <e32base.h>	
#include <e32std.h>
#include <es_sock.h>
#include <e32cmn.h>
#include <ecom/ecom.h>

#include <mtp/mptpiphostfilter.h>


class CPTPIPHostFilterInterface : public MPTPIPHostFilter
{	
	public:
		IMPORT_C static CPTPIPHostFilterInterface* NewL();				

		IMPORT_C static void ListImplementations(RImplInfoPtrArray & aImplInfoArray);	
		
		IMPORT_C virtual ~CPTPIPHostFilterInterface();	 				
		
	private:
		TInt iID_offset;//offset of Implementation UID
		
};


#endif