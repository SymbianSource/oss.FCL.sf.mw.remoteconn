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
// An instance of  PTP/IP Controller is created by RPTPIPFramework::OpenL().
// Controller() method returns the instance created by OpenL().In the destructor of the 
// caller of Controller(), RPTPIPFramework::Close() has to be called to delete the instance of 
// PTP/IP Controller.
// publishedPartner
// 
//


#ifndef __RPTPIPFRAMEWORK_H__
#define __RPTPIPFRAMEWORK_H__

#include <mtp/mptpipcontroller.h>


class CPTPIPController;

class RPTPIPFramework
{
	public:
			
			IMPORT_C void OpenL();		
			IMPORT_C MPTPIPController& Controller();
			IMPORT_C void Close();	
			IMPORT_C RPTPIPFramework();					
													
	private:
																				
			CPTPIPController* iCtrl;													 
};

#endif