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
 @publishedPartner
 @released
*/

#ifndef RMTPTYPE_H
#define RMTPTYPE_H

#include <e32std.h>
#include <mtp/mmtptype.h>

/**
Defines a general purpose class for containing, accessing, and manipulating 
a reference to any MMTPType derived data type. The data type buffer itself 
resides on the heap and is managed by this class.

By default all MTP simple (signed and unsigned integer), array (signed and 
unsigned integer), and string types are supported. Support for any other data 
types can be provided by derived subclasses which provide suitable 
implementions of the CreateL and Destroy methods.

This class and it's derived subclasses can be freely contained as member 
variables within any CBase derived class (typically a CMTPTypeCompoundBase 
derived class). When used as a stack variable the class's CleanupClosePushL
method should always be used to ensure that the heap storage managed by the 
class is correctly deallocated.

@publishedPartner
@released 
*/
class RMTPType : public MMTPType
    {
public:

    IMPORT_C RMTPType();    
    IMPORT_C virtual ~RMTPType();
    
    IMPORT_C void CleanupClosePushL();
    IMPORT_C void Close();
    IMPORT_C void OpenL(TUint aDataType);
    
    IMPORT_C MMTPType& Data();
    
    //internal API
    static MMTPType* AllocL(const TUint aDataType);
    static void Destroy(const TUint aDataType, MMTPType* aType);
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    IMPORT_C TInt Validate() const;
    
protected:
    
    IMPORT_C virtual MMTPType* CreateL(TUint aDataType);
    IMPORT_C virtual void Destroy(MMTPType* aType);
    
private:

//   MMTPType* AllocL(TUint aDataType);
    
private: // Owned
   
    /**
    The data type buffer.
    */
    MMTPType*   iData;
    };

#endif // RMTPTYPE_H
