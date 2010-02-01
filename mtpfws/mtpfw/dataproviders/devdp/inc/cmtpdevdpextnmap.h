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
#include "cmtpextndevdp.h"

#ifndef CMTPDEVDPEXTNMAP_H
#define CMTPDEVDPEXTNMAP_H


  class  CDevDpExtnPluginMap : public CBase
  {
  public:
  static	CDevDpExtnPluginMap* NewL(MExtnDevPluginCallback& aDevdp, TUid aUid);
  	void ConstructL(MExtnDevPluginCallback& aDevdp, TUid aUid);
  	virtual ~CDevDpExtnPluginMap();
  	void SetExtPlugin(CMtpExtnDevicePropPlugin* aPlugin);
  	CMtpExtnDevicePropPlugin* ExtPlugin();
  	RArray<TUint>* SupportedOpCodes();
  private:	
  	RArray<TUint> iSupportedOpCodes;
  	CMtpExtnDevicePropPlugin* iPlugin;// MExtnDevicePropDp
  	
  };
  
  #endif