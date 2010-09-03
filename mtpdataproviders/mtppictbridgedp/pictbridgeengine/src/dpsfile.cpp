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
* Description:  This class reads and writes the file content. 
*
*/


#include "dpsfile.h"
#include "dpsdefs.h"
#include "dpsconst.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsfileTraces.h"
#endif

    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CDpsFile* CDpsFile::NewL()
	{
	CDpsFile* self = new (ELeave) CDpsFile();
	CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(); 
	return self;
	}

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CDpsFile::ConstructL()
    {
    OstTraceFunctionEntry0( CDPSFILE_CONSTRUCTL_ENTRY );  
	LEAVEIFERROR(iFs.Connect(),
	        OstTrace1( TRACE_ERROR, CDPSFILE_CONSTRUCTL, 
	                "Connect to file server failed! error code %d", munged_err));
    OstTraceFunctionExit0( CDPSFILE_CONSTRUCTL_EXIT );
    }
    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CDpsFile::~CDpsFile()
	{
	OstTraceFunctionEntry0( CDPSFILE_CDPSFILE_DES_ENTRY );
	iFs.Close();
    OstTraceFunctionExit0( CDPSFILE_CDPSFILE_DES_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TInt CDpsFile::CreateScriptFile(const TDesC& aFileName, const TDesC8& aScript,
								const TInt aFileSize)
    {
    OstTraceFunctionEntry0( CDPSFILE_CREATESCRIPTFILE_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSFILE_CREATESCRIPTFILE, "File size %d", aFileSize );          
    RFile file;
    TInt err = file.Replace(iFs, aFileName, EFileShareExclusive);
    OstTrace1( TRACE_NORMAL, DUP1_CDPSFILE_CREATESCRIPTFILE, "---the error is %d", err );
    if (err != KErrNone)
        {
        OstTraceFunctionExit0( CDPSFILE_CREATESCRIPTFILE_EXIT );
        return err;
        }
        
    TInt fileSize = aScript.Size();
    if (!fileSize)
        {
        err = file.Write(_L8(""), 0);
        }
    else if ( fileSize <= aFileSize)
        {
        err = file.Write(aScript, fileSize);
        }
    else
        {
        err = file.Write(aScript, aFileSize);
        }
        
    file.Close();     
    OstTrace1( TRACE_NORMAL, DUP2_CDPSFILE_CREATESCRIPTFILE, "return value %d", err );        
    OstTraceFunctionExit0( DUP1_CDPSFILE_CREATESCRIPTFILE_EXIT );
    return err;   
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsFile::GetContentL(const TDesC& aFileName, TDes8& aScript)
    {
    OstTraceFunctionEntry0( CDPSFILE_GETCONTENTL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CDPSFILE_GETCONTENTL, "File name %S", aFileName );              
    RFile file;
    CleanupClosePushL(file);
    LEAVEIFERROR(file.Open(iFs, aFileName, EFileRead),
            OstTraceExt2( TRACE_ERROR, DUP1_CDPSFILE_GETCONTENTL, 
                    "Open %S failed! error code %d", aFileName, munged_err));
    LEAVEIFERROR(file.Read(aScript),
            OstTrace1( TRACE_ERROR, DUP2_CDPSFILE_GETCONTENTL, 
                    "Read from file failed! error code %d", munged_err ));
    CleanupStack::PopAndDestroy();                      
    OstTraceFunctionExit0( CDPSFILE_GETCONTENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsFile::FileSizeL(const TDesC& aFileName, TInt& aSize)
    {
    OstTraceFunctionEntry0( CDPSFILE_FILESIZEL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CDPSFILE_FILESIZEL, "File name %S", aFileName );       
    RFile file;
    CleanupClosePushL(file);
    LEAVEIFERROR(file.Open(iFs, aFileName, EFileRead),
            OstTraceExt2( TRACE_ERROR, DUP1_CDPSFILE_FILESIZEL, 
                    "Open %S failed! error code %d", aFileName, munged_err));
    LEAVEIFERROR(file.Size(aSize),
            OstTrace1( TRACE_ERROR, DUP2_CDPSFILE_FILESIZEL, 
                    "Gets file size failed! error code %d", munged_err));
    CleanupStack::PopAndDestroy();    
    OstTrace1( TRACE_NORMAL, DUP3_CDPSFILE_FILESIZEL, "File size %d", aSize );    
    OstTraceFunctionExit0( CDPSFILE_FILESIZEL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
TInt CDpsFile::Delete(const TDesC& aFileName)
    {
    OstTraceFunctionEntry0( CDPSFILE_DELETE_ENTRY );
    OstTraceFunctionExit0( CDPSFILE_DELETE_EXIT );                  
    return iFs.Delete(aFileName);
    }
