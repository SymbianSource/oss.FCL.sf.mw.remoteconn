// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

ABTester is both a client of the SBEngine and ABServer only for test purposes
*/

#ifndef __ABTESTER_H__
#define __ABTESTER_H__

#include <sbeclient.h>
#include <abclient.h>

namespace conn
	{
	class CABDataOwner;
	class CABData;
	
	const TInt KABTestABSID = 0x0AB7E57E;
	_LIT(KABTestPanicText, "ABTester");
	
	enum TABTestStep
		{
		EABGetListOfDataOwners,
		EABSetBackupMode,
		EABSetBaseBackupOwners,
		EABGetExpectedDataSize,
		EABGetBackupRegFile,
		EABGetABDOState,
		EABGetActiveSnapshot,
		EABGetActiveData,
		EABSetNormalMode,
		EABSetRestoreMode,
		EABSupplyRegData,
		EABSupplyActiveSnapshot,
		EABSupplyActiveData
		};
	
	/**
	This class owns the SBEClient and also owns the test ABClient
	
	It's responsible for running the test steps and storing the results
	*/
	class CABTester : public CActive
		{
	public:
		static CABTester* NewLC();
		void Run();
		~CABTester();
		
	// CActive
	protected:
		void DoCancel();
		void RunL();
		TInt RunError(TInt aError);
	
	private:
		CABTester() : CActive(EPriorityNormal), iSBEClient(NULL), iStepNumber(0), 
			iABSID(KABTestABSID), iTestDOI(NULL), iTestDOISID(NULL), iBackupRegFile(NULL),
			iActiveSnapshot(NULL), iActiveData(NULL), iABDOStatus(EUnset)
			{}
		void ConstructL();
		
	// The test steps
		void InitTestStepArray();
		void DoTestStepL(TABTestStep aStepId);
		void StepGetListOfDataOwnersL();
		void StepSetBaseBackupOwnersL();
		void Pass();
		void Fail(TInt aCode = KErrAbort);
		void GetABDOStateL();

	private:
		CSBEClient* iSBEClient;
		RArray<TABTestStep> iTestSteps;
		TInt iStepNumber;
		TSecureId iABSID;
		CDataOwnerInfo* iTestDOI;
		CSBSecureId* iTestDOISID;
		HBufC8* iBackupRegFile;
		HBufC8* iActiveSnapshot;
		HBufC8* iActiveData;
		TDataOwnerStatus iABDOStatus;
		};
	}

#endif //__ABTESTER_H__
