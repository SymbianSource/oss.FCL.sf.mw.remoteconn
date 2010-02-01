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

#include "cmtpextndevdp.h"
#include "mextndevplugincallback.h"

EXPORT_C TInt  CMtpExtnDevicePropPlugin::GetDevPropertyL (const TMTPDevicePropertyCode /*aPropCode*/, MMTPType**)
	{
	return KErrNotSupported;
	}

EXPORT_C TInt CMtpExtnDevicePropPlugin::GetDevPropertyDescL (const TMTPDevicePropertyCode /*aPropCode*/, MMTPType**)
	{
	return KErrNotSupported;
	}
 
EXPORT_C TInt CMtpExtnDevicePropPlugin::GetDevicePropertyContainerL(TMTPDevicePropertyCode /*aPropCode*/, MMTPType**)
	 {
	 return KErrNotSupported;	
	 }

EXPORT_C TMTPResponseCode CMtpExtnDevicePropPlugin::SetDevicePropertyL()
	{
	return EMTPRespCodeDevicePropNotSupported;
	}
 
EXPORT_C void CMtpExtnDevicePropPlugin::Supported (TMTPSupportCategory /*aCategory*/, RArray <TUint>& /*aArray*/ ,TMTPOperationalMode /*aMode*/)
	{
	return;
	}
 
EXPORT_C TInt CMtpExtnDevicePropPlugin::ResetDevPropertyL(const TMTPDevicePropertyCode /*aPropCode*/)
	{
	return KErrNotSupported;
	}

EXPORT_C CMtpExtnDevicePropPlugin* CMtpExtnDevicePropPlugin::NewL(TUid aUid )
	{
	CMtpExtnDevicePropPlugin* self = reinterpret_cast<CMtpExtnDevicePropPlugin*>(
	REComSession::CreateImplementationL(aUid, _FOFF(CMtpExtnDevicePropPlugin, iDtorIdKey))); 
	return self; 
	}

EXPORT_C  CMtpExtnDevicePropPlugin::~CMtpExtnDevicePropPlugin()
	{
	REComSession::DestroyedImplementation (iDtorIdKey);
	}
	
EXPORT_C void CMtpExtnDevicePropPlugin::SetCallback(MExtnDevPluginCallback& aExtnDevPluginCallback)
	{
	iExtnDevPluginCallback = &aExtnDevPluginCallback;
	}
	
EXPORT_C void CMtpExtnDevicePropPlugin::NotifyOnDevicePropertyChanged (TMTPDevicePropertyCode& aPropCode)
	{
	TRAP_IGNORE(iExtnDevPluginCallback->OnDevicePropertyChangedL (aPropCode));	
	}

	/**
	* This method return mtpframework
	*/
EXPORT_C MMTPDataProviderFramework& CMtpExtnDevicePropPlugin::Framework ( ) const
	{
        	//RDebug::Printf(" data provider id is %d", );
	return iExtnDevPluginCallback->DataProviderFramework();
	}

