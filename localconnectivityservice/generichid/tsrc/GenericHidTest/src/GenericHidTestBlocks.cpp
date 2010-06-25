/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
*
*/




// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>
#include "GenericHidTest.h"

#include "hidreports.h"
#include "tGenericHIDAPI.h"
#include "tdialclient.h"
#include "alarmcontrol.h"
#include "timer.h"

// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGenericHidTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CGenericHidTest::Delete() 
    {
    if ( iGenericHIDTest )
		{
		delete iGenericHIDTest;
		iGenericHIDTest = NULL;
		}
        	
	if ( iDialClient )
		{
		delete iDialClient;
		iDialClient = NULL;
		}
	
	if ( iAlarmControl )
		{
		delete iAlarmControl;
		iAlarmControl = NULL;
		}
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove
		ENTRY( "ExecuteApiTestBlock", CGenericHidTest::ExecuteApiTestBlock ),
        ENTRY( "ExecuteModuleTestBlock", CGenericHidTest::ExecuteModuleTestBlock ),
        ENTRY( "ExecuteBranchTestBlock", CGenericHidTest::ExecuteBranchTestBlock ),
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CGenericHidTest::GetTestBlockParamsL
// -----------------------------------------------------------------------------

void CGenericHidTest::GetTestBlockParamsL( CStifItemParser& aItem )
    {
    STIF_LOG( ">>> GetTestBlockParamsL" );
    
    // Add new test block branches below, get all required test parameters    
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );
        User::LeaveIfError( aItem.GetNextInt( iTestBlockParams.iTestIntOption1 ) );        
        User::LeaveIfError( aItem.GetNextChar( iTestBlockParams.iTestCharOption1 ) );        
        }   
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Connect" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );       
        } 
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Disconnect" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DataIn" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Wait" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) ); 
        User::LeaveIfError( aItem.GetNextInt( iTestBlockParams.iTestIntOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateCall" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "AddAlarm" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );
        User::LeaveIfError( aItem.GetNextInt( iTestBlockParams.iTestIntOption1 ) );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DeleteAlarm" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CountryCode" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "VendorId" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ProductId" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "SetProtocol" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "GetProtocol" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "GetReport" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "SetReport" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );      
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );         
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DataOut" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );     
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "GetIdle" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "SetIdle" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CommandResult" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportDescriptor" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportGenerator" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DeleteReportGenerator" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportGeneratorReport" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportGeneratorSetField" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );     
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );      
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator_FieldNull" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );    
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator_NotArray" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator_FieldCountZero" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DeleteReportTranslator" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorGetValue" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorValue" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorGetUsageId" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );         
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorUsageId" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorRawValue" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorCount" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Collectiontype" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsPhysical" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsLogical" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsReport" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsNamedArray" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsUsageSwitch" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsUsageModifier" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsApplication" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionUsagePage" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionUsage" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionCollectionCount" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionFieldCount" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionCollectionByIndex" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );         
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionFieldByIndex" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportrootReportSizeBytes" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldLogicalMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldPhysicalMin" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldPhysicalMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUnit" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUnitExponent" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldCount" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSize" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldOffset" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldReportId" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsInReport" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldDesignatorIndex" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldStringIndex" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsagePage" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldHasUsage" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsageArray" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsage" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsageCount" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldClearUsageList" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldLastUsage" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );         
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldAttributes" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldType" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsVariable" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsArray" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsConstant" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsData" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetLogicalMin" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetLogicalMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetPhysicalMin" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetPhysicalMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetDesignatorMin" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetDesignatorMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetUsageMin" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetUsageMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetStringMin" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetStringMax" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetLogicalRange" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetUsageRange" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetPhysicalRange" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetStringRange" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetDesignatorRange" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsInput" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsOutput" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsFeature" ) ) )
        {          
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        }
    else
        {
        STIF_LOG( "GetTestBlockParamsL, Test type: not found" );
        User::Leave( KErrNotFound );
        }
    STIF_LOG( "<<< GetTestBlockParams" );
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ExecuteApiTestBlock
// -----------------------------------------------------------------------------

TInt CGenericHidTest::ExecuteApiTestBlock( CStifItemParser& aItem )
    {
	STIF_LOG( ">>>ExecuteApiTestBlock" );
	
	TInt res;
    TGenericHidTestResult testResult;
	
    TRAP( res, DoExecuteApiTestBlockL( aItem, testResult ) );
    if ( res != KErrNone )
        {
        STIF_LOG1( "DoExecuteApiTestBlockL error: %d", res );
        return res;
        }
    
    STIF_ASSERT_EQUALS( ETestCasePassed, testResult );
    STIF_LOG( "Test case passed" );
	STIF_LOG( "<<<ExecuteApiTestBlock" );
	
    return KErrNone;
    }
	
	
void CGenericHidTest::DoExecuteApiTestBlockL( CStifItemParser& aItem, TGenericHidTestResult& aTestResult )
    {
	STIF_LOG( ">>>DoExecuteApiTestBlock" );

	User::LeaveIfError( aItem.GetString( _L( "ExecuteApiTestBlock" ), iTestBlockParams.iTestBlockName ) );
	STIF_LOG1( "Api test type: %S", &iTestBlockParams.iTestBlockName );
	
	GetTestBlockParamsL( aItem );
	
	// Add new API test block branches with optional test parameters here	
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {      
        ExampleTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, 
                iTestBlockParams.iTestIntOption1, iTestBlockParams.iTestCharOption1, aTestResult );
        }
    else
        {
        STIF_LOG( "Test type: not found" );
        User::Leave( KErrNotFound );
        }
	
	STIF_LOG( "<<<DoExecuteApiTestBlockL" );
    }
	
// -----------------------------------------------------------------------------
// CGenericHidTest::ExecuteModuleTestBlock
// -----------------------------------------------------------------------------	

TInt CGenericHidTest::ExecuteModuleTestBlock( CStifItemParser& aItem )
    {
	STIF_LOG( "[STIF_LOG] >>>ExecuteModuleTestBlock" );
	
    TInt res;
    TGenericHidTestResult testResult;
    
    TRAP( res, DoExecuteModuleTestBlockL( aItem, testResult ) );
    if ( res != KErrNone )
        {
        STIF_LOG1( "DoExecuteModuleTestBlockL error: %d", res );
        return res;
        }
    
    STIF_ASSERT_EQUALS( ETestCasePassed, testResult );
    STIF_LOG( "[STIF_LOG] Test case passed" );
	STIF_LOG( "[STIF_LOG] <<<ExecuteModuleTestBlock" );
    return KErrNone;
    }	
	
	
void CGenericHidTest::DoExecuteModuleTestBlockL( CStifItemParser& aItem, TGenericHidTestResult& aTestResult )
    {
	STIF_LOG( "[STIF_LOG] >>>DoExecuteModuleTestBlockL" );
	
    User::LeaveIfError( aItem.GetString( _L( "ExecuteModuleTestBlock" ), iTestBlockParams.iTestBlockName ) );
    STIF_LOG1( "Module test type: %S", &iTestBlockParams.iTestBlockName );
    
    GetTestBlockParamsL( aItem );
    
    // Add new module test block branches with optional test parameters here   
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {      
        ExampleTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, 
                iTestBlockParams.iTestIntOption1, iTestBlockParams.iTestCharOption1, aTestResult );
        }
    else
        {
        STIF_LOG( "Test type: not found" );
        User::Leave( KErrNotFound );
        }
    
	STIF_LOG( "[STIF_LOG] <<<DoExecuteModuleTestBlockL" );
    }
	
// -----------------------------------------------------------------------------
// CGenericHidTest::ExecuteBranchTestBlock
// -----------------------------------------------------------------------------
	
TInt CGenericHidTest::ExecuteBranchTestBlock( CStifItemParser& aItem )
    {
	STIF_LOG( "[STIF_LOG] >>>ExecuteBranchTestBlock" );
	
    TInt res;
    TGenericHidTestResult testResult = ETestCaseFailed;
    
    TRAP( res, DoExecuteBranchTestBlockL( aItem, testResult ) );
    if ( res != KErrNone )
        {
        STIF_LOG1( "DoExecuteBranchTestBlockL error: %d", res );
        return res;
        }   
    
    STIF_ASSERT_EQUALS( ETestCasePassed, testResult );
    STIF_LOG( "[STIF_LOG] Test case passed" );
	STIF_LOG( "[STIF_LOG] <<<ExecuteBranchTestBlock" );
    return KErrNone;
    }

	
void CGenericHidTest::DoExecuteBranchTestBlockL( CStifItemParser& aItem, TGenericHidTestResult& aTestResult )
    {
	STIF_LOG( "[STIF_LOG] >>>DoExecuteBranchTestBlockL" );
	
    User::LeaveIfError( aItem.GetString( _L( "ExecuteBranchTestBlock" ), iTestBlockParams.iTestBlockName ) );
    STIF_LOG1( "Branch test type: %S", &iTestBlockParams.iTestBlockName );
    
    GetTestBlockParamsL( aItem );
    
    // Add new branch test block branches with optional test parameters here   
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {      
        ExampleTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, 
                iTestBlockParams.iTestIntOption1, iTestBlockParams.iTestCharOption1, aTestResult );
        }  
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Connect" ) ) )
        {      
        ConnectL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );
        }  
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Disconnect" ) ) )
        {      
        DisconnectL( iTestBlockParams.iTestOption1, aTestResult );
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DataIn" ) ) )
        {      
        DataIn( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Wait" ) ) )
        {          
        WaitL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestIntOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateCall" ) ) )
        {          
        CreateCallL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult ); 
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "AddAlarm" ) ) )
        {          
        AddAlarmL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestIntOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DeleteAlarm" ) ) )
        {          
        DeleteAlarm( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CountryCode" ) ) )
        {          
        CountryCodeL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "VendorId" ) ) )
        {          
        VendorIdL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ProductId" ) ) )
        {          
        ProductIdL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "SetProtocol" ) ) )
        {          
        SetProtocolL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "GetProtocol" ) ) )
        {          
        GetProtocoL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "GetReport" ) ) )
        {          
        GetReportL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "SetReport" ) ) )
        {          
        SetReportL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );         
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DataOut" ) ) )
        {          
        DataOutL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );   
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "GetIdle" ) ) )
        {          
        GetIdleL( iTestBlockParams.iTestOption1, aTestResult );  
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "SetIdle" ) ) )
        {          
        SetIdleL( iTestBlockParams.iTestOption1, aTestResult );   
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CommandResult" ) ) )
        {          
        CommandResultL( iTestBlockParams.iTestOption1, aTestResult );
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportDescriptor" ) ) )
        {          
        ReportDescriptorL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportGenerator" ) ) )
        {          
        CreateReportGeneratorL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DeleteReportGenerator" ) ) )
        {          
        DeleteReportGeneratorL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportGeneratorReport" ) ) )
        {          
        ReportGeneratorReportL( iTestBlockParams.iTestOption1, aTestResult ); 
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportGeneratorSetField" ) ) )
        {          
        ReportGeneratorSetFieldL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator" ) ) )
        {          
        CreateReportTranslatorL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );   
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator_FieldNull" ) ) )
        {          
        CreateReportTranslator_FieldNullL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );  
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator_NotArray" ) ) )
        {          
        CreateReportTranslator_NotArrayL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateReportTranslator_FieldCountZero" ) ) )
        {          
        CreateReportTranslator_FieldCountZeroL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "DeleteReportTranslator" ) ) )
        {          
        DeleteReportTranslatorL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorGetValue" ) ) )
        {          
        ReportTranslatorGetValueL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorValue" ) ) )
        {          
        ReportTranslatorValueL( iTestBlockParams.iTestOption1, aTestResult );    
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorGetUsageId" ) ) )
        {          
        ReportTranslatorGetUsageIdL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorUsageId" ) ) )
        {          
        ReportTranslatorUsageIdL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorRawValue" ) ) )
        {          
        ReportTranslatorRawValueL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportTranslatorCount" ) ) )
        {          
        ReportTranslatorCountL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "Collectiontype" ) ) )
        {          
        CollectiontypeL( iTestBlockParams.iTestOption1, aTestResult );   
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsPhysical" ) ) )
        {          
        CollectionIsPhysicalL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsLogical" ) ) )
        {          
        CollectionIsLogicalL( iTestBlockParams.iTestOption1, aTestResult );    
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsReport" ) ) )
        {          
        CollectionIsReportL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsNamedArray" ) ) )
        {          
        CollectionIsNamedArrayL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsUsageSwitch" ) ) )
        {          
        CollectionIsUsageSwitchL( iTestBlockParams.iTestOption1, aTestResult );   
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsUsageModifier" ) ) )
        {          
        CollectionIsUsageModifierL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionIsApplication" ) ) )
        {          
        CollectionIsApplicationL( iTestBlockParams.iTestOption1, aTestResult );   
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionUsagePage" ) ) )
        {          
        CollectionUsagePageL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionUsage" ) ) )
        {          
        CollectionUsageL( iTestBlockParams.iTestOption1, aTestResult );    
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionCollectionCount" ) ) )
        {          
        CollectionFieldCountL( iTestBlockParams.iTestOption1, aTestResult ); 
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionFieldCount" ) ) )
        {          
        CollectionFieldCountL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionCollectionByIndex" ) ) )
        {          
        CollectionCollectionByIndexL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CollectionFieldByIndex" ) ) )
        {          
        CollectionFieldByIndexL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ReportrootReportSizeBytes" ) ) )
        {          
        ReportrootReportSizeBytes( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldLogicalMax" ) ) )
        {          
        FieldLogicalMaxL( iTestBlockParams.iTestOption1, aTestResult );  
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldPhysicalMin" ) ) )
        {          
        FieldPhysicalMinL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldPhysicalMax" ) ) )
        {          
        FieldPhysicalMaxL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUnit" ) ) )
        {          
        FieldUnitL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUnitExponent" ) ) )
        {          
        FieldUnitExponentL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldCount" ) ) )
        {          
        FieldCountL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSize" ) ) )
        {          
        FieldSizeL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldOffset" ) ) )
        {          
        FieldOffsetL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldReportId" ) ) )
        {          
        FieldReportIdL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsInReport" ) ) )
        {          
        FieldIsInReportL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldDesignatorIndex" ) ) )
        {          
        FieldDesignatorIndexL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldStringIndex" ) ) )
        {          
        FieldStringIndexL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsagePage" ) ) )
        {          
        FieldUsagePageL( iTestBlockParams.iTestOption1, aTestResult );     
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldHasUsage" ) ) )
        {          
        FieldHasUsageL( iTestBlockParams.iTestOption1, aTestResult ); 
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsageArray" ) ) )
        {          
        FieldUsageArrayL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsage" ) ) )
        {          
        FieldUsageL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldUsageCount" ) ) )
        {          
        FieldUsageCountL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldClearUsageList" ) ) )
        {          
        FieldClearUsageListL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldLastUsage" ) ) )
        {          
        FieldLastUsageL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldAttributes" ) ) )
        {          
        FieldAttributesL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldType" ) ) )
        {          
        FieldTypeL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsVariable" ) ) )
        {          
        FieldIsVariableL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsArray" ) ) )
        {          
        FieldIsArrayL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsConstant" ) ) )
        {          
        FieldIsConstantL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsData" ) ) )
        {          
        FieldIsDataL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetLogicalMin" ) ) )
        {          
        FieldSetLogicalMinL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetLogicalMax" ) ) )
        {          
        FieldSetPhysicalMinL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetPhysicalMin" ) ) )
        {          
        FieldSetPhysicalMinL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetPhysicalMax" ) ) )
        {          
        FieldSetPhysicalMaxL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetDesignatorMin" ) ) )
        {          
        FieldSetDesignatorMinL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetDesignatorMax" ) ) )
        {          
        FieldSetUsageMinL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetUsageMin" ) ) )
        {          
        FieldSetUsageMinL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetUsageMax" ) ) )
        {          
        FieldSetUsageMaxL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetStringMin" ) ) )
        {          
        FieldSetStringMinL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetStringMax" ) ) )
        {          
        FieldSetStringMaxL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetLogicalRange" ) ) )
        {          
        FieldSetLogicalRangeL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetUsageRange" ) ) )
        {          
        FieldSetUsageRangeL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetPhysicalRange" ) ) )
        {          
        FieldSetPhysicalRangeL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetStringRange" ) ) )
        {          
        FieldSetStringRangeL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldSetDesignatorRange" ) ) )
        {          
        FieldSetDesignatorRangeL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsInput" ) ) )
        {          
        FieldIsInputL( iTestBlockParams.iTestOption1, aTestResult );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsOutput" ) ) )
        {          
        FieldIsOutputL( iTestBlockParams.iTestOption1, aTestResult );       
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "FieldIsFeature" ) ) )
        {          
        FieldIsFeatureL( iTestBlockParams.iTestOption1, aTestResult );      
        }
    else
        {
        STIF_LOG( "DoExecuteBranchTestBlockL; Test type: not found" );
        User::Leave( KErrNotFound );
        }
    
	STIF_LOG( "[STIF_LOG] <<<DoExecuteBranchTestBlock" );
    }

// Add test block methods implementation here
// -----------------------------------------------------------------------------
// CGenericHidTest::ExampleTestL
// -----------------------------------------------------------------------------

void CGenericHidTest::ExampleTestL( TPtrC aTestOption, TPtrC aTestSubOption, 
        TInt aTestIntOption, TInt aTestCharOption, TGenericHidTestResult& aTestResult )
    {
    STIF_LOG( ">>>ExampleTest" );
    
    if ( !aTestOption.Compare( _L( "API" ) ) )
        {
        STIF_LOG1( "Api test option: %S", &aTestOption );
        STIF_LOG1( "Api test sub-option: %S", &aTestSubOption );
        STIF_LOG1( "Api test int option: %d", aTestIntOption );
        STIF_LOG1( "Api test char option: %c", aTestCharOption );
        }
    else if ( !aTestOption.Compare( _L( "MODULE" ) ) )
        {
        STIF_LOG1( "Module test option: %S", &aTestOption );
        STIF_LOG1( "Module test sub-option: %S", &aTestSubOption );
        STIF_LOG1( "Module test int option: %d", aTestIntOption );
        STIF_LOG1( "Module test char option: %c", aTestCharOption );
        }
    else if ( !aTestOption.Compare( _L( "BRANCH" ) ) )
        {
        STIF_LOG1( "Branch test option: %S", &aTestOption );
        STIF_LOG1( "Branch test sub-option: %S", &aTestSubOption );
        STIF_LOG1( "Branch test int option: %d", aTestIntOption );
        STIF_LOG1( "Branch test char option: %c", aTestCharOption );
        }
    else
        {
        STIF_LOG( "Invalid test parameter" );
        User::Leave( KErrNotFound );
        }
    
    aTestResult = ETestCasePassed;
    
    STIF_LOG( "<<<ExampleTest" );
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ConnectL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::ConnectL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {
    STIF_LOG( "Connect" );
    
    if ( !iGenericHIDTest )
        {
        iGenericHIDTest = CtGenericHIDAPI::NewL();
        }
    HBufC8* report = ConvertArray(aTestSubOption); 
    
    CleanupStack::PushL( report );
    iGenericHIDTest->ConnectL(0,*report);
    CleanupStack::PopAndDestroy();
    
    STIF_LOG( "Connect passed" );
    
    aTestResult = ETestCasePassed;
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::DisconnectL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::DisconnectL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
   {
   iGenericHIDTest->DisconnectL(0);   
   delete iGenericHIDTest;   
   iGenericHIDTest = NULL;
   
   STIF_LOG( "Disconnect passed" );
   
   aTestResult = ETestCasePassed;
   
   return KErrNone;
   }

// -----------------------------------------------------------------------------
// CGenericHidTest::DataIn
// -----------------------------------------------------------------------------
//  
TInt CGenericHidTest::DataIn( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult ) 
   {
   TInt retVal = KErrNone;
   HBufC8* report = ConvertArray(aTestSubOption);
   TInt firstByte = (*report)[0];
   STIF_LOG1("aTestSubOption >> report: %d", firstByte);
   
   retVal = iGenericHIDTest->DataInL(0,*report);
   STIF_LOG1( "DataInL return value: %d" , retVal);
   
   delete report;
   report = NULL;
   
   STIF_LOG( "Test case passed" );
   
   aTestResult = ETestCasePassed;
   
   return retVal;
   }

// -----------------------------------------------------------------------------
// CGenericHidTest::Wait
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::WaitL( TPtrC aTestOption, TInt aTestIntOption, TGenericHidTestResult& aTestResult )
    {
    CWaitTimer* timer = CWaitTimer::NewLC(aTestIntOption * 1000000);
    CleanupStack::PopAndDestroy(timer);
    
    STIF_LOG( "Test case passed" );
    
    aTestResult = ETestCasePassed;
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CreateCallL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CreateCallL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {
    TInt err = KErrNone;
    
    if ( iDialClient )
        {
        delete iDialClient;
        iDialClient = NULL;
        }
    iDialClient = CDialClient::NewL();     
    
    iDialClient->CreateCall( aTestSubOption );
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return err;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::AddAlarm
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::AddAlarmL( TPtrC aTestOption, TInt aTestIntOption, TGenericHidTestResult& aTestResult  )
    {
    STIF_LOG( ">>AddAlarmL" ); 

    if ( !iAlarmControl )
        {
        iAlarmControl = CAlarmControl::NewL( iLog );
        }

    STIF_LOG( "Create alarm" ); 
    
    iAlarmControl->CreateClockAlarm( aTestIntOption );

    STIF_LOG( "Test case passed" ); 
    
    aTestResult = ETestCasePassed;
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::DeleteAlarm
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::DeleteAlarm( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {    
    iAlarmControl->DeleteAlarm();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CountryCodeL
// -----------------------------------------------------------------------------
//    
TInt CGenericHidTest::CountryCodeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->RunCountryCodeL();
    STIF_LOG2( "RunCountryCodeL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::VendorIdL
// -----------------------------------------------------------------------------
//            
TInt CGenericHidTest::VendorIdL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->RunVendorIdL();
    STIF_LOG2( "RunVendorIdL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ProductIdL
// -----------------------------------------------------------------------------
//            
TInt CGenericHidTest::ProductIdL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->RunProductIdL();  
    STIF_LOG2( "RunProductIdL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::SetProtocol
// -----------------------------------------------------------------------------
//            
TInt CGenericHidTest::SetProtocolL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->RunSetProtocolL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
       
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::GetProtocoL
// -----------------------------------------------------------------------------
//            
TInt CGenericHidTest::GetProtocoL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->RunGetProtocoL(); 
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
       
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::GetReportL
// -----------------------------------------------------------------------------
//            
TInt CGenericHidTest::GetReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->RunGetReportL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CGenericHidTest::SetReportL
// -----------------------------------------------------------------------------
//            
TInt CGenericHidTest::SetReportL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    { 
    HBufC8* report = ConvertArray(aTestSubOption);    
    CleanupStack::PushL( report );
    iGenericHIDTest->RunSetReportL( *report );  
    CleanupStack::PopAndDestroy();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }            
 
// -----------------------------------------------------------------------------
// CGenericHidTest::DataOutL
// -----------------------------------------------------------------------------
//        
TInt CGenericHidTest::DataOutL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {   
    HBufC8* report = ConvertArray(aTestSubOption);    
    CleanupStack::PushL( report );        
    iGenericHIDTest->RunDataOutL( *report );        
    CleanupStack::PopAndDestroy();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    } 

// -----------------------------------------------------------------------------
// CGenericHidTest::GetIdleL
// -----------------------------------------------------------------------------
//          
TInt CGenericHidTest::GetIdleL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->GetGetIdleL(); 
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
       
    return KErrNone;
    } 
    
// -----------------------------------------------------------------------------
// CGenericHidTest::SetIdleL
// -----------------------------------------------------------------------------
//           
TInt CGenericHidTest::SetIdleL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->RunSetIdleL(); 
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
       
    return KErrNone;
    } 
// -----------------------------------------------------------------------------
// CGenericHidTest::CommandResultL
// -----------------------------------------------------------------------------
//           
TInt CGenericHidTest::CommandResultL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->GetCommandResultL();  
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    } 
// -----------------------------------------------------------------------------
// CGenericHidTest::ReportDescriptorL
// -----------------------------------------------------------------------------
//           
TInt CGenericHidTest::ReportDescriptorL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->GetReportDescriptorL();  
    STIF_LOG2( "GetReportDescriptorL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
        
// -----------------------------------------------------------------------------
// CGenericHidTest::ReportDescriptorL
// -----------------------------------------------------------------------------
//           
TInt CGenericHidTest::CreateReportGeneratorL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  )
    {
    TInt ret = iGenericHIDTest->CreateReportGeneratorL();
    STIF_LOG2( "CreateReportGeneratorL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }        
    
// -----------------------------------------------------------------------------
// CGenericHidTest::ReportDescriptorL
// -----------------------------------------------------------------------------
//           
TInt CGenericHidTest::DeleteReportGeneratorL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  )
    {
    iGenericHIDTest->DeleteReportGeneratorL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }        

// -----------------------------------------------------------------------------
// CGenericHidTest::ReportDescriptorL
// -----------------------------------------------------------------------------
//      
TInt CGenericHidTest::ReportGeneratorReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  )
    {
    iGenericHIDTest->ReportGeneratorReport();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CGenericHidTest::ReportGeneratorSetFieldL
// -----------------------------------------------------------------------------
//      
TInt CGenericHidTest::ReportGeneratorSetFieldL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret;
    TInt expectedResult = KErrNone;
    
	if ( !aTestSubOption.Compare( _L( "Array" ) ) )
		{
		iLog -> Log( _L("Test type: Array") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( ETrue, EFalse, EFalse, EFalse, ETrue, EFalse );
		}
	else if ( !aTestSubOption.Compare( _L( "NotArray" ) ) )
		{
		iLog -> Log( _L("Test type: NotArray") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( EFalse, EFalse, EFalse, EFalse, ETrue, EFalse );
		}
	else if ( !aTestSubOption.Compare( _L( "NotArrayOutOfRange" ) ) )
		{
		iLog -> Log( _L("Test type: NotArrayOutOfRange") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( EFalse, ETrue, EFalse, EFalse, ETrue, EFalse );
		expectedResult = KErrValueOutOfRange;
		}
	else if ( !aTestSubOption.Compare( _L( "FieldNull" ) ) )
		{
		iLog -> Log( _L("Test type: FieldNull") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( EFalse, EFalse, ETrue, EFalse, ETrue, EFalse );
		expectedResult = KErrUsageNotFound;
		}
	else if ( !aTestSubOption.Compare( _L( "FieldCountZero" ) ) )
		{
		iLog -> Log( _L("Test type: FieldCountZero") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( ETrue, EFalse, EFalse, ETrue, ETrue, EFalse );
		expectedResult = KErrNoSpaceInArray;
		}
	else if ( !aTestSubOption.Compare( _L( "NotExistingUsage" ) ) )
		{
		iLog -> Log( _L("Test type: NotExistingUsage") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( ETrue, EFalse, EFalse, ETrue, EFalse, EFalse );//getIndexOfUsage
		expectedResult = KErrUsageNotFound;
		}
	else if ( !aTestSubOption.Compare( _L( "NotExistingUsage2" ) ) )
		{
		iLog -> Log( _L("Test type: NotExistingUsage2") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( ETrue, EFalse, EFalse, EFalse, EFalse, EFalse );//getIndexOfUsage
		expectedResult = KErrUsageNotFound;
		}
	else if ( !aTestSubOption.Compare( _L( "BadIndex" ) ) )
		{
		iLog -> Log( _L("Test type: BadIndex") );
		ret = iGenericHIDTest->ReportGeneratorSetFieldL( EFalse, EFalse, EFalse, EFalse, ETrue, ETrue );
		expectedResult = KErrBadControlIndex;
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}
	

    STIF_LOG2( "ReportGeneratorSetFieldL return value: %d, expected: %d" , ret, expectedResult);
    STIF_ASSERT_EQUALS(ret, expectedResult);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    
    return KErrNone;    
    }   

// -----------------------------------------------------------------------------
// CGenericHidTest::CreateReportTranslatorL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::CreateReportTranslatorL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {   
    HBufC8* report = ConvertArray(aTestSubOption);    
    CleanupStack::PushL( report ); 
    iGenericHIDTest->CreateReportTranslatorL( *report , ETrue, EFalse, EFalse);  
    CleanupStack::PopAndDestroy();

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::CreateReportTranslator_FieldNullL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {   
    HBufC8* report = ConvertArray(aTestSubOption);    
    CleanupStack::PushL( report ); 
    iGenericHIDTest->CreateReportTranslatorL( *report, EFalse, ETrue, EFalse );  
    CleanupStack::PopAndDestroy();

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::CreateReportTranslator_NotArrayL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    { 
    HBufC8* report = ConvertArray(aTestSubOption);    
    CleanupStack::PushL( report ); 
    iGenericHIDTest->CreateReportTranslatorL( *report , EFalse, EFalse, EFalse);  
    CleanupStack::PopAndDestroy();

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::CreateReportTranslator_FieldCountZeroL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {   
    HBufC8* report = ConvertArray(aTestSubOption);    
    CleanupStack::PushL( report ); 
    iGenericHIDTest->CreateReportTranslatorL( *report , ETrue, EFalse, ETrue);  
    CleanupStack::PopAndDestroy();

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CGenericHidTest::DeleteReportTranslatorL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::DeleteReportTranslatorL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->DeleteReportTranslatorL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::ReportTranslatorGetValueL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret;
    TInt expectedResult = KErrNone;
    
	if ( !aTestSubOption.Compare( _L( "Ok" ) ) )
		{
		iLog -> Log( _L("Test type: Ok") );
		}
	else if ( !aTestSubOption.Compare( _L( "NotFound" ) ) )
		{
		iLog -> Log( _L("Test type: NotFound") );
		expectedResult = KErrUsageNotFound;
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}
    
    ret = iGenericHIDTest->ReportTranslatorGetValueL();
    STIF_LOG2( "ReportTranslatorGetValueL return value: %d, expected: %d" , ret, expectedResult);
    STIF_ASSERT_EQUALS(ret, expectedResult);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ReportTranslatorValueL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::ReportTranslatorValueL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->ReportTranslatorValueL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ReportTranslatorGetUsageIdL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::ReportTranslatorGetUsageIdL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret;
    TInt expectedResult = KErrNone;
    TInt badIndex = EFalse;
    
	if ( !aTestSubOption.Compare( _L( "Ok" ) ) )
		{
		iLog -> Log( _L("Test type: Ok") );
		}
	else if ( !aTestSubOption.Compare( _L( "Error" ) ) )
		{
		iLog -> Log( _L("Test type: Error") );
		expectedResult = KErrNoMemory;
		}
	else if ( !aTestSubOption.Compare( _L( "BadIndex" ) ) )
		{
		iLog -> Log( _L("Test type: BadIndex") );
		badIndex = ETrue;
		expectedResult = KErrBadControlIndex;
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}
    
    ret = iGenericHIDTest->ReportTranslatorGetUsageIdL( badIndex );
    STIF_LOG2( "ReportTranslatorGetUsageIdL return value: %d, expected: %d" , ret, expectedResult);
    STIF_ASSERT_EQUALS(ret, expectedResult);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ReportTranslatorUsageIdL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::ReportTranslatorUsageIdL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->ReportTranslatorUsageIdL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ReportTranslatorRawValueL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::ReportTranslatorRawValueL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->ReportTranslatorRawValueL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ReportTranslatorCountL
// -----------------------------------------------------------------------------
//   
TInt CGenericHidTest::ReportTranslatorCountL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult)
    {    
    TInt zeroExpected = EFalse;
    
	if ( !aTestSubOption.Compare( _L( "Zero" ) ) )
		{
		iLog -> Log( _L("Test type: Zero") );
		zeroExpected = ETrue;
		}
	else if ( !aTestSubOption.Compare( _L( "NotZero" ) ) )
		{
		iLog -> Log( _L("Test type: NotZero") );
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}
    
    TInt count = iGenericHIDTest->ReportTranslatorCountL();
    
    STIF_LOG1( "ReportTranslatorCountL return value: %d" , count);
    if(zeroExpected != EFalse)
    	{
    	STIF_ASSERT_EQUALS(0, count)
    	}

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CollectiontypeL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectiontypeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectiontypeL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CollectionIsPhysicalL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectionIsPhysicalL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsPhysicalL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CollectionIsLogicalL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectionIsLogicalL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsLogicalL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CollectionIsReportL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectionIsReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsReportL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CollectionIsNamedArrayL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectionIsNamedArrayL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsNamedArrayL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::CollectionIsUsageSwitchL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectionIsUsageSwitchL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsUsageSwitchL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::CollectionIsUsageModifierL
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::CollectionIsUsageModifierL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsUsageModifierL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

TInt CGenericHidTest::CollectionIsApplicationL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionIsApplicationL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }


TInt CGenericHidTest::CollectionUsagePageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionUsagePageL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

TInt CGenericHidTest::CollectionUsageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionUsageL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

TInt CGenericHidTest::CollectionCollectionCountL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionCollectionCountL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

TInt CGenericHidTest::CollectionFieldCountL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->CollectionFieldCountL();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }

TInt CGenericHidTest::CollectionCollectionByIndexL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret;
    
	if ( !aTestSubOption.Compare( _L( "Present" ) ) )
		{
		iLog -> Log( _L("Test type: Present") );
		ret = iGenericHIDTest->CollectionCollectionByIndexL(ETrue);
		}
	else if ( !aTestSubOption.Compare( _L( "NotPresent" ) ) )
		{
		iLog -> Log( _L("Test type: NotPresent") );
		ret = iGenericHIDTest->CollectionCollectionByIndexL(EFalse);
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}

    STIF_LOG2( "CollectionCollectionByIndexL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::CollectionFieldByIndexL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->CollectionFieldByIndexL();
    STIF_LOG2( "CollectionFieldByIndexL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;   
    }


// -----------------------------------------------------------------------------
// CGenericHidTest::ReportrootReportSizeBytes
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::ReportrootReportSizeBytes( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    iGenericHIDTest->ReportrootReportSizeBytes();
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;    
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::FieldLogicalMax
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldLogicalMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->FieldLogicalMaxL();
    STIF_LOG2( "FieldLogicalMaxL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::FieldPhysicalMin
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldPhysicalMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->FieldPhysicalMinL();
    STIF_LOG2( "FieldPhysicalMinL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::FieldPhysicalMax
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldPhysicalMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldPhysicalMaxL();
    STIF_LOG2( "FieldPhysicalMaxL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::FieldUnit
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldUnitL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldUnitL();
    STIF_LOG2( "FieldUnitL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::FieldUnitExponent
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldUnitExponentL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldUnitExponentL();
    STIF_LOG2( "FieldUnitExponentL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldCountL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldCountL();
    STIF_LOG2( "FieldCountL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSizeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSizeL();
    STIF_LOG2( "FieldSizeL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldOffsetL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {  
    TInt ret;
    
	if ( !aTestSubOption.Compare( _L( "SetReport" ) ) )
		{
		iLog -> Log( _L("Test type: SetReport") );
		ret = iGenericHIDTest->FieldOffsetL(ETrue);
		}
	else if ( !aTestSubOption.Compare( _L( "NotSetReport" ) ) )
		{
		iLog -> Log( _L("Test type: NotSetReport") );
		ret = iGenericHIDTest->FieldOffsetL(EFalse);
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}

    STIF_LOG2( "FieldOffsetL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldReportIdL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldReportIdL();
    STIF_LOG2( "FieldReportIdL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldIsInReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsInReportL();
    STIF_LOG2( "FieldIsInReportL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldDesignatorIndexL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldDesignatorIndexL();
    STIF_LOG2( "FieldDesignatorIndexL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldStringIndexL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldStringIndexL();
    STIF_LOG2( "FieldStringIndexL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldUsagePageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldUsagePageL();
    STIF_LOG2( "FieldUsagePageL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CGenericHidTest::FieldHasUsage
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldHasUsageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldHasUsageL();
    STIF_LOG2( "FieldHasUsageL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldUsageArrayL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldUsageArrayL();
    STIF_LOG2( "FieldUsageArrayL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldUsageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldUsageL();
    STIF_LOG2( "FieldUsageL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldUsageCountL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldUsageCountL();
    STIF_LOG2( "FieldUsageCountL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldClearUsageListL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldClearUsageListL();
    STIF_LOG2( "FieldClearUsageListL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldLastUsageL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult )
    {        
    TInt ret;

	if ( !aTestSubOption.Compare( _L( "Empty" ) ) )
		{
		iLog -> Log( _L("Test type: Empty") );
		ret = iGenericHIDTest->FieldLastUsageL( ETrue );
		}
	else if ( !aTestSubOption.Compare( _L( "NotEmpty" ) ) )
		{
		iLog -> Log( _L("Test type: NotEmpty") );
		ret = iGenericHIDTest->FieldLastUsageL( EFalse );
		}
	else
		{
		iLog -> Log( _L("Test type: not supported") );
		return KErrNotFound;
		}

    STIF_LOG2( "FieldLastUsageL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CGenericHidTest::FieldAttributes
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldAttributesL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {
    TInt ret = iGenericHIDTest->FieldAttributesL();
    STIF_LOG2( "FieldAttributesL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);

    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldType
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldTypeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldTypeL();
    STIF_LOG2( "FieldTypeL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldIsVariable
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldIsVariableL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsVariableL();
    STIF_LOG2( "FieldIsVariableL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldIsArrayL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsArrayL();
    STIF_LOG2( "FieldIsArrayL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldIsConstant
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldIsConstantL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsConstantL();
    STIF_LOG2( "FieldIsConstantL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldIsDataL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsDataL();
    STIF_LOG2( "FieldIsDataL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetLogicalMin
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetLogicalMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetLogicalMinL();
    STIF_LOG2( "FieldSetLogicalMinL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetLogicalMax
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetLogicalMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetLogicalMaxL();
    STIF_LOG2( "FieldSetLogicalMaxL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetPhysicalMin
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetPhysicalMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetPhysicalMinL();
    STIF_LOG2( "FieldSetPhysicalMinL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetPhysicalMax
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetPhysicalMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetPhysicalMaxL();
    STIF_LOG2( "FieldSetPhysicalMaxL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetDesignatorMin
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetDesignatorMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetDesignatorMinL();
    STIF_LOG2( "FieldSetDesignatorMinL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }
// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetDesignatorMax
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetDesignatorMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetDesignatorMaxL();
    STIF_LOG2( "FieldSetDesignatorMaxL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetUsageMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetUsageMinL();
    STIF_LOG2( "FieldSetUsageMinL return value: %d, expected: %d" , ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetUsageMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetUsageMaxL();
    STIF_LOG2( "FieldSetUsageMaxL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetStringMin
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetStringMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetStringMinL();
    STIF_LOG2( "FieldSetStringMinL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldSetStringMax
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldSetStringMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetStringMaxL();
    STIF_LOG2( "FieldSetStringMaxL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetLogicalRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetLogicalRangeL();
    STIF_LOG2( "FieldSetLogicalRangeL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
        
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetUsageRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetUsageRangeL();
    STIF_LOG2( "FieldSetUsageRangeL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetPhysicalRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetPhysicalRangeL();
    STIF_LOG2( "FieldSetPhysicalRangeL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetStringRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetStringRangeL();
    STIF_LOG2( "FieldSetStringRangeL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldSetDesignatorRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldSetDesignatorRangeL();
    STIF_LOG2( "FieldSetDesignatorRangeL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

TInt CGenericHidTest::FieldIsInputL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsInputL();
    STIF_LOG2( "FieldIsInputL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldIsOutput
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldIsOutputL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsOutputL();
    STIF_LOG2( "FieldIsOutputL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::FieldIsFeature
// -----------------------------------------------------------------------------
//
TInt CGenericHidTest::FieldIsFeatureL( TPtrC aTestOption, TGenericHidTestResult& aTestResult )
    {            
    TInt ret = iGenericHIDTest->FieldIsFeatureL();
    STIF_LOG2( "FieldIsFeatureL return value: %d, expected: %d", ret, KErrNone);
    STIF_ASSERT_EQUALS(ret, KErrNone);
    
    aTestResult = ETestCasePassed;

    STIF_LOG( "Test case passed" );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ConvSingleItem
// -----------------------------------------------------------------------------
//
TUint CGenericHidTest::ConvSingleItem(TBuf8<2>& singleItem)
    {
    TLex8 lex(singleItem);
    TUint value;    
    lex.Val(value,EHex);
    singleItem.Delete(0,2);    
    return value;
    }

// -----------------------------------------------------------------------------
// CGenericHidTest::ConvertArray
// -----------------------------------------------------------------------------
//
HBufC8* CGenericHidTest::ConvertArray( const TPtrC& aFilename )
   {
   RFs theFs; 
   
   RFile someFile;
   if(theFs.Connect() != KErrNone)
    {
    return NULL;    
    }
   TInt error = someFile.Open(theFs, aFilename, EFileShareExclusive);

   RArray<TUint> arrDesc;       
   if (!error)
       {
       TBool seekEOL = EFalse;
       TBuf8<2> singleItem;

       TInt hexByte = 0;

       TBool bTerminator = EFalse;

       TBuf8<1>   oneChar;
       someFile.Read(oneChar, 1);

       while ( (!error) && (oneChar.Size()) )
           {
           switch ( oneChar[0] )
               {
               case 10:
               case 13:
                   seekEOL = EFalse;
                   break;

               case '/':
                   seekEOL = ETrue;
                   // Comments are special case terminators
                   // interntional drop through to the next case line
               case ' ':
               case '\t':
               case ',':
                   // whitespace - ignore, unless we've started storing a value
                   // (in which case treat the char as a terminator)
                   bTerminator = ETrue;
                   break;

               default:
                   if (!seekEOL)
                       {
                       if (4==hexByte)
                           {
                           error = KErrGeneral;    // Can't store more than two digits
                           }
                       else if (hexByte>1)
                           {
                           singleItem.Append(oneChar);
                           }

                       hexByte++;
                       }
                   break;
                   }

               if ( bTerminator && singleItem.Size() )
                   {
                   TUint value = ConvSingleItem(singleItem);
                   arrDesc.Append(value);
                   hexByte = 0;
                   }

               bTerminator = EFalse;

               someFile.Read(oneChar, 1);
               }

           if (singleItem.Size())
               {
               TUint value = ConvSingleItem(singleItem);
               arrDesc.Append(value);
               }

           someFile.Close();
           }

       theFs.Close();

       HBufC8* theDesc = 0;
       
       if (!error)
           {
           TInt size = arrDesc.Count();
           TRAPD( error, theDesc = HBufC8::NewL(size) );
           if ( error != KErrNone )
                {
                return NULL;    
                }
           TPtr8 des = theDesc->Des();
           for(TInt x=0; x<size; x++)
               {
               des.Append(TUint8(arrDesc[x]));
               }
           }
   arrDesc.Close();
   return theDesc;   
   }
// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
