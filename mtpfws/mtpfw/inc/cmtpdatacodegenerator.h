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
@internalComponent
*/ 

#ifndef CMTPDATACODEGENERATOR_H_
#define CMTPDATACODEGENERATOR_H_

#include <e32base.h>
#include <mtp/mmtpdatacodegenerator.h>
#include "rmtpframework.h"


class CMTPDataCodeGenerator :
    public CBase,
    public MMTPDataCodeGenerator
    {
public:
    static CMTPDataCodeGenerator* NewL();
    ~CMTPDataCodeGenerator();

public: // From MMTPDataCodeGenerator
    TInt AllocateServiceID(const TMTPTypeGuid& aPGUID, const TUint aServiceType, TUint& aServiceID );
    TInt AllocateServicePropertyCode(const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint16& aServicePropertyCode );
    TInt AllocateServiceFormatCode(const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aServiceFormatCode);
    TInt AllocateServiceMethodFormatCode(const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aMethodFormatCode );


private:
    CMTPDataCodeGenerator();
    void ConstructL();
    TInt IncServiceIDResource( const TUint aServiceType, TUint& aServiceID );
    void DecServiceIDResource();
    TBool IsValidServiceType( const TUint aServiceType ) const;
    
private:

    /**
    The framework singletons.
    */
    RMTPFramework                   iSingletons;

    /*
     * Undefined Section.1 : 0x0000 --- 0x0FFF
     */
    TUint16 iUndefinedNextCode;
    
    /*
     * Vendor Extension Object Format Code : 0xB000 --- 0xB7FF
     *    (EMTPFormatCodeVendorExtStart -- EMTPFormatCodeVendorExtEnd)
     */
    TUint16 iVendorExtFormatCode;

    };

#endif /* CMTPDATACODEGENERATOR_H_ */

    
    
