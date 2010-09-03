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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdatacodegeneratorTraces.h"
#endif




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
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_CMTPDATACODEGENERATOR_DES_ENTRY );

    iSingletons.Close();

    OstTraceFunctionExit0( CMTPDATACODEGENERATOR_CMTPDATACODEGENERATOR_DES_EXIT );
    }

void CMTPDataCodeGenerator::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_CONSTRUCTL_ENTRY );

    iSingletons.OpenL ();

    OstTraceFunctionExit0( CMTPDATACODEGENERATOR_CONSTRUCTL_EXIT );
    }

CMTPDataCodeGenerator::CMTPDataCodeGenerator() :
    iUndefinedNextCode(KUndenfinedStartCode ),
    iVendorExtFormatCode(EMTPFormatCodeVendorExtDynamicStart)
    {

    }

TInt CMTPDataCodeGenerator::IncServiceIDResource( const TUint aServiceType, TUint& aServiceID )
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_INCSERVICEIDRESOURCE_ENTRY);

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
           OstTrace0( TRACE_NORMAL, CMTPDATACODEGENERATOR_INCSERVICEIDRESOURCE, "CMTPDataCodeGenerator::IncServiceIDResource - Service Type not supported" );          
           }
       }

    OstTraceFunctionExit0( CMTPDATACODEGENERATOR_INCSERVICEIDRESOURCE_EXIT);
    return KErrNone;
    }

void CMTPDataCodeGenerator::DecServiceIDResource()
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_DECSERVICEIDRESOURCE_ENTRY );
    iUndefinedNextCode--;
    OstTraceFunctionExit0( CMTPDATACODEGENERATOR_DECSERVICEIDRESOURCE_EXIT );
    }

TBool CMTPDataCodeGenerator::IsValidServiceType( const TUint aServiceType ) const
    {
    return ( (EMTPServiceTypeNormal == aServiceType) || (EMTPServiceTypeAbstract == aServiceType) );
    }

TInt CMTPDataCodeGenerator::AllocateServiceID(const TMTPTypeGuid& aPGUID, const TUint aServiceType, TUint& aServiceID )
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_ALLOCATESERVICEID_ENTRY );
    
    if( !IsValidServiceType(aServiceType) )
        {
        OstTraceFunctionExit0( CMTPDATACODEGENERATOR_ALLOCATESERVICEID_EXIT );
        return KErrArgument;
        }
    
    TInt err(KErrNone);
    TUint retID (KInvliadServiceID);
    if( iSingletons.ServiceMgr().IsSupportedService(aPGUID) )
        {
        if( iSingletons.ServiceMgr().ServiceTypeOfSupportedService(aPGUID) != aServiceType )
            {
            OstTraceFunctionExit0( DUP1_CMTPDATACODEGENERATOR_ALLOCATESERVICEID_EXIT );
            return KErrArgument;
            }
                       
        err = iSingletons.ServiceMgr().GetServiceId(aPGUID , retID);
        if( KErrNone != err )
            {
            if((err = IncServiceIDResource( aServiceType, retID )) != KErrNone)
                {
                OstTraceFunctionExit0( DUP2_CMTPDATACODEGENERATOR_ALLOCATESERVICEID_EXIT );
                return err;
                }
                            
            err = iSingletons.ServiceMgr().EnableService( aPGUID, retID );
            if( KErrNone != err )
                {
                DecServiceIDResource();
                OstTraceFunctionExit0( DUP3_CMTPDATACODEGENERATOR_ALLOCATESERVICEID_EXIT );
                return err;
                }
            }
        
        }
    else
        {
        if((err = IncServiceIDResource( aServiceType, retID )) != KErrNone)
            {
            OstTraceFunctionExit0( DUP4_CMTPDATACODEGENERATOR_ALLOCATESERVICEID_EXIT );
            return err;
            }
        }
    
   aServiceID = retID;
   iSingletons.ServiceMgr().InsertServiceId( retID );

    OstTraceFunctionExit0( DUP5_CMTPDATACODEGENERATOR_ALLOCATESERVICEID_EXIT );
    return KErrNone;
    }

TInt CMTPDataCodeGenerator::AllocateServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint16& aServicePropertyCode )
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_ALLOCATESERVICEPROPERTYCODE_ENTRY );

    TUint retID = KInvliadU16DataCode;
    if( iSingletons.ServiceMgr().IsSupportedService(aServicePGUID) )
        {
        TInt err = iSingletons.ServiceMgr().GetServicePropertyCode( aServicePGUID, aPKNamespace, aPKID, retID );
        if( KErrNone != err )
            {
            OstTraceFunctionExit0( CMTPDATACODEGENERATOR_ALLOCATESERVICEPROPERTYCODE_EXIT );
            return err;
            }
        
        if(retID == KInvliadU16DataCode)
           {
           if ( iUndefinedNextCode >= KUndenfinedEndCode )
               {
               OstTraceFunctionExit0( DUP1_CMTPDATACODEGENERATOR_ALLOCATESERVICEPROPERTYCODE_EXIT );
               return KErrOverflow;
               }
           
           retID = ++iUndefinedNextCode;
           iSingletons.ServiceMgr().SetServicePropertyCode( aServicePGUID, aPKNamespace, aPKID, retID);
           }
        }
    else
        {
        if ( iUndefinedNextCode >= KUndenfinedEndCode )
            {
            OstTraceFunctionExit0( DUP2_CMTPDATACODEGENERATOR_ALLOCATESERVICEPROPERTYCODE_EXIT );
            return KErrOverflow;
            }
            
    
        retID = ++iUndefinedNextCode;
        }
    
    aServicePropertyCode = retID;
    
    OstTraceFunctionExit0( DUP3_CMTPDATACODEGENERATOR_ALLOCATESERVICEPROPERTYCODE_EXIT );
    return KErrNone;
    }

TInt CMTPDataCodeGenerator::AllocateServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aServiceFormatCode )
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_ALLOCATESERVICEFORMATCODE_ENTRY );

    TUint retID = KInvliadU16DataCode;
    if( iSingletons.ServiceMgr().IsSupportedService(aServicePGUID) )
        {
        TInt err = iSingletons.ServiceMgr().GetServiceFormatCode( aServicePGUID, aGUID, retID );
        if( KErrNone != err )
            {
            OstTraceFunctionExit0( CMTPDATACODEGENERATOR_ALLOCATESERVICEFORMATCODE_EXIT );
            return err;
            }
        
        if(retID == KInvliadU16DataCode)
           {
           if ( iVendorExtFormatCode > EMTPFormatCodeVendorExtDynamicEnd )
               {
               OstTraceFunctionExit0( DUP1_CMTPDATACODEGENERATOR_ALLOCATESERVICEFORMATCODE_EXIT );
               return KErrOverflow;
               }
           
           retID = ++iVendorExtFormatCode;
           iSingletons.ServiceMgr().SetServiceFormatCode( aServicePGUID, aGUID, retID);
           }
        }
    else
        {
        if ( iVendorExtFormatCode > EMTPFormatCodeVendorExtDynamicEnd )
            {
            OstTraceFunctionExit0( DUP2_CMTPDATACODEGENERATOR_ALLOCATESERVICEFORMATCODE_EXIT );
            return KErrOverflow;
            }
        
        retID = ++iVendorExtFormatCode;
        }
    
    aServiceFormatCode = retID;
    

    OstTraceFunctionExit0( DUP3_CMTPDATACODEGENERATOR_ALLOCATESERVICEFORMATCODE_EXIT );
    return KErrNone;
    }

TInt CMTPDataCodeGenerator::AllocateServiceMethodFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint16& aMethodFormatCode )
    {
    OstTraceFunctionEntry0( CMTPDATACODEGENERATOR_ALLOCATESERVICEMETHODFORMATCODE_ENTRY );
    
    TUint retID = KInvliadU16DataCode;
    if( iSingletons.ServiceMgr().IsSupportedService(aServicePGUID) )
        {
        TInt err = iSingletons.ServiceMgr().GetServiceMethodCode( aServicePGUID, aGUID, retID );
        if( KErrNone != err )
            {
            OstTraceFunctionExit0( CMTPDATACODEGENERATOR_ALLOCATESERVICEMETHODFORMATCODE_EXIT );
            return err;
            }
        
        if(retID == KInvliadU16DataCode)
           {
           if ( iUndefinedNextCode > KUndenfinedEndCode )
               {
               OstTraceFunctionExit0( DUP1_CMTPDATACODEGENERATOR_ALLOCATESERVICEMETHODFORMATCODE_EXIT );
               return KErrOverflow;
               }
           
           retID = ++iUndefinedNextCode;
           iSingletons.ServiceMgr().SetServiceMethodCode( aServicePGUID, aGUID, retID);
           }
        }
    else
        {
        if ( iUndefinedNextCode > KUndenfinedEndCode )
            {
            OstTraceFunctionExit0( DUP2_CMTPDATACODEGENERATOR_ALLOCATESERVICEMETHODFORMATCODE_EXIT );
            return KErrOverflow;
            }
        
        retID = ++iUndefinedNextCode;
        }
    
    aMethodFormatCode = retID;
    
    OstTraceFunctionExit0( DUP3_CMTPDATACODEGENERATOR_ALLOCATESERVICEMETHODFORMATCODE_EXIT );
    return KErrNone;
    }



