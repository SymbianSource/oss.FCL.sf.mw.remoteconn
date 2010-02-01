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

#include "cmtpdevdpextnmap.h"

CDevDpExtnPluginMap* CDevDpExtnPluginMap::NewL(MExtnDevPluginCallback& aDevdp, TUid aUid)
	{
	CDevDpExtnPluginMap* self = new (ELeave) CDevDpExtnPluginMap();
	CleanupStack::PushL(self);
	self->ConstructL(aDevdp, aUid);
	CleanupStack::Pop(self);
	return self;
	}

void CDevDpExtnPluginMap::ConstructL(MExtnDevPluginCallback& aDevdp, TUid aUid)
	{
	CMtpExtnDevicePropPlugin* extnPlugin = CMtpExtnDevicePropPlugin::NewL(aUid);
	extnPlugin->SetCallback(aDevdp);
	SetExtPlugin(extnPlugin);
	}

void CDevDpExtnPluginMap::SetExtPlugin(CMtpExtnDevicePropPlugin* aPlugin)
	{
	iPlugin = aPlugin;
	}


CMtpExtnDevicePropPlugin* CDevDpExtnPluginMap::ExtPlugin()
	{
	return iPlugin;
	}
 	
CDevDpExtnPluginMap::~CDevDpExtnPluginMap()
	{
	iSupportedOpCodes.Close();

	if(NULL != iPlugin)
		{
		delete iPlugin;
		iPlugin = NULL;
		}
	} 	
 	
RArray<TUint>* CDevDpExtnPluginMap::SupportedOpCodes()
	{
	return &iSupportedOpCodes;
	}

