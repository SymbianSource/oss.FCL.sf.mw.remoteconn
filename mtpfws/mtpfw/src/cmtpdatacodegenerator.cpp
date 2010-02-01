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
#include <e32err.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtpservicecommon.h"
#include "rmtpframework.h"
#include "cmtpdatacodegenerator.h"
#include "cmtpservicemgr.h"



// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DataCodeGenerator");)

const TUint16 KUndenfinedStartCode = EMTPCodeUndefined1Start + 1;
const TUint16 KUndenfinedEndCode = EMTPCodeUndefined1End;


CMTPDataCodeGenerator* CMTPDataCodeGenerator::NewL()
    {
    CMTPDataCodeGenerator* self = new (ELeave) CMTPDataCodeGenerator();
    CleanupStack::PushL ( self );
    self->ConstructL ();
    CleanupStack::Pop ( self );
    return self;
    }

CMTPDataCodeGenerator::~CMTPDataCodeGenerator()
    {
    __FLOG(_L8("CMTPDataCodeGenerator::~CMTPDataCodeGenerator - Entry"));

    iSingletons.Close();

    __FLOG(_L8("CMTPDataCodeGenerator::~CMTPDataCodeGenerator - Exit"));
    
    __FLOG_CLOSE;
    }

void CMTPDataCodeGenerator::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPDataCodeGenerator::ConstructL - Entry"));

    iSingletons.OpenL ();

    __FLOG(_L8("CMTPDataCodeGenerator::ConstructL - Exit"));
    }

CMTPDataCodeGenerator::CMTPDataCodeGenerator() :
    iUndefinedNextCode(KUndenfinedStartCode ),
    iVendorExtFormatCode(EMTPFormatCodeVendorExtDynamicStart)
    {

    }

TInt CMTPDataCodeGenerator::IncServiceIDResource( const TUint aServiceType, TUint& aServiceID )
    {
    __FLOG(_L8("CMTPDataCodeGenerator::IncServiceIDResource - Entry"));
    if ( iUndefinedNextCode >= KUndenfinedEndCode )
        return KErrOverflow;
    
    switch ( aServiceType )
       {
       case EMTPServiceTypeNormal:
           {
           aServiceID = ( ( ++iUndefinedNextCode ) | KNormalServiceTypeMask );
           }
           break;
   
       case EMTPServiceTypeAbstract:
           {
           aServiceID = ( (++iUndefinedNextCode) | KAbstrackServiceTypeMask );
           }
           break;
       default:
           {
           __FLOG(_L8("CMTPDataCodeGenerator::IncServiceIDResource - Service Type not supported")); 
           }
       }
    __FLOG(_L8("CMTPDataCodeGenerator::IncServiceIDResource - Exit"));
    return KErrNone;
    }

void CMTPDataCodeGenerator::DecServiceIDResource()
    {
    __FLOG(_L8("CMTPDataCodeGenerator::DecServiceIDResource - Entry"));
    iUndefinedNextCode--;
    __FLOG(_L8("CMTPDataCodeGenerator::DecServiceIDResource - Exit"));
    }

TBool CMTPDataCodeGenerator::IsValidServiceType( const TUint aServiceType ) const
    {
    return ( (EMTPServiceTypeNormal == aServiceType) || (EMTPServiceTypeAbstract == aServiceType) );
    }

TInt CMTPDataCodeGenerator::AllocateServiceID(const TMTPTypeGuid& aPGUID, const TUint aServiceType, TUint& aServiceID )
    {
    __FLOG(_L8("CMTPDataCodeGenerator::AllocateServiceID - Entry"));
    
    if( !IsValidServiceType(aServiceType) )
        return KErrArgument;
        
    TInt err(KErrNone);
    TUint retID (KInvliadServiceID);
    if( iSingletons.ServiceMgr().IsSupportedService(aPGUID) )
        {
        if( iSingletons.ServiceMgr().ServiceTypeOfSupportedService(aPGUID) != aServiceType )
            return KErrArgument;
            
        err = iSingletons.ServiceMgr().GetServiceId(aPGUID , retID);
        if( KErrNone != err )
            {
            if((err = IncServiceIDResource( aServiceType, retID )) != KErrNone)
                return err;
            
            err = iSingletons.ServiceMgr().EnableService( aPGUID, retID );
            if( KErrNone != err )
                {
                DecServiceIDResource();
                return err;
                }
            }
        
        }
    else
        {
        if((err = IncServiceIDResource( aServiceType, retID )) != KErrNone)
            return err;
        }
    
   aServiceID = retID;
   iSingletons.ServiceMgr().InsertServiceId( retID );

    __FLOG(_L8("CMTPDataCodeGenerator::AllocateServiceID - Exit"));
    return KErrNone;
    }

TInt CMTPDataCodeGenerator::AllocateServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint16& aServicePropertyCode )
    {
    __FLOG(_L8("CMTPDataCodeGenerator::AllocateServicePropertyCode - Entry"));

    TUint retID = KInvliadU16DataCode;
    if( iSingletons.ServiceMgr().IsSupportedService(aServicePGUID) )
        {
        TInt err = iSingletons.ServiceMgr().GetServicePropertyCode( aServicePGUID, aPKNamespace, aPKID, retID );
        if( KErrNone != err )
            return err;

        if(retID == KInvliadU16DataCode)
           {
           if ( iUndefinedNextCode >= KUndenfinedEndCode )
               return KErrOverflow;
           
           retID = ++iUndefinedNextCode;
           iSingletons.ServiceMgr().SetServicePropertyCode( aServicePGUID, aPKNamespace, aPKID, retID);
           }
        }
    else
        {
        if ( iUndefinedNextCode >= KUndenfinedEndCode )
            return KErrOverflow;
    
        retID = ++iUndefinedNextCode;
        }
    
    aServicePropertyCode = retID;
    
    __FLOG(_L8("CMTPDataCodeGenerator::AllocateServicePropertyCode - Exit"));
    return KErrNone;
    }

TInt CMTPDataCodeGenerator::AllocateServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aServiceFormatCode )
    {
    __FLOG(_L8("CMTPServiceConfig::AllocateServiceFormatCode - Entry"));

    TUint retID = KInvliadU16DataCode;
    if( iSingletons.ServiceMgr().IsSupportedService(aServicePGUID) )
        {
        TInt err = iSingletons.ServiceMgr().GetServiceFormatCode( aServicePGUID, aGUID, retID );
        if( KErrNone != err )
            return err;

        if(retID == KInvliadU16DataCode)
           {
           if ( iVendorExtFormatCode > EMTPFormatCodeVendorExtDynamicEnd )
               return KErrOverflow;
           
           retID = ++iVendorExtFormatCode;
           iSingletons.ServiceMgr().SetServiceFormatCode( aServicePGUID, aGUID, retID);
           }
        }
    else
        {
        if ( iVendorExtFormatCode > EMTPFormatCodeVendorExtDynamicEnd )
            return KErrOverflow;
            
        retID = ++iVendorExtFormatCode;
        }
    
    aServiceFormatCode = retID;
    
    __FLOG(_L8("CMTPServiceConfig::AllocateServiceFormatCode - Exit"));
    return KErrNone;
    }

TInt CMTPDataCodeGenerator::AllocateServiceMethodFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aMethodFormatCode )
    {
    __FLOG(_L8("CMTPDataCodeGenerator::AllocateServiceMethodFormatCode - Entry"));
    
    TUint retID = KInvliadU16DataCode;
    if( iSingletons.ServiceMgr().IsSupportedService(aServicePGUID) )
        {
        TInt err = iSingletons.ServiceMgr().GetServiceMethodCode( aServicePGUID, aGUID, retID );
        if( KErrNone != err )
            return err;
    
        if(retID == KInvliadU16DataCode)
           {
           if ( iUndefinedNextCode > KUndenfinedEndCode )
               return KErrOverflow;
           
           retID = ++iUndefinedNextCode;
           iSingletons.ServiceMgr().SetServiceMethodCode( aServicePGUID, aGUID, retID);
           }
        }
    else
        {
        if ( iUndefinedNextCode > KUndenfinedEndCode )
            return KErrOverflow;
            
        retID = ++iUndefinedNextCode;
        }
    
    aMethodFormatCode = retID;
    
    __FLOG(_L8("CMTPDataCodeGenerator::AllocateServiceMethodFormatCode - Exit"));
    return KErrNone;
    }



