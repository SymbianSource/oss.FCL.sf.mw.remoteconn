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




#ifndef GENERICHIDTEST_H
#define GENERICHIDTEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <TestclassAssert.h>

#include "hidparser.h"

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

#ifdef STIF_LOG
#undef STIF_LOG
#endif

#define STIF_LOG( s )\
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ) );\
    RDebug::Print( traceBuf );\
    }

#define STIF_LOG1( s, v ) \
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ), v );\
    RDebug::Print( traceBuf, v );\
    }

#define STIF_LOG2( s, v1, v2 ) \
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ), v1, v2 );\
    RDebug::Print( traceBuf, v1, v2 );\
    }

#define STIF_LOG3( s, v1, v2, v3 ) \
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ), v1, v2, v3 );\
    RDebug::Print( traceBuf, v1, v2, v3 );\
    }

// Logging path
//_LIT( KGenericHidTestLogPath, "\\logs\\testframework\\GenericHidTest\\" );

// Logging path for ATS - for phone builds comment this line
_LIT( KGenericHidTestLogPath, "e:\\testing\\stiflogs\\" );

// Log file
_LIT( KGenericHidTestLogFile, "GenericHidTest.txt" ); 
_LIT( KGenericHidTestLogFileWithTitle, "GenericHidTest_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CGenericHidTest;

class CHidTestLogger;
class CtGenericHIDAPI;
//class CMediaControl;
class CDialClient;  
class CAlarmControl;

// DATA TYPES
//enum ?declaration

enum TGenericHidTestResult
    {
    ETestCasePassed,
    ETestCaseFailed
    };

//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

NONSHARABLE_CLASS( TGenericHidTestBlockParams )
    {
    public:
        TPtrC iTestBlockName;
        
        TPtrC iTestOption1;
        TPtrC iTestOption2;
        TPtrC iTestOption3;
        
        TInt iTestIntOption1;
        TInt iTestIntOption2;
        
        TChar iTestCharOption1;
        TChar iTestCharOption2;
    };

/**
*  CGenericHidTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS( CGenericHidTest ) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGenericHidTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CGenericHidTest();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

        /**
        * From ?base_class ?member_description
        */
        //?type ?member_function();

    private:

        /**
        * C++ default constructor.
        */
        CGenericHidTest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below. 
        */

        virtual TInt ExecuteApiTestBlock( CStifItemParser& aItem );
        virtual TInt ExecuteModuleTestBlock( CStifItemParser& aItem );
        virtual TInt ExecuteBranchTestBlock( CStifItemParser& aItem );
        
        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove

        void GetTestBlockParamsL( CStifItemParser& aItem );
        
    	void DoExecuteApiTestBlockL( CStifItemParser& aItem, TGenericHidTestResult& aTestResult );    	
    	void DoExecuteModuleTestBlockL( CStifItemParser& aItem, TGenericHidTestResult& aTestResult );    
    	void DoExecuteBranchTestBlockL( CStifItemParser& aItem, TGenericHidTestResult& aTestResult );
    	
        void ExampleTestL( TPtrC aTestOption, TPtrC aTestSubOption, 
                 TInt aTestIntOption, TInt aTestCharOption, TGenericHidTestResult& aTestResult );
        
        virtual TInt ConnectL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );							  
		virtual TInt DisconnectL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt DataIn( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult  );
		virtual TInt WaitL( TPtrC aTestOption, TInt aTestIntOption, TGenericHidTestResult& aTestResult );		
		virtual TInt CreateCallL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt AddAlarmL( TPtrC aTestOption, TInt aTestIntOption, TGenericHidTestResult& aTestResult  );		
		virtual TInt DeleteAlarm( TPtrC aTestOption, TGenericHidTestResult& aTestResult );	
		virtual TInt CountryCodeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt VendorIdL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ProductIdL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt SetProtocolL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt GetProtocoL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt GetReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt SetReportL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );			   
		virtual TInt DataOutL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult);		
		virtual TInt GetIdleL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt SetIdleL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		
		virtual TInt CommandResultL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportDescriptorL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt CreateReportGeneratorL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt DeleteReportGeneratorL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportGeneratorReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportGeneratorSetFieldL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt CreateReportTranslatorL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );	
		virtual TInt CreateReportTranslator_FieldNullL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt CreateReportTranslator_NotArrayL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt CreateReportTranslator_FieldCountZeroL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt DeleteReportTranslatorL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportTranslatorGetValueL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportTranslatorValueL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );		
		virtual TInt ReportTranslatorGetUsageIdL(  TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportTranslatorUsageIdL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportTranslatorRawValueL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		virtual TInt ReportTranslatorCountL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );
		
		virtual TInt CollectiontypeL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionIsPhysicalL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt CollectionIsLogicalL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionIsReportL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt CollectionIsNamedArrayL(   TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt CollectionIsUsageSwitchL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionIsUsageModifierL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );		
		
		virtual TInt CollectionIsApplicationL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionUsagePageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionUsageL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionCollectionCountL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt CollectionFieldCountL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt CollectionCollectionByIndexL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );
		virtual TInt CollectionFieldByIndexL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );		
		
		virtual TInt ReportrootReportSizeBytes(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		
		virtual TInt FieldLogicalMaxL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldPhysicalMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldPhysicalMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldUnitL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldUnitExponentL(   TPtrC aTestOption, TGenericHidTestResult& aTestResult);
		
		virtual TInt FieldCountL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSizeL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldOffsetL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldReportIdL(   TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldIsInReportL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldDesignatorIndexL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldStringIndexL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldUsagePageL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		
		virtual TInt FieldHasUsageL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		
		virtual TInt FieldUsageArrayL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldUsageL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldUsageCountL( TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldClearUsageListL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldLastUsageL( TPtrC aTestOption, TPtrC aTestSubOption, TGenericHidTestResult& aTestResult );
		
		virtual TInt FieldAttributesL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldTypeL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldIsVariableL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldIsArrayL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldIsConstantL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldIsDataL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetLogicalMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldSetLogicalMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetPhysicalMinL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetPhysicalMaxL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetDesignatorMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldSetDesignatorMaxL( TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetUsageMinL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldSetUsageMaxL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldSetStringMinL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldSetStringMaxL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		
		virtual TInt FieldSetLogicalRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldSetUsageRangeL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetPhysicalRangeL( TPtrC aTestOption, TGenericHidTestResult& aTestResult   );
		virtual TInt FieldSetStringRangeL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldSetDesignatorRangeL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		virtual TInt FieldIsInputL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
		
		virtual TInt FieldIsOutputL(   TPtrC aTestOption, TGenericHidTestResult& aTestResult );
		virtual TInt FieldIsFeatureL(  TPtrC aTestOption, TGenericHidTestResult& aTestResult  );
					   
		HBufC8* ConvertArray(const TPtrC& aFilename);		
		TUint ConvSingleItem(TBuf8<2>& singleItem);
		
    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data
        TGenericHidTestBlockParams iTestBlockParams;

        CtGenericHIDAPI*    iGenericHIDTest;
        CDialClient*        iDialClient;
        CAlarmControl*      iAlarmControl;
        
        CParser* iParser;
        // Reserved pointer for future extension
        //TAny* iReserved;

    public:     // Friend classes
        //?friend_class_declaration;
    protected:  // Friend classes
        //?friend_class_declaration;
    private:    // Friend classes
        //?friend_class_declaration;
        
    };

#endif      // GENERICHIDTEST_H

// End of File
