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
*/

#ifndef MMTPDATACODEGENERATOR_H_
#define MMTPDATACODEGENERATOR_H_

#include <e32def.h>
#include <mtp/tmtptypeguid.h>

class TMTPTypeGuid;

/** 
Defines the MTP data provider framework datacode generator 
interface.
*/
class MMTPDataCodeGenerator
    {
public:

    /**
    Generate one ServiceID .
    @param aPGUID The GUID of Service.
    @param aServiceType service type (normal or abstract).
    @param aServiceID the service id that is allocated.
    @return System wide error code. KErrNone, if successful.
    */
   virtual TInt AllocateServiceID( const TMTPTypeGuid& aPGUID, const TUint aServiceType, TUint& aServiceID ) = 0;
   
   /**
    Generate one service property code .
    @param aServicePGUID The service GUID .
    @param aPKNamespace The GUID of PKey Namespace
    @param aPKID The PKey ID of service property
    @param aServicePropertyCode The service property code that is allocated.
    @return System wide error code. KErrNone if successful.
    */
   virtual TInt AllocateServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint16& aServicePropertyCode ) = 0;
   
   /**
    Generate one service format code.
    @param aServicePGUID The service GUID .
    @param aGUID The GUID of service format
    @param aServiceFormatCode The service format code that is allocated.
    @return System wide error code. KErrNone if successful.
    */
   virtual TInt AllocateServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aServiceFormatCode ) = 0;
   
   /**
    Generate one service method code .
    @param aServicePGUID The service GUID .
    @param aGUID The GUID of method
    @param aMethodFormatCode The method format code that is allocated.
    @return System wide error code. KErrNone if successful.
    */
   virtual TInt AllocateServiceMethodFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aMethodFormatCode ) = 0;
   
   };



#endif /* MMTPDATACODEGENERATOR_H_ */
