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
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef CMTPDATAPROVIDERPLUGIN_H
#define CMTPDATAPROVIDERPLUGIN_H

#include <e32base.h>
#include <mtp/mmtpdataprovider.h>
#include <mtp/mtpdataproviderapitypes.h>

class MMTPDataProviderFramework;

/** 
CMTPDataProviderPlugin interface UID.
*/
const TUid KMTPDataProviderPluginInterfaceUid = {0x102827AD};

/** 
Defines the MTP data provider ECOM plug-in service provider 
interface.
@publishedPartner
@released
*/
class CMTPDataProviderPlugin : 
    public CBase, 
    public MMTPDataProvider
    {
    
public:

    IMPORT_C static CMTPDataProviderPlugin* NewL(TUid aImplementationUid, TAny* aParams);
    IMPORT_C virtual ~CMTPDataProviderPlugin();

protected: // From MMTPDataProvider

    IMPORT_C MMTPDataProviderFramework& Framework() const;
    
protected:

    IMPORT_C CMTPDataProviderPlugin(TAny* aParams);

private: // Owned

    TUid                        iDtorIdKey;
    
private: // Not owned
    
    MMTPDataProviderFramework*  iFramework;
    };
    
#endif // CMTPDATAPROVIDERPLUGIN_H
