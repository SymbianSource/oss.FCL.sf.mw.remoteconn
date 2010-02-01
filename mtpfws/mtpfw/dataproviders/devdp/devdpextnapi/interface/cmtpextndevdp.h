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
// Defines the MTP Device dp extension data provider ECOM plug-in service provider 
// interface.
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef CMTPDEVDPEXTN_H
#define CMTPDEVDPEXTN_H

#include <e32debug.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/mmtpdevdpextn.h>
#include <mtp/mmtpdataproviderframework.h>

class MExtnDevPluginCallback;
	
/** 
Those who are implementing "MExtnDevicePropDp" interface as ecom plugin,
should use 0x20010ADA as the interface UID in ECOM resource file.
*/
const TUid KMTPExtnDevDpPluginInterfaceUid = {0x20010ADA};


class CMtpExtnDevicePropPlugin : public CBase, public MExtnDevicePropDp
{

public:
/**
loads the plugin having implementation uid as aUid 
*/
	IMPORT_C static CMtpExtnDevicePropPlugin* NewL(TUid aUid );

	IMPORT_C ~CMtpExtnDevicePropPlugin();

	IMPORT_C TInt  GetDevPropertyL (const TMTPDevicePropertyCode /*aPropCode*/, MMTPType**);

	IMPORT_C TInt GetDevPropertyDescL (const TMTPDevicePropertyCode /*aPropCode*/, MMTPType**);

	IMPORT_C TInt GetDevicePropertyContainerL(TMTPDevicePropertyCode /*aPropCode*/, MMTPType**);

	IMPORT_C TMTPResponseCode SetDevicePropertyL();

	IMPORT_C void Supported (TMTPSupportCategory /*aCategory*/, RArray <TUint>& /*aArray*/ ,TMTPOperationalMode /*aMode*/);

	IMPORT_C TInt ResetDevPropertyL(const TMTPDevicePropertyCode aPropCode);


	IMPORT_C void SetCallback(MExtnDevPluginCallback& aExtnDevPluginCallback);

	IMPORT_C void NotifyOnDevicePropertyChanged (TMTPDevicePropertyCode& aPropCode);

	/**
	* This method return mtpframework
	*/
	IMPORT_C MMTPDataProviderFramework& Framework( ) const;
 
private:
	MExtnDevPluginCallback* iExtnDevPluginCallback;

	TUid                        iDtorIdKey;
};

#endif //






