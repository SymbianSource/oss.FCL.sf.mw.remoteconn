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

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CDpsFile* CDpsFile::NewL()
	{
	IF_DEBUG(Print(_L("CDpsFile::NewL")));
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
    IF_DEBUG(Print(_L(">>>CDpsFile::ConstructL")));    
	User::LeaveIfError(iFs.Connect());
    IF_DEBUG(Print(_L("<<<CDpsFile::ConstructL")));    
    }
    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
CDpsFile::~CDpsFile()
	{
	IF_DEBUG(Print(_L(">>>CDpsFile::~")));        
	iFs.Close();
	IF_DEBUG(Print(_L("<<<CDpsFile::~")));        
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TInt CDpsFile::CreateScriptFile(const TDesC& aFileName, const TDesC8& aScript,
								const TInt aFileSize)
    {
    IF_DEBUG(Print(_L(">>>CDpsFile::CreateScriptFile size %d"), aFileSize));            
    RFile file;
    TInt err = file.Replace(iFs, aFileName, EFileShareExclusive);
    IF_DEBUG(Print(_L("---the error is %d"), err));
    if (err != KErrNone)
        {
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
    IF_DEBUG(Print(_L("<<<CDpsFile::CreateScriptFile %d"), err));            
    return err;   
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsFile::GetContentL(const TDesC& aFileName, TDes8& aScript)
    {
    IF_DEBUG(Print(_L(">>>CDpsFile::GetContent %S"), &aFileName));                
    RFile file;
    CleanupClosePushL(file);
    User::LeaveIfError(file.Open(iFs, aFileName, EFileRead));
    User::LeaveIfError(file.Read(aScript));
    CleanupStack::PopAndDestroy();
    IF_DEBUG(Print(_L("<<<CDpsFile::GetContent")));                       
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsFile::FileSizeL(const TDesC& aFileName, TInt& aSize)
    {
    IF_DEBUG(Print(_L(">>>CDpsFile::FileSize %S"), &aFileName));       
    RFile file;
    CleanupClosePushL(file);
    User::LeaveIfError(file.Open(iFs, aFileName, EFileRead)); 
    User::LeaveIfError(file.Size(aSize));    
    CleanupStack::PopAndDestroy();
    IF_DEBUG(Print(_L("<<<CDpsFile::FileSize %d"), aSize));                
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
TInt CDpsFile::Delete(const TDesC& aFileName)
    {
    IF_DEBUG(Print(_L("CDpsFile::Delete")));                    
    return iFs.Delete(aFileName);
    }
