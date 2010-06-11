/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  inline functions of dps parameter. 
*
*/


#include "dpsparam.h"

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
EXPORT_C void TDpsPrintInfo::Reset()
    {
    isDPOF = EFalse; iFileID = 0; iFileName = EFalse; iDate.Zero(); 
    iCopies = 0; iPrtPID = 0; iCopyID = 0;
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsStartJobReq::GetParamNum()
    {
    TInt num = iJobConfig.Count(), count = iPrintInfo.Count();
	for (TInt i = 0; i < count; i++)
	    {
	    // there is always fileID field
	    num++;
	    if (iPrintInfo[i].iFileName)
	        {
	        num++;
	        }
	    if (iPrintInfo[i].iDate.Size())
	        {
	        num++;
	        }
	    if (iPrintInfo[i].iCopies !=0)
	        {
	        num++;
	        }
	    if (iPrintInfo[i].iPrtPID != 0)
	        {
	        num++;
	        }             
	        
	    if (iPrintInfo[i].iCopyID != 0)
	        {
	        num++;
	        }    
	    }
	return num;   
	}
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    	
EXPORT_C void TDpsJobStatusRep::Reset()
    {
    iProgress = 0; iImagesPrinted = 0; iFilePath.Zero(); 
    iPrtPID = 0; iCopyID = 0;
    }    
