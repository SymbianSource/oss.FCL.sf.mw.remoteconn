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


#ifndef DPSFILE_H
#define DPSFILE_H

#include <e32base.h>
#include <f32file.h>

/**
*   This class creates, reads, writes and deletes dps files.
*/
NONSHARABLE_CLASS(CDpsFile) : public CBase
    {
public:
    /**
    * Two-phased constructor.
    * @return An instance of CDpsFile.
    */
    static CDpsFile* NewL();
	
    /**
    * C++ destructor.
    */
    ~CDpsFile();

public: 
    /**
    *   Creates the dps script file
    *   @param aFileName the script file name
    *   @param aScript the script file content
    *   @aFileSize the file size
    *   @return KErrNone if successful or systme wide error if failed
    */
    TInt CreateScriptFile(const TDesC& aFileName, const TDesC8& aScript, 
                          const TInt aFileSize);

    /**
    *   Gets the content of the script file
    *   @param aFileName the file name
    *   @param aScript the file content. It has the valid content after this 
    *   call is returned.
    *   @return KErrNone if successful or systme wide error if failed
    */						  
    void GetContentL(const TDesC& aFileName, TDes8& aScript);
	
    /**
    *   Deletes the file
    *   @param aFileName the name of the file to be delted.
    *   @return KErrNone if successful or systme wide error if failed
    */
    TInt Delete(const TDesC& aFileName);
    
    /**
    *   @return RFs& the reference to the file server session, which
    *   is shared by the whole component (dps engine binary)
    */
    inline RFs& FileSession();
    
    void FileSizeL(const TDesC& aFileName, TInt& aSize);
	
private:
    
    /**
    *   Second phase constructor
    */
    void ConstructL();
    
private:
    // file server session, owned by this class
    RFs iFs;
    };
    
#include "dpsfile.inl"
#endif
