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

/**
 @file
 @internalTechnology
*/

#ifndef CMTPDATAPROVIDERCONFIG_H
#define CMTPDATAPROVIDERCONFIG_H

#include <mtp/mmtpdataproviderconfig.h>
#include <mtp/mtpdataproviderapitypes.h>

#include <e32std.h>

class TResourceReader;

/**
Implements the data provider configurability parameter data interface. Each 
loaded data provider is required to supply a compiled resource file which 
implements a number of configurability parameters. These parameters are read 
accessible only.
@internalTechnology
 
*/
class CMTPDataProviderConfig : 
    public CBase, 
    public MMTPDataProviderConfig
    {
public:

    static CMTPDataProviderConfig* NewL(TResourceReader& aReader, const TDesC& aFileName);
    virtual ~CMTPDataProviderConfig();
    
    TBool ModeSupported(TMTPOperationalMode aMode) const;

public: // From MMTPDataProviderConfig

    TUint UintValue(TParameter aParam) const;
    TBool BoolValue(TParameter aParam) const;
    const TDesC& DesCValue(TParameter aParam) const;
    const TDesC8& DesC8Value(TParameter aParam) const;
    void GetArrayValue(TParameter aParam, RArray<TUint>& aArray) const;
        
private:

    CMTPDataProviderConfig();
    void ConstructL(TResourceReader& aReader, const TDesC& aFileName);

private:

    TMTPDataProviderTypes   iType;
    TInt                    iVersion;
    TBool                   iObjectEnumerationPersistent;
    TUint32                 iSupportedModes;
    HBufC*                  iServerName;
    HBufC*                  iServerImageName;
    TUint32                 iOpaqueResource;
    HBufC*                  iResourceFileName;
    TUint8                  iEnumerationPhase;
    };

#endif // CMTPDATAPROVIDERCONFIG_H
