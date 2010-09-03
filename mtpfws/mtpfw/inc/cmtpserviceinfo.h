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
@internalComponent
*/

#ifndef CMTPSERVICEINFO_H_
#define CMTPSERVICEINFO_H_


#include <e32base.h>
#include <e32des8.h> 
#include <e32cmn.h>


#include <mtp/tmtptypeuint128.h>
#include <mtp/tmtptypeguid.h>

/** 
Implements the MTP service configure interface.
@internalComponent
 
*/


class TPKeyPair
    {
public:
    TPKeyPair( const TMTPTypeGuid& aNamespace, const TUint aID );
public:
    TMTPTypeGuid iNamespace;
    TUint     iPKeyId;
    };

class CServiceElementBase : public CBase
    {
public:
    virtual ~CServiceElementBase();
    IMPORT_C virtual TBool IsUsed();
    IMPORT_C virtual TUint Code() const;
    IMPORT_C virtual const TDesC& Name() const;
    IMPORT_C virtual const TMTPTypeGuid& GUID() const;
    
    virtual void SetCode( const TUint aCode );
    virtual TInt Compare( const CServiceElementBase& aR ) const;
    
protected:
    CServiceElementBase(const TMTPTypeGuid& aGUID, HBufC* aName);
    
protected:
    TUint16          iCode;
    TMTPTypeGuid  iGUID;
    HBufC*           iName;
    };

class CServiceProperty : public CServiceElementBase
    {
public:
    static CServiceProperty* NewLC(const TMTPTypeGuid& aGUID, const TUint aPkeyID, HBufC* aName, const TUint aDataType, const TUint aGetSet, const TUint aFormFlag);

    IMPORT_C const TMTPTypeGuid& Namespace() const;
    IMPORT_C TUint PKeyID() const;
    IMPORT_C TUint DataType() const;
    IMPORT_C TUint GetSet() const;
    IMPORT_C TUint FormFlag() const;
    
    
private:
    CServiceProperty( const TMTPTypeGuid& aGUID, const TUint aPkeyID, HBufC* aName, const TUint aDataType, const TUint aGetSet, const TUint aFormFlag );
    void ConstructL();
    
private:
    TUint            iPKeyId;
    TUint16          iDataType;
    TUint8           iGetSet;
    TUint8           iFormFlag;
    };


class CServiceFormat : public CServiceElementBase
    {
public:
    static CServiceFormat* NewLC(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aBaseFormat, HBufC* aMime );
    IMPORT_C TUint FormatBase() const;
    IMPORT_C const TDesC& MIMEType1() const;
    ~CServiceFormat();
    
private:
    CServiceFormat( const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aBaseFormat, HBufC* aMime );
    void ConstructL();
    
private:
    TUint16     iFormatBase;
    HBufC*      iMIMEType1;
    };


class CServiceMethod: public CServiceElementBase
    {
public:
    static CServiceMethod* NewLC(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aAssFormat);
    IMPORT_C TUint ObjAssociateFormatCode() const;
    
private:
    CServiceMethod(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aAssFormat);
    void ConstructL();
    
private:
    TUint16     iObjAssociateFormatCode;
    };

class CServiceEvent : public CServiceElementBase
    {
public:
    static CServiceEvent* NewLC(const TUint aCode, const TMTPTypeGuid& aGUID, HBufC* aName);

private:
    CServiceEvent(const TUint aCode, const TMTPTypeGuid& aGUID, HBufC* aName);
    void ConstructL();

    };


class CMTPServiceInfo :
    public CBase
    {
public:
    static CMTPServiceInfo* NewLC();
    ~CMTPServiceInfo();
    
private:
    CMTPServiceInfo();
    void ConstructL();
    
public:
    static TInt ServicePropertyOrderFromAscending( const CServiceProperty& aL, const CServiceProperty& aR);
    static TInt ServicePropertyOrderFromKeyAscending( const TPKeyPair* aL, const CServiceProperty& aR);
    
    static TInt ServiceElementBaseOrderFromAscending( const CServiceElementBase& aL, const CServiceElementBase& aR);
    static TInt ServiceElementBaseOrderFromKeyAscending( const TMTPTypeGuid* aL, const CServiceElementBase& aR );
    
    static TInt ServiceFormatOrderFromAscending( const CServiceFormat& aL, const CServiceFormat& aR);
    static TInt ServiceFormatOrderFromKeyAscending( const TMTPTypeGuid* aL, const CServiceFormat& aR );

    static TInt ServiceMethodOrderFromAscending( const CServiceMethod& aL, const CServiceMethod& aR );
    static TInt ServiceMethodOrderFromKeyAscending( const TMTPTypeGuid* aL, const CServiceMethod& aR );

    static TInt ServiceEventOrderFromAscending( const CServiceEvent& aL, const CServiceEvent& aR );

    
public:
    void SetServiceID( TUint aServiceID );
    void SetServiceStorageID( TUint aStorageID );
    void SetPersistentGUIDL( const TMTPTypeGuid& aGUID );
    void SetServiceVersion( TUint aVersion );
    void SetServiceGUIDL( const TMTPTypeGuid& aGUID );
    void SetServiceName( HBufC* aName );
    void SetServiceType( TUint aServiceType );
    void SetBaseServiceID( TUint aID );
    void AppendUserServiceL( const TMTPTypeGuid& aServiceGUID );
    void InsertPropertyL( const CServiceProperty* aProperty );
    void InsertFormatL( const CServiceFormat* aFormat );
    void InsertMethodL( const CServiceMethod* aMethod );
    void InsertEventL( const CServiceEvent* aEvent );
    void AppendDataGUIDL( const TMTPTypeGuid& aGUID );
    
    IMPORT_C TUint ServiceID() const;
    IMPORT_C TUint ServiceStorageID() const;
    IMPORT_C const TMTPTypeGuid& ServicePersistentGUID()const;
    IMPORT_C TUint ServiceVersion() const;
    IMPORT_C const TMTPTypeGuid& ServiceGUID() const;
    IMPORT_C const TDesC& ServiceName() const;
    IMPORT_C TUint ServiceType() const;
    IMPORT_C TUint BaseServiceID() const;
    
    CServiceProperty* ServiceProperty( const TMTPTypeGuid& aPKNamespace, const TUint aPKID ) const;
    CServiceFormat* ServiceFormat( const TMTPTypeGuid& aGUID ) const;
    CServiceMethod* ServiceMethod( const TMTPTypeGuid& aGUID ) const;
    
    IMPORT_C const RArray<TMTPTypeGuid>& UsedServiceGUIDs() const;
    IMPORT_C const RPointerArray<CServiceProperty>& ServiceProperties() const;
    IMPORT_C const RPointerArray<CServiceFormat>& ServiceFormats() const;
    IMPORT_C const RPointerArray<CServiceMethod>& ServiceMethods() const;
    IMPORT_C const RPointerArray<CServiceEvent>& ServiceEvents() const;
    IMPORT_C const RArray<TMTPTypeGuid>& DataBlockGUIDs() const;

    
private:
        TUint     iServiceId;
        TUint     iServiceStorageId;
        TMTPTypeGuid iPersistentGUID;
        TUint     iServiceVersion;
        TMTPTypeGuid iServiceGUID;
        HBufC*      iServiceName;
        TUint     iServiceType;
        TUint     iBaseServiceId;

        RArray<TMTPTypeGuid>     iUsedServiceGUIDs;
        RPointerArray<CServiceProperty>   iServiceProperties;
        RPointerArray<CServiceFormat>      iServiceFormats;
        RPointerArray<CServiceMethod>     iServiceMethods;
        RPointerArray<CServiceEvent>		iServiceEvents;
        RArray<TMTPTypeGuid>     iDataBlock;

    };




#endif /* CMTPSERVICEINFO_H_ */
