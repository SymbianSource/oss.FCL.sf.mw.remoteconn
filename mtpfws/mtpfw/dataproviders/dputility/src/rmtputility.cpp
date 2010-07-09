
// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <bautils.h>
#include <f32file.h>
#include <e32math.h>
#include <e32def.h>
#include <caf/content.h>

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtpdataproviderplugin.h>

#include "rmtputility.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpextensionmapping.h"
#include "cmtpdataprovider.h"
#include "mtpframeworkconst.h"

using namespace ContentAccess;
// Class constants.
const TInt KMTPDateStringLength = 15;
const TInt KMTPDateStringTIndex = 8;
const TInt KMimeTypeMaxLength = 76;
//const TInt KMAXPackageIDStringLen = 32;

_LIT( KTxtBackSlash, "\\" );
    
_LIT( KTxtExtensionODF, ".odf" );
    
_LIT( KMimeTypeAudio3gpp, "audio/3gpp" );
_LIT( KMimeTypeVideo3gpp, "video/3gpp" );
_LIT( KMimeTypeAudioMp4, "audio/mp4" );
_LIT( KMimeTypeVideoMp4, "video/mp4" );

__FLOG_STMT(_LIT8(KComponent,"RMTPUtility");)

RMTPUtility::RMTPUtility():
	iFramework(NULL)
	{
	}

void RMTPUtility::OpenL(MMTPDataProviderFramework& aFramework)
	{
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenL - Entry"));
    
	iFramework = &aFramework;
	iSingleton.OpenL();
	
    __FLOG(_L8("OpenL - Exit"));
	}

void RMTPUtility::Close()
	{
	iSingleton.Close();
	iFramework = NULL;
	iFormatMappings.ResetAndDestroy();
	__FLOG_CLOSE;
	}

/*
 * Convert the TTime to the MTP datatime string
 * 
 * 	MTP datatime string format: YYYYMMDDThhmmss.s  Optional(.s)
 *  TTime string format       : YYYYMMDD:HHMMSS.MMMMMM
 *  
 */
EXPORT_C TBool  RMTPUtility::TTime2MTPTimeStr(const TTime& aTime, TDes& aRet ) const
	{
	_LIT(KMTPDateStringFormat,"%F%Y%M%DT%H%T%S");		
	TRAPD(err, aTime.FormatL(aRet,KMTPDateStringFormat));
	if(err == KErrNone)
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

/*
 * Convert the MTP datatime string to TTime:
 * 
 * 	MTP datatime string format: YYYYMMDDThhmmss.s  Optional(.s)
 *  TTime string format       : YYYYMMDD:HHMMSS.MMMMMM
 *  
 */
EXPORT_C TBool RMTPUtility::MTPTimeStr2TTime(const TDesC& aTimeString, TTime& aRet) const
	{
    __FLOG(_L8("ConvertMTPTimeStr2TTimeL - Entry"));

	TBool result = EFalse;
	TInt year = 0;
	TMonth month = EJanuary;
	TInt day = 0;
	TInt hour = 0;
	TInt minute  = 0;
	TInt second = 0;
	TInt tenthSecond = 0;
	TBool positiveTimeZone = ETrue;
	TInt timeZoneInHour = 0;
	TInt timeZoneInMinute = 0;

    const TChar KCharT = 'T';
	if( aTimeString.Length() >= KMTPDateStringLength && ( aTimeString.Locate(KCharT) == KMTPDateStringTIndex ) && GetYear(aTimeString, year) && GetMonth(aTimeString, month) && 
		GetDay(aTimeString, day) && GetHour(aTimeString, hour) && GetMinute(aTimeString, minute) && 
		GetSecond(aTimeString, second) && GetTenthSecond(aTimeString, tenthSecond) && GetTimeZone(aTimeString, positiveTimeZone, timeZoneInHour, timeZoneInMinute))
		{
		TDateTime dateTime(year, month, day, hour, minute, second, tenthSecond * 100000);
		
		TTime dateTimeInTTime(dateTime);
		if(positiveTimeZone)
			{
			dateTimeInTTime += TTimeIntervalHours(timeZoneInHour);
			dateTimeInTTime += TTimeIntervalMinutes(timeZoneInMinute);
			}
		else
			{
			dateTimeInTTime -= TTimeIntervalHours(timeZoneInHour);
			dateTimeInTTime -= TTimeIntervalMinutes(timeZoneInMinute);
			}		
		
		aRet = dateTimeInTTime.Int64();
		result = ETrue;
		}
	
	return result;
	}

TBool RMTPUtility::GetYear(const TDesC& aTimeString, TInt& aYear) const
	{
	aYear = 0;
	TLex dateBuf(aTimeString.Left(4));
	return dateBuf.Val(aYear) == KErrNone;
	}

TBool RMTPUtility::GetMonth(const TDesC& aTimeString, TMonth& aMonth) const
	{
	TBool result = EFalse;
	aMonth = EJanuary;
	TInt month = 0;
	TLex dateBuf(aTimeString.Mid(4, 2));
	if(dateBuf.Val(month) == KErrNone && month > 0 && month < 13)
		{
		month--;
		aMonth = (TMonth)month;
		result = ETrue;
		}
	return result;
	}

TBool RMTPUtility::GetDay(const TDesC& aTimeString, TInt& aDay) const
	{
	TBool result = EFalse;
	aDay = 0;
	TLex dateBuf(aTimeString.Mid(6, 2));
	if(dateBuf.Val(aDay) == KErrNone && aDay > 0 && aDay < 32)
		{
		aDay--;
		result = ETrue;
		}
	return result;	
	}

TBool RMTPUtility::GetHour(const TDesC& aTimeString, TInt& aHour) const
	{
	aHour = 0;
	TLex dateBuf(aTimeString.Mid(9, 2));
	return (dateBuf.Val(aHour) == KErrNone && aHour >=0 && aHour < 60);
	}
				
TBool RMTPUtility::GetMinute(const TDesC& aTimeString, TInt& aMinute) const
	{
	aMinute = 0;
	TLex dateBuf(aTimeString.Mid(11, 2));
	return (dateBuf.Val(aMinute) == KErrNone && aMinute >=0 && aMinute < 60);
	}

TBool RMTPUtility::GetSecond(const TDesC& aTimeString, TInt& aSecond) const
	{
	aSecond = 0;
	TLex dateBuf(aTimeString.Mid(13, 2));
	return (dateBuf.Val(aSecond) == KErrNone && aSecond >= 0 && aSecond < 60);
	}

TBool RMTPUtility::GetTenthSecond(const TDesC& aTimeString, TInt& aTenthSecond) const
	{
	TBool result = EFalse;
	aTenthSecond = 0;
	TInt dotPos = aTimeString.Find(_L("."));
	if(dotPos != KErrNotFound && dotPos == KMTPDateStringLength && aTimeString.Length() > KMTPDateStringLength + 1)
		{
		TLex dateBuf(aTimeString.Mid(dotPos + 1, 1));
		result = (dateBuf.Val(aTenthSecond) == KErrNone && aTenthSecond >=0 && aTenthSecond < 10);
		}
	else if(dotPos == KErrNotFound)
		{
		result = ETrue;
		}
	
	return result;	
	}

TBool RMTPUtility::GetTimeZone(const TDesC& aTimeString, TBool& aPositiveTimeZone, TInt& aTimeZoneInHour, TInt& aTimeZoneInMinute) const	 		
	{
	TBool result = EFalse;
	aTimeZoneInHour = 0;
	aTimeZoneInMinute = 0;
	TInt plusTimeZonePos = aTimeString.Find(_L("+"));
	TInt minusTimeZonePos = aTimeString.Find(_L("-"));
	TInt timeZoneIndicatorPos = Max(plusTimeZonePos, minusTimeZonePos);		
	aPositiveTimeZone = (plusTimeZonePos != KErrNotFound);
	if(timeZoneIndicatorPos != KErrNotFound)
		{
		if(aTimeString.Length() - timeZoneIndicatorPos == 5)
			{
			TLex dateBuf(aTimeString.Mid(timeZoneIndicatorPos + 1, 2));
			if(dateBuf.Val(aTimeZoneInHour) == KErrNone)
				{
				dateBuf.Assign(aTimeString.Mid(timeZoneIndicatorPos + 3, 2));
				if(dateBuf.Val(aTimeZoneInMinute) == KErrNone)
					{
					result = ETrue;
					}
				}
			}
		}
	else
		{
		result = ETrue;
		}
	return result;
	}

EXPORT_C void RMTPUtility::RenameObjectL( TUint aObjectHandle, const TDesC& aNewName )
	{
    __FLOG(_L8("RenameAssocationObjectL - Entry"));
    
    CMTPObjectMetaData* meta = CMTPObjectMetaData::NewLC();
       
    if( !iFramework->ObjectMgr().ObjectL(aObjectHandle, *meta) )
    	{
    	User::Leave(KErrNotFound);
    	}
			
   if( !BaflUtils::FileExists( iFramework->Fs(), meta->DesC(CMTPObjectMetaData::ESuid) ) )
	   {
	   User::Leave(KErrNotFound);
	   }
	
	TFileName filename;
	User::LeaveIfError(BaflUtils::MostSignificantPartOfFullName(meta->DesC(CMTPObjectMetaData::ESuid), filename));
	RBuf oldFullName;
	oldFullName.CleanupClosePushL();
	
	TInt maxLen = (KMaxFileName > meta->DesC(CMTPObjectMetaData::ESuid).Length()? KMaxFileName: meta->DesC(CMTPObjectMetaData::ESuid).Length());
	oldFullName.CreateL(maxLen);
	oldFullName = meta->DesC(CMTPObjectMetaData::ESuid);
		
	// Update the folder name using the new passed value.
	RBuf newFullName;
	newFullName.CleanupClosePushL();
	newFullName.CreateL(maxLen);
	newFullName.Append(oldFullName);
	if(meta->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation)
		{
		newFullName.SetLength(newFullName.Length() - filename.Length() - 1);
		}
	else
		{
		newFullName.SetLength(newFullName.Length() - filename.Length());
		}
	
	maxLen = newFullName.Length() + aNewName.Length() + 1;
	if(maxLen > newFullName.MaxLength())
		{
		newFullName.ReAllocL(maxLen);
		}
	newFullName.Append(aNewName);
	
	if(meta->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeAssociation)
		{
		// Modify the filename
		User::LeaveIfError( iFramework->Fs().Rename(meta->DesC(CMTPObjectMetaData::ESuid), newFullName) );
		
		meta->SetDesCL( CMTPObjectMetaData::ESuid, newFullName );
		iFramework->ObjectMgr().ModifyObjectL(*meta);
		}
	else
		{
		// Add backslash.
		_LIT(KBackSlash, "\\");
		newFullName.Append(KBackSlash);
		// Modify the filename
		User::LeaveIfError( iFramework->Fs().Rename(meta->DesC(CMTPObjectMetaData::ESuid), newFullName) );
		
		meta->SetDesCL( CMTPObjectMetaData::ESuid, newFullName );
		iFramework->ObjectMgr().ModifyObjectL(*meta);
		
		RenameAllChildrenL( meta->Uint(CMTPObjectMetaData::EStorageId), meta->Uint(CMTPObjectMetaData::EHandle), newFullName, oldFullName);
		
		if(meta->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation)
			{
			TMTPNotificationParamsHandle param = { meta->Uint(CMTPObjectMetaData::EHandle) ,oldFullName};
			iSingleton.DpController().NotifyDataProvidersL(EMTPRenameObject, static_cast<TAny*>(&param));
			}
		}

	CleanupStack::PopAndDestroy(3);//oldFullName, newFullName,meta
    __FLOG(_L8("RenameAssocationObjectL - Exit"));
	}

EXPORT_C TMTPFormatCode RMTPUtility::FormatFromFilename( const TDesC& aFullFileName )
    {
    if ( aFullFileName.Right( 1 ).CompareF( KTxtBackSlash ) == 0 ) 
        {
        return EMTPFormatCodeAssociation;
        }

    TParsePtrC file( aFullFileName );

    if ( file.Ext().CompareF( KTxtExtensionODF ) == 0 )
        {
        HBufC* mime =ContainerMimeType( file.FullName() );

        // 3GP
        if ( mime->CompareF( KMimeTypeAudio3gpp ) == 0 || mime->CompareF( KMimeTypeVideo3gpp ) == 0)
            {
            delete mime;
            mime = NULL;
            return EMTPFormatCode3GPContainer;
            }
        else if (  mime->CompareF( KMimeTypeAudioMp4 ) == 0 || mime->CompareF( KMimeTypeVideoMp4 ) == 0 )
            {
            delete mime;
            mime = NULL;
            return EMTPFormatCodeMP4Container;
            }
        if ( mime != NULL )
            {
            delete mime;
            mime = NULL;
            }
        }

    return EMTPFormatCodeUndefined;
    }

EXPORT_C HBufC* RMTPUtility::ContainerMimeType( const TDesC& aFullPath )
    {

    TParsePtrC file( aFullPath );
    HBufC* mime = NULL;
    TInt err = KErrNone;
    
    if ( file.Ext().CompareF( KTxtExtensionODF ) == 0 )
        {
        TRAP( err, mime = OdfMimeTypeL( aFullPath ) );
        __FLOG_VA((_L("ContainerMimeType err %d mime %S"), err, mime));
        }
    return mime;
    }

EXPORT_C void RMTPUtility::FormatExtensionMapping()
    {
     TInt count = iSingleton.DpController().Count();
    
    while(count--)
        {
        CDesCArraySeg* FormatExtensionMapping = new (ELeave) CDesCArraySeg(4);
        CleanupStack::PushL(FormatExtensionMapping);
        TRAP_IGNORE(
        iSingleton.DpController().DataProviderByIndexL(count).Plugin().SupportedL(EFormatExtensionSets,*FormatExtensionMapping);
        AppendFormatExtensionMappingL(*FormatExtensionMapping,iSingleton.DpController().DataProviderByIndexL(count).DataProviderId());
        );
        CleanupStack::PopAndDestroy(FormatExtensionMapping);
        }
    }

EXPORT_C TMTPFormatCode RMTPUtility::GetFormatByExtension(const TDesC& aExtension)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::Compare));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->FormatCode();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return EMTPFormatCodeUndefined;
    }

EXPORT_C TUint32 RMTPUtility::GetDpIdL(const TDesC& aExtension,const TDesC& aMIMEType)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined,aMIMEType);
    CleanupStack::PushL(extensionMapping);
    TInt  found = KErrNotFound;
    if(aMIMEType == KNullDesC)
        {
        found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::Compare));
        }
    else
        {
        found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME));
        }
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->DpId();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return KMTPFileDPID;
    }

EXPORT_C TUint16 RMTPUtility::GetSubFormatCodeL(const TDesC& aExtension,const TDesC& aMIMEType)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined,aMIMEType);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->SubFormatCode();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return 0;
    }

EXPORT_C TMTPFormatCode RMTPUtility::GetFormatCodeByMimeTypeL(const TDesC& aExtension,const TDesC& aMIMEType)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined,aMIMEType);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->FormatCode();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return EMTPFormatCodeUndefined;
    }


EXPORT_C TUint RMTPUtility::GetEnumerationFlag(const TDesC& aExtension)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::Compare));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->EnumerationFlag();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return 1;
    }

void RMTPUtility::RenameAllChildrenL(TUint32 aStorageId, TUint32 aParentHandle, const TDesC& aNewFolderName, const TDesC& aOldFolderName)
	{
    __FLOG(_L8("RenameAllChildrenL - Entry"));
    
    CMTPObjectMetaData* objectInfo(CMTPObjectMetaData::NewLC());
    TInt count = 0; 
    RArray<TUint>               handles;
    CleanupClosePushL(handles);
    GetAllDecendents(aStorageId, aParentHandle, handles);
    count = handles.Count();
    
    TEntry entry;
    for(TInt i(0); (i < count); ++i)
        {
        if (!iFramework->ObjectMgr().ObjectL(handles[i], *objectInfo))
            {
            User::Leave(KErrCorrupt);
            }
        
        /**
         * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
         * but it does not really own the format 0x3002.
         * 
         * Make the same behavior betwen 0x3000 and 0x3002.
         */
        if( (objectInfo->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeAssociation)
            && (objectInfo->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeUndefined)
            && (objectInfo->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeScript) )
           continue;

        RBuf entryName; 
        entryName.CreateL(KMaxFileName);
        entryName.CleanupClosePushL();
        entryName = objectInfo->DesC(CMTPObjectMetaData::ESuid);
        
        RBuf rightPartName;
        rightPartName.CreateL(KMaxFileName);
        rightPartName.CleanupClosePushL();
        rightPartName = entryName.Right(entryName.Length() - aOldFolderName.Length());
        
        if ((aNewFolderName.Length() + rightPartName.Length()) > entryName.MaxLength())
            {
            entryName.ReAllocL(aNewFolderName.Length() + rightPartName.Length());
            }
        
        entryName.Zero();
        entryName.Append(aNewFolderName);
        entryName.Append(rightPartName);
        
        if (KErrNone != iFramework->Fs().Entry(entryName, entry))
            {
            // Skip objects that don't use the file path as SUID.
            CleanupStack::PopAndDestroy(&rightPartName);            
            CleanupStack::PopAndDestroy(&entryName);
            continue;
            }        
        
        TFileName oldfilename(objectInfo->DesC(CMTPObjectMetaData::ESuid));
        objectInfo->SetDesCL(CMTPObjectMetaData::ESuid, entryName);
        iFramework->ObjectMgr().ModifyObjectL(*objectInfo);
        
        if(objectInfo->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation)
            {
            //Send the Rename notification 
            TMTPNotificationParamsHandle param = { handles[i], oldfilename};
            iSingleton.DpController().NotifyDataProvidersL(EMTPRenameObject, static_cast<TAny*>(&param));
            }
            
        CleanupStack::PopAndDestroy(2); // rightPartName, entryName             
        }
    
    CleanupStack::PopAndDestroy(2); //objectInfo; &handles; 
	
    __FLOG(_L8("RenameAllChildrenL - Exit"));
	}

void RMTPUtility::GetAllDecendents(TUint32 aStorageId, TUint aParentHandle, RArray<TUint>& aHandles) const
    {
    TInt index = 0; 
    TBool firstLevel = ETrue;
    
    do
        {
        TUint parentHandle;
        if (firstLevel)
            {
            parentHandle = aParentHandle; //Get the first level children handles
            firstLevel = EFalse;
            }        
        else
            {
            parentHandle = aHandles[index];
            ++index;
            }        
        
        RMTPObjectMgrQueryContext   context;
        RArray<TUint>               childrenHandles;
        TMTPObjectMgrQueryParams    params(aStorageId, KMTPFormatsAll, parentHandle);
        CleanupClosePushL(context);
        CleanupClosePushL(childrenHandles);
        
        do
            {
            iFramework->ObjectMgr().GetObjectHandlesL(params, context, childrenHandles);
            TInt count = childrenHandles.Count(); 
            for (TUint i = 0; i < count; ++i)
                {
                aHandles.Append(childrenHandles[i]);
                }
            }
        while (!context.QueryComplete());
        CleanupStack::PopAndDestroy(2); //&childrenHandles; &context
        }
    while(index < aHandles.Count());

    }

HBufC* RMTPUtility::OdfMimeTypeL( const TDesC& aFullPath )
    {
    HBufC* mimebuf = NULL;
    
    TParsePtrC file( aFullPath );
        
    if ( file.Ext().CompareF( KTxtExtensionODF ) == 0 )
        {
        CContent* content = CContent::NewL( aFullPath );
        CleanupStack::PushL( content ); // + content
        
        HBufC* buffer = HBufC::NewL( KMimeTypeMaxLength );
        CleanupStack::PushL( buffer ); // + buffer
        
        TPtr data = buffer->Des();
        TInt err = content->GetStringAttribute( ContentAccess::EMimeType, data );
        if ( err == KErrNone )
            {
            mimebuf = HBufC::New( buffer->Length() );
    
            if (mimebuf == NULL)
                {
                User::LeaveIfError( KErrNotFound );
                }
            mimebuf->Des().Copy( *buffer );
            
            }
        
        // leave if NULL
        if ( mimebuf == NULL )
            {
            User::Leave( KErrNotFound );
            }
        
        CleanupStack::PopAndDestroy( buffer ); // - buffer
        CleanupStack::PopAndDestroy( content ); // - content
        }
    else
        {
        User::Leave( KErrNotSupported );
        }
    
    return mimebuf;
    }
void RMTPUtility::ParseFormatCode(const TDesC& aString, CMTPExtensionMapping& aMapping, TParseState& aState)
    {
    
    TLex lex(aString.Mid(2)); //skip 0x
    TUint formatCode = EMTPFormatCodeUndefined;
    lex.Val(formatCode, EHex);
    aMapping.SetFormatCode(static_cast<TMTPFormatCode>(formatCode));
    aState = Extension;
    __FLOG_VA((_L("ParseFormatCode %S, 0x%x"), &aString, formatCode));
    }
void RMTPUtility::ParseExtension(const TDesC& aString, CMTPExtensionMapping& aMapping, TParseState& aState)
    {
    aMapping.SetExtensionL(aString);
    aState = EMimeType;
    __FLOG_VA((_L("ParseExtension %S"), &aString));
    }
void RMTPUtility::ParseMimeType(const TDesC& aString, CMTPExtensionMapping& aMapping, TParseState& aState)
    {
    aMapping.SetMIMETypeL(aString);
    aState = ESubFormatCode;
    __FLOG_VA((_L("ParseMimeType %S"), &aString));
    }
void RMTPUtility::ParseSubFormatCode(const TDesC& aString, CMTPExtensionMapping& aMapping, TParseState& aState)
    {
    iEnumFlag = 1;
    TUint32 subFormatCode = 0;
    if(aString != KNullDesC)
        {
        if(aString.Length() > 2) //temp code will remove after the correct their format string
            {
            TLex lex(aString.Mid(2)); //skip 0x
            lex.Val(subFormatCode, EHex);
            }
        else//temp code will remove after the correct their format string
            {
            TLex lex(aString);
            lex.Val(iEnumFlag, EDecimal);
            }
        }
    aMapping.SetSubFormatCode(subFormatCode);
    aState = EnumerationFlag;
    __FLOG_VA((_L("ParseSubFormatCode %S, 0x%x"), &aString, subFormatCode));
    }
void RMTPUtility::ParseEnumerationFlag(const TDesC& aString, CMTPExtensionMapping& aMapping, TParseState& aState)
    {
    TUint enumFlag = iEnumFlag;           //default is delegate to file DP
    //temp code(iEnumFlag is temp the value should be 1) will remove after the correct their format string
    if(aString != KNullDesC)
        {
        TLex lex(aString);
        lex.Val(enumFlag, EDecimal);
        }
    aMapping.SetEnumerationFlag(enumFlag);
    
    __FLOG_VA((_L8("ParseEnumerationFlag %S, %d"), &aString, enumFlag));
    aState = EParseStateEnd;
    }
void RMTPUtility::Parse(const TDesC& aString, CMTPExtensionMapping& aMapping, TParseState& aState)
    {
    switch(aState)
        {
        case EFormatCode:
            ParseFormatCode(aString, aMapping, aState);
            break;
        case Extension:
            ParseExtension(aString, aMapping, aState);
            break;
        case EMimeType:
            ParseMimeType(aString, aMapping, aState);
            break;
        case ESubFormatCode:
            ParseSubFormatCode(aString, aMapping, aState);
            break;
        case EnumerationFlag:
            ParseEnumerationFlag(aString, aMapping, aState);
            break;
        default:
            //do nothing;
            break;
        }
    }

void  RMTPUtility::AppendFormatExtensionMappingL(const CDesCArray& aFormatExtensionMapping,TUint32 aDpId)
    {
    //Parse the descriptor formatcode,fileextension, e.g. 0x3009:mp3
    //FromatCode:extension:MimeType:SubFormatCode:EnumerationFlag

    TChar spliter(':');
    for(TInt i=0; i < aFormatExtensionMapping.Count(); ++i)
        {
        CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(KNullDesC, EMTPFormatCodeUndefined);
      
        TParseState state = EFormatCode;
        TLex lex(aFormatExtensionMapping[i]);
        lex.Mark();
        while(!lex.Eos())
            {
            if(lex.Peek() == spliter)
                {
                Parse(lex.MarkedToken(), *extensionMapping, state);
                lex.Inc(); //skip :
                lex.Mark();
                }
            else
                {
                lex.Inc(); //move to next char
                }
            }
        while(state < EParseStateEnd)
            {
            Parse(lex.MarkedToken(), *extensionMapping, state);
            lex.Mark();
            }
        extensionMapping->SetDpId(aDpId);
        if(iFormatMappings.InsertInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME)) != KErrNone)
            {
            delete extensionMapping;
            }
        }
    }



