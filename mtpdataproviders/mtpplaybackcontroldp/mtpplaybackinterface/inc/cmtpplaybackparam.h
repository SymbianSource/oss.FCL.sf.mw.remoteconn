// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPPLAYBACKPARAM_H
#define CMTPPLAYBACKPARAM_H

#include "mtpdebug.h"

/**
 Category identifier
*/
enum TMTPPbCategory
    {
    /** Init value **/
    EMTPPbCatNone=0,
    /** SingleMusic **/
    EMTPPbCatMusic=1,
    /** PlayList**/
    EMTPPbCatPlayList = 2,
    /** Ablum **/
    EMTPPbCatAlbum = 3       
    };

enum TMTPPbDataType
    {
    EMTPPbTypeNone,
    EMTPPbInt32,
    EMTPPbUint32,
    EMTPPbSuidSet,
    EMTPPbVolumeSet,
    EMTPPbTypeEnd
    };

class TMTPPbDataSuid
    {
public:
    TMTPPbDataSuid(TMTPPbCategory aCategory, const TDesC& aSuid);

    TMTPPbCategory Category() const;
    const TDesC& Suid() const;

private:
    //The category
    TMTPPbCategory iPlayCategory;
    //The suid of object for initialization    
    TFileName iSuid;
    };

/**
* Encapsulates parameter for playback commands and events.
*/
class CMTPPbParamBase : public CBase
    {
public:
    virtual const TMTPPbDataSuid& SuidSetL() const;
    virtual TInt32 Int32L() const;
    virtual TUint32 Uint32L() const;
    virtual ~CMTPPbParamBase();
    TMTPPbDataType Type() const;

protected:
    /**
     * The constuctor.
     * @param aCategory, category of object for initialization,
     */
    CMTPPbParamBase();
    CMTPPbParamBase(TInt32 aValue);
    CMTPPbParamBase(TUint32 aValue);
    CMTPPbParamBase(TMTPPbCategory aCategory, const TDesC& aSuid);

    void ConstructL(TMTPPbCategory aCategory, const TDesC& aSuid);
    void ConstructL(TInt32 aValue);
    void ConstructL(TUint32 aValue);
    void ConstructL(const CMTPPbParamBase& aParam);
    void SetType(TMTPPbDataType);
    TAny* GetData() const;
    void SetData(TAny* aData);

private:
    __FLOG_DECLARATION_MEMBER;

    TMTPPbDataType  iParamType;
    TAny*       iData;
    };

#endif //CMTPPLAYBACKPARAM_H

