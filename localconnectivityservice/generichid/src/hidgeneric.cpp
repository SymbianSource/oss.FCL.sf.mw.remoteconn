/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Generic hid implementation
*
*/

#include <e32std.h>

#include "debug.h"
#include "hidgeneric.h"
#include "hidreportroot.h"
#include "hidparser.h"
#include "hiddriveritem.h"
#include "hidconnectioninfo.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CGenericHid* CGenericHid::NewLC(MTransportLayer* aTransportLayer)
    {
    TRACE_INFO((_L("[HID]\tCGenericHid::NewLC(0x%08x)"), aTransportLayer));
    CGenericHid* self = new (ELeave) CGenericHid(aTransportLayer);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CGenericHid* CGenericHid::NewL(MTransportLayer* aTransportLayer)
    {
    TRACE_INFO((_L("[HID]\tCGenericHid::NewL(0x%08x)"), aTransportLayer));
	CGenericHid* self = NewLC(aTransportLayer);
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CGenericHid::ConstructL()
    {
    TRACE_INFO(_L("[HID]\tCGenericHid::ConstructL()"));
    TRACE_INFO(_L("[HID]\tCGenericHid::ConstructL(): Creating Parser..."));
    iParser = CParser::NewL();
    iInputHandlingReg = CHidInputDataHandlingReg::NewL();
    }

// ---------------------------------------------------------------------------
// CGenericHid()
// ---------------------------------------------------------------------------
//
CGenericHid::CGenericHid(MTransportLayer* aTransportLayer) :
    iDriverList(_FOFF(CDriverListItem, iSlink)),
    iTransportLayer(aTransportLayer)
    {
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CGenericHid::~CGenericHid()
    {
    TRACE_FUNC_ENTRY
    RemoveDrivers();
    iConnectionInfo.ResetAndDestroy();
    iConnectionInfo.Close();
    delete iInputHandlingReg;
    delete iParser;
    REComSession::FinalClose();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// CountryCodeL
// ---------------------------------------------------------------------------
//
TUint CGenericHid::CountryCodeL(TInt aConnID)
    {
    // Pass the request through to the transport layer.
    return (iTransportLayer->CountryCodeL(aConnID));
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// VendorIdL
// ---------------------------------------------------------------------------
//
TUint CGenericHid::VendorIdL(TInt aConnID)
    {
    // Pass the request through to the transport layer.
    return (iTransportLayer->VendorIdL(aConnID));
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// ProductIdL()
// ---------------------------------------------------------------------------
//
TUint CGenericHid::ProductIdL(TInt aConnID)
    {
    // Pass the request through to the transport layer.
    return iTransportLayer->ProductIdL(aConnID);
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// SetProtocol
// ---------------------------------------------------------------------------
//
void CGenericHid::SetProtocolL(TInt aConnectionId, TUint16 aInterface,
                              MDriverAccess::TProtocols aProtocol, 
                              CHidDriver* aDriver)
    {
    iTransportLayer->SetProtocolL(aConnectionId, static_cast<TUint16>(aProtocol), 
            aInterface);    
    CConnectionInfo* conninfo = SeekConnectionInfo( aConnectionId );
        if ( conninfo )
            {
            conninfo->SetLastCommandHandler(aDriver);
            }          
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// GetProtocol
// ---------------------------------------------------------------------------
//
void CGenericHid::GetProtocolL(TInt aConnectionId,TUint16 aInterface)
    {
    iTransportLayer->GetProtocolL(aConnectionId, aInterface);    
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// GetReport
// ---------------------------------------------------------------------------
//
void CGenericHid::GetReportL(TInt aConnectionId,
    TUint8 aReportId, TUint16 aInterface, TUint16 aLength)
    {
    iTransportLayer->GetReportL(aConnectionId, MDriverAccess::EInput, aReportId, 
            aInterface, aLength);    
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// SetReport()
// ---------------------------------------------------------------------------
//
void CGenericHid::SetReportL(TInt aConnectionId, TUint8 aReportId,
    MDriverAccess::TReportType aReportType, const TDesC8& aPayload,
    TUint16 aInterface, CHidDriver* aDriver)
    {
    iTransportLayer->SetReportL(aConnectionId, static_cast<TUint8>(aReportType),
        aReportId, aInterface, aPayload);    
    CConnectionInfo* conninfo = SeekConnectionInfo( aConnectionId );
        if ( conninfo )
            {
            conninfo->SetLastCommandHandler(aDriver);
            }          
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// DataOut()
// ---------------------------------------------------------------------------
//
void CGenericHid::DataOutL(TInt aConnectionId, TUint8 aReportId,
                          const TDesC8& aPayload,
                          TUint16 aInterface)
    {
    iTransportLayer->DataOutL(aConnectionId, aReportId, aInterface, aPayload);    
    }


// ---------------------------------------------------------------------------
// From MDriverAccess
// GetIdle()
// ---------------------------------------------------------------------------
//
void CGenericHid::GetIdleL(TInt aConnectionId, TUint8 aReportId,
    TUint16 aInterface )
    {
    iTransportLayer->GetIdleL(aConnectionId, aReportId, aInterface);    
    }

// ---------------------------------------------------------------------------
// From MDriverAccess
// SetIdle()
// ---------------------------------------------------------------------------
//
void CGenericHid::SetIdleL(TInt aConnectionId, TUint8 aDuration,
    TUint8 aReportId, TUint16 aInterface, CHidDriver* aDriver)
    {
    iTransportLayer->SetIdleL(aConnectionId, aDuration, aReportId, aInterface);
    CConnectionInfo* conninfo = SeekConnectionInfo( aConnectionId );
    if ( conninfo )
        {
        conninfo->SetLastCommandHandler(aDriver);
        }          
                       
    }

// ---------------------------------------------------------------------------
// RemoveDrivers()
// ---------------------------------------------------------------------------
//
void CGenericHid::RemoveDrivers()
    {
    TRACE_FUNC
    // Driver instances
    CDriverListItem* driverItem;
    while ( !iDriverList.IsEmpty() )
        {
        driverItem = iDriverList.Last();
        iDriverList.Remove( *driverItem );
        delete driverItem;
        }
    }

// ---------------------------------------------------------------------------
// From CHidTransport
// Disconnected()
// ---------------------------------------------------------------------------
//
TInt CGenericHid::Disconnected( TInt aConnectionId )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNone;
    
    TSglQueIter<CDriverListItem> driverIter( iDriverList );
    driverIter.SetToFirst();    

    CDriverListItem* driverItem = driverIter;    
    while ( driverItem )
        {            
        driverIter++;    
        if ( driverItem->ConnectionId() == aConnectionId )
            {
            TRACE_INFO(_L("[HID]\tCGenericHid::Disconnected driver"));
            if (driverItem->Driver())
                {
                driverItem->Driver()->Disconnected(0);
                }
            // Remove it from the list of driver instances           
            
            iDriverList.Remove(*driverItem);
            delete driverItem;
            driverItem = NULL;
            retVal = KErrNone;
            }                                  
        driverItem = driverIter;
        
        TRACE_INFO(_L("[HID]\tCGenericHid::Disconnected next driver"));
        }
    TRACE_INFO(_L("[HID]\tCGenericHid::Disconnected remove connection info"));
    TInt count = iConnectionInfo.Count();
    for (TInt i = count-1 ; i>=0; i--)
        {
        TRACE_INFO((_L("[HID]\tCGenericHid::Disconnected remove connection info %d"),i));
        CConnectionInfo* conninfo = iConnectionInfo[i];
        TRACE_INFO((_L("[HID]\tCGenericHid::Disconnected remove connection info %d"),i));
        if ( conninfo->ConnectionID() == aConnectionId )                
            {
            iConnectionInfo.Remove(i);
            delete conninfo;
            TRACE_INFO((_L("[HID]\tCGenericHid::Disconnected remove connection info %d removed"),i));
            }
        }     
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// From CHidTransport
// ConnectedL
// HID device has been connected.  Attempt to find a driver that can
// handle reports in the format specified by the report descriptor.
// ---------------------------------------------------------------------------
//
TInt CGenericHid::ConnectedL( TInt aConnectionId, const TDesC8& aDescriptor )
    {
    TRACE_INFO((_L("[HID]\tCGenericHid::ConnectedL(%d, ...)"), aConnectionId))    
    
    // Place the parsed report descriptor in the driver list item:
    
    TBool found = EFalse;
    CConnectionInfo* conninfo = SeekConnectionInfo( aConnectionId );
    if ( conninfo )
        {
        return KErrInUse;    
        }
    
    CReportRoot* reportRoot = iParser->ParseL( aDescriptor );
    CleanupStack::PushL(reportRoot);
        
    TRACE_INFO(_L("[HID]\tCGenericHid::ConnectedL: evaluate driver array"));
    // Implementation info array
    RImplInfoPtrArray implInfoArray;
    REComSession::ListImplementationsL( KHidDriverPluginInterfaceUid, implInfoArray );
    CleanupClosePushL(implInfoArray);    
    
    TRACE_INFO((_L("[HID]\tCGenericHid::ConnectedL: %d implementations found"), implInfoArray.Count()));
    TInt index = 0;
    TInt retVal = KErrHidNoDriver;
    TInt supportedfields = 0;
    CHidDriver* driver = NULL;
    for ( index  = 0; index < implInfoArray.Count(); index++ )
        {
        // parse implementation UID
        CImplementationInformation* info = implInfoArray[ index  ];
        TUid implUid = info->ImplementationUid();
        TRACE_INFO((_L("[HID]\tCGenericHid::ConnectedL: load plugin 0x%08x"),implUid ));
        // load driver
        // Trap so other drivers will be enumerated even if
        // this fails:

        TRAPD(retTrap, driver = CHidDriver::NewL( implUid, this ));
        if ( retTrap != KErrNone)
            {
            continue;    
            }
        CleanupStack::PushL(driver);
        TRACE_INFO((_L("[HID]\tCGenericHid::ConnectedL: init plugin 0x%08x"),implUid ));
        driver->InitialiseL( aConnectionId );
        TInt ret = driver->CanHandleReportL( reportRoot );
        if (ret == KErrNone)
            {
            TRACE_INFO(_L("[HID]\tCGenericHid::ConnectedL(): found driver"));            
	        
	        // Make a new driver list item:
	        CDriverListItem* driverItem = new ( ELeave ) CDriverListItem( aConnectionId );	        
	        CleanupStack::PushL( driverItem );	                
	        driver->SetInputHandlingReg( iInputHandlingReg );
	        supportedfields += driver->SupportedFieldCount();	        
            iDriverList.AddLast( *driverItem );
            CleanupStack::Pop( driverItem );
            driverItem->SetDriver( driver );    
            CleanupStack::Pop( driver );
            retVal = KErrNone;
            found = ETrue;
            }
        else
        	{
 	        CleanupStack::PopAndDestroy( driver );
        	}
        }
    TRACE_INFO((_L("[HID]\tCGenericHid::ConnectedL Partial supported hid device supported %d in report %d&"),supportedfields,iParser->FieldCount()));
    if (supportedfields < iParser->FieldCount() && found )
        {
        TRACE_INFO(_L("[HID]\tCGenericHid::ConnectedL Partial supported hid device"));
        }
    implInfoArray.ResetAndDestroy();     
    CleanupStack::PopAndDestroy();  // info
    if ( found )
        {   
        TRACE_INFO(_L("[HID]\tCGenericHid::ConnectedL append connection info"));    
        conninfo = CConnectionInfo::NewL(aConnectionId, reportRoot);
        CleanupStack::Pop(reportRoot); // ownership transfered to conninfo        
        CleanupStack::PushL(conninfo);
        iConnectionInfo.AppendL(conninfo);
        CleanupStack::Pop(conninfo);
        }         
    else
        {
        CleanupStack::PopAndDestroy(reportRoot);     
        }
    
    return retVal;
    }

// ---------------------------------------------------------------------------
// From CHidTransport
// DataIn
// Determine which driver is handling this connection ID and pass the payload
// reference to it
// ---------------------------------------------------------------------------
//
TInt CGenericHid::DataIn(TInt aConnectionId,
    CHidTransport::THidChannelType aChannel, const TDesC8& aPayload)
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrHidNoDriver;
    TInt ret = KErrNone;
    
    TSglQueIter<CDriverListItem> driverIter( iDriverList );
    driverIter.SetToFirst();    

    CDriverListItem* item = driverIter;
    TBool found = EFalse; 
    while ( item )
        {            
        if ( item->ConnectionId() == aConnectionId )
                {                
                ret = item->Driver()->DataIn( aChannel, aPayload );
                if (ret == KErrNone)
                    {
                    TRACE_INFO(_L("[HID]\tCGenericHid::DataIn command handled"));
                    found = ETrue;
                    retVal = KErrNone;
                    }
                }
         TRACE_INFO(_L("[HID]\tCGenericHid::DataIn next driver"));
         driverIter++;
         item = driverIter;
         }
    if ( !found && aChannel == CHidTransport::EHidChannelCtrl )
        {
        retVal = KErrNone;
        }
    iInputHandlingReg->Reset();    
    TRACE_FUNC_EXIT    
    return retVal;
    }


// ---------------------------------------------------------------------------
// ReportDescriptor
// Provides access to the parsed results to the factory
// ---------------------------------------------------------------------------
//
CReportRoot* CGenericHid::ReportDescriptor(TInt aConnectionId)
    {
    CConnectionInfo* conninfo = SeekConnectionInfo( aConnectionId );
    if ( conninfo )
        {
        return conninfo->ReportRoot();
        }
    return NULL;
    }

// ---------------------------------------------------------------------------
// From CHidTransport
// DriverActive()
// ---------------------------------------------------------------------------
//
TInt CGenericHid::DriverActive(TInt aConnectionId,
    CHidTransport::TDriverState aActive)
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrHidNoDriver;    
    // Find the driver handling the connection and stop it    
    TSglQueIter<CDriverListItem> driverIter( iDriverList );
    driverIter.SetToFirst();  
    CDriverListItem* item = driverIter;
    
    while ( item )
        {
        TRACE_INFO(_L("[HID]\tCGenericHid::DriverActive"));        
        if ( item->ConnectionId() == aConnectionId && item->Driver() )
            {
            TRACE_INFO(_L("[HID]\tCGenericHid::DriverActive driver found"));
            if ( aActive == CHidTransport::EActive )
                {
                TRAP(retVal, item->Driver()->StartL( aConnectionId ));
                if (retVal != KErrNone)
                    {
                    break;
                    }
                }
            else if ( aActive == CHidTransport::ESuspend)
                {
                item->Driver()->Stop();
                retVal = KErrNone;
                }            
            }   
         driverIter++;
         item = driverIter;
         }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// CommandResult()
// ---------------------------------------------------------------------------
//
void CGenericHid::CommandResult(TInt aConnectionId, TInt aCmdAck)
    {
    // Get the driver handling this connection    
    CConnectionInfo* conninfo = SeekConnectionInfo( aConnectionId );
    if ( conninfo )
        {
        CHidDriver*  hiddriver = conninfo->ReturnLastCommandHandler();
        if (hiddriver)
            {
            hiddriver->CommandResult(aCmdAck);
            }
        }    
    }

// ---------------------------------------------------------------------------
// SeekConnectionInfo()
// ---------------------------------------------------------------------------
//
CConnectionInfo* CGenericHid::SeekConnectionInfo(TInt aConnectionId)
    {
    TRACE_FUNC    
    CConnectionInfo* conninfo = NULL;
    TInt count = iConnectionInfo.Count();
    TRACE_INFO((_L("[HID]\tCGenericHid::SeekConnectionInfo count %d"), count));   
    for (TInt i = 0 ; i < count; i++)
        {
        conninfo = iConnectionInfo[i];
        TRACE_INFO((_L("[HID]\tCGenericHid::SeekConnectionInfo connection info check %d %d"),aConnectionId, conninfo->ConnectionID()));    
        if ( conninfo->ConnectionID() == aConnectionId)                
            {
            TRACE_INFO(_L("[HID]\tCGenericHid::SeekConnectionInfo connection info found"));    
            return conninfo;
            }
        }
    return NULL;
    }
