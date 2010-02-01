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
#ifndef CMTPEXTNDEVDPPLUGIN_H_
#define CMTPEXTNDEVDPPLUGIN_H_

#include <mtp/cmtpextndevdp.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtptypedevicepropdesc.h>

/*enum TMTPDeviceDpExtnSupportedProperties1 
	{
	0x5008,
	
	};*/


class CDevDpReferncePlugin : public CMtpExtnDevicePropPlugin
{
public:
	static CDevDpReferncePlugin* NewL();
	void  ConstructL();
	CDevDpReferncePlugin();
	~CDevDpReferncePlugin();
	TInt  GetDevPropertyL(const TMTPDevicePropertyCode aPropCode, MMTPType**) ;
	TInt GetDevPropertyDescL (const TMTPDevicePropertyCode aPropCode, MMTPType**) ;
	TInt GetDevicePropertyContainerL(TMTPDevicePropertyCode aPropCode, MMTPType**) ;
	TMTPResponseCode SetDevicePropertyL();
	TInt ResetDevPropertyL(const TMTPDevicePropertyCode aPropCode);
	void Supported (TMTPSupportCategory aCategory, RArray <TUint>& aArray, TMTPOperationalMode aMode);


private:
	TMTPTypeUint32* iData;
	CMTPTypeDevicePropDesc* iPropDesc;	
	TMTPDevicePropertyCode iPropCode;
};


#endif /*CMTPEXTNDEVDPPLUGIN_H_*/
