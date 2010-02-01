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
 @file
 @internalComponent
*/

#ifndef MEXTNDEVPLUGINCALLBACK_H_
#define MEXTNDEVPLUGINCALLBACK_H_

#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpdataproviderframework.h>

class MExtnDevPluginCallback
{
public:
	virtual void OnDevicePropertyChangedL (TMTPDevicePropertyCode& aPropCode) = 0;

	/** 
	  * This method return reference to the MMTPDataProviderFramework
	  *
	  * @return : reference to MMTPDataProviderFramework
	  */
	virtual MMTPDataProviderFramework& DataProviderFramework () = 0;
};

#endif /*MEXTNDEVPLUGINCALLBACK_H_*/
