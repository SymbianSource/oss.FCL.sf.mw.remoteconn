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

/**
 @file
 @internalTechnology
*/

#include <bautils.h>
#include <mdesession.h>
#include <mdequery.h>
#include <mdeconstants.h>

#include "cmediasyncserverdef.h"
#include "cmediasyncdatabase.h"
#include "cmediasyncdatawriter.h"

__FLOG_STMT(_LIT8(KComponent,"MediaSyncDatabase");)

const TInt KCompactThreshold = 50;
const TInt KMaxRetryTimes = 3;
const TInt KDelayPeriod = 3 * 1000000;

CMediaSyncDatabase* CMediaSyncDatabase::NewL(RFs& aFs)
    {
    CMediaSyncDatabase* self = new (ELeave) CMediaSyncDatabase(aFs);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CMediaSyncDatabase::ConstructL()
    {
    __FLOG_OPEN(KMSSSubsystem, KComponent);
    __FLOG(_L8("CMediaSyncDatabase::ConstructL - Entry"));

    //Connect to the file server
    User::LeaveIfError(iFs.Connect());

    TFileName databasePath;
    iFs.PrivatePath(databasePath);
    TDriveUnit driveNum = RFs::GetSystemDrive();
    databasePath.Insert(0, driveNum.Name());
    databasePath.Append(KMssDbName);

    CreateTableL(databasePath);    
    
    User::LeaveIfError(iBatched.Open(iDatabase, KImageTableName, RDbRowSet::EUpdatable));
    
    __FLOG(_L8("CMediaSyncDatabase::ConstructL - Exit"));
    }

CMediaSyncDatabase::CMediaSyncDatabase(RFs& aFs) :
    iFs(aFs),
    iDbCorrupt(EFalse),
    iSavePosition(EFalse)
    {
    }

CMediaSyncDatabase::~CMediaSyncDatabase()
    {
    __FLOG(_L8("CMediaSyncDatabase::~CMediaSyncDatabase - Entry"));
    
    iBatched.Close();
    iDatabase.Close();
    
    __FLOG(_L8("CMediaSyncDatabase::~CMediaSyncDatabase - Exit"));
    __FLOG_CLOSE;
    }

void CMediaSyncDatabase::CreateTableL(const TDesC& aDbFile)
    {
    __FLOG(_L8("CMediaSyncDatabase::CreateTableL - Entry"));
    
    _LIT(KSQLCreateTable, "CREATE TABLE ImageStore(ObjectId UNSIGNED INTEGER, NotificationType UNSIGNED INTEGER, URI VARCHAR(255))");           
    
    TInt err = KErrNone;
    if (!BaflUtils::FileExists(iFs, aDbFile))
        {
        __FLOG(_L8("CreateTableL - Table ImageStore does not exist"));
        
        BaflUtils::EnsurePathExistsL(iFs, aDbFile);
        
        User::LeaveIfError(iDatabase.Create(iFs, aDbFile));
        User::LeaveIfError(iDatabase.Execute(KSQLCreateTable));
        TRAP_IGNORE(CreateTabIndexL());
        }    
    else
        {
        //Open the database
        TBool recreateDbFile = EFalse;
        err = iDatabase.Open(iFs, aDbFile);
        if (err == KErrNone)
            {
            if (iDatabase.IsDamaged())
                {
                recreateDbFile = (iDatabase.Recover() == KErrNone) ? EFalse : ETrue;
                }
            }
        else
            {
            recreateDbFile = ETrue;
            }
        
        if (recreateDbFile)
            {
            __FLOG_VA((_L8("CreateTableL - Open Table ImageStore failed: %d"), err));
            iDatabase.Close();

            TInt retryCount = KMaxRetryTimes;
            TInt result = KErrNone;            
            for (; retryCount > 0; retryCount--)
                {
                result = BaflUtils::DeleteFile(iFs, aDbFile);
                if (result == KErrNone)
                    {
                    // We have succesfully delete corrupt database file
                    break;
                    }       
                else
                    {
                    User::After(KDelayPeriod);
                    }
                }
            
            User::LeaveIfError(result);
            User::LeaveIfError(iDatabase.Create(iFs, aDbFile));
            User::LeaveIfError(iDatabase.Execute(KSQLCreateTable));
            TRAP_IGNORE(CreateTabIndexL());
            iDbCorrupt = ETrue;          
            }
        }    
    
    __FLOG(_L8("CMediaSyncDatabase::CreateTableL - Exit"));
    }

void CMediaSyncDatabase::CreateTabIndexL()
    {    
    __FLOG(_L8("CMediaSyncDatabase::CreateTabIndexL - Entry"));
    
    _LIT(KSQLCreateCombinedIndexText,"CREATE UNIQUE INDEX CombinedIndex on ImageStore (ObjectId, NotificationType)");      
    User::LeaveIfError(iDatabase.Execute(KSQLCreateCombinedIndexText));
    
    __FLOG(_L8("CMediaSyncDatabase::CreateTabIndexL - Exit"));
    }

void CMediaSyncDatabase::SaveNotificationsL(const RArray<TItemId>& aObjectIdArray, TObserverNotificationType aType, CMdESession& aSession)
    {    
    iDatabase.Begin();  

    switch (aType)
        {
        case ENotifyAdd:
            __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL Addition - Entry"));
            SaveAddNotificationsL(aObjectIdArray, aSession);
            break;
            
        case ENotifyRemove:
            __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL Remove - Entry"));
            SaveWithoutUriL(aObjectIdArray, KMssRemoval);
            break;
            
        case ENotifyModify:
            __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL Modify - Entry"));
            SaveAndCheckWithUriL(aObjectIdArray, KMssChange, aSession);
            break;
            
        default:
            __FLOG_VA((_L8("SaveNotificationsL - Unknown argument: %d"), aType));
            User::Leave(KErrArgument);
            break;
        }
   
    iDatabase.Commit();
    
    __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL - Exit"));
    }

inline TBool CMediaSyncDatabase::OptimizeL(TItemId aObjectId, TUint aType)
    {    
    return OptimizeL(aObjectId, aType, KNullDesC);
    }

void CMediaSyncDatabase::Rollback()
    {
    __ASSERT_DEBUG(iDatabase.InTransaction(), User::Invariant());
    iDatabase.Rollback();
    }

TBool CMediaSyncDatabase::OptimizeL(TItemId aObjectId, TUint aType, const TDesC& aUri)
    {
    __FLOG(_L8("CMediaSyncDatabase::OptimizeL - Entry"));
    
    TBool saveNotification = ETrue;
    
    switch (aType)
        {                
    case KMssChange:
        if ( UpdateUriColumnL(aObjectId, KMssAddition, aUri) ||
             UpdateUriColumnL(aObjectId, KMssChange, aUri) )
            {
            saveNotification = EFalse;// ignore this update notification
            }
        __FLOG_VA((_L8("OptimizeL - KMssChange ObjectId: %u, Ignore saving: %d"), aObjectId, saveNotification));
        break;
                
    case KMssPresent:
        if (RemoveNotificationL(aObjectId, KMssNotPresent))
            {
            saveNotification = EFalse;// ignore this present notification
            }
        __FLOG_VA((_L8("OptimizeL - KMssPresent ObjectId: %u, Ignore saving: %d"), aObjectId, saveNotification));
        break;        
        
    case KMssRemoval:
        if (RemoveNotificationL(aObjectId, KMssAddition))
            {
            saveNotification = EFalse;// ignore this removal notification
            }        
        else
            {
            RemoveNotificationL(aObjectId, KMssChange);
            }
        __FLOG_VA((_L8("OptimizeL - KMssRemoval ObjectId: %u, Ignore saving: %d"), aObjectId, saveNotification));
        break;
        
    case KMssNotPresent:
        if (RemoveNotificationL(aObjectId, KMssPresent))
            {
            saveNotification = EFalse;// ignore this not present notification
            }
        __FLOG_VA((_L8("OptimizeL - KMssNotPresent ObjectId: %u, Ignore saving: %d"), aObjectId, saveNotification));
        break;
        
    default:
        // Nothing to do
        break;
        }
    
    __FLOG(_L8("CMediaSyncDatabase::OptimizeL - Exit"));
    
    return saveNotification;
    }

void CMediaSyncDatabase::SaveNotificationsL(const RArray<TItemId>& aObjectIdArray, TBool aPresent, CMdESession& aSession)
    {        
    iDatabase.Begin();
   
    if (aPresent)
        {
        __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL Present - Entry"));
        SaveAndCheckWithUriL(aObjectIdArray, KMssPresent, aSession);
        }
    else
        {
        __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL Not Present - Entry"));
        SaveWithoutUriL(aObjectIdArray, KMssNotPresent);
        }      
    
    iDatabase.Commit();  
    
    __FLOG(_L8("CMediaSyncDatabase::SaveNotificationsL Present - Exit"));
    }

void CMediaSyncDatabase::SaveAddNotificationsL(const RArray<TItemId>& aObjectIdArray, CMdESession& aSession)
    {
    __FLOG(_L8("CMediaSyncDatabase::SaveAddNotificationsL - Entry"));
    
    CMdENamespaceDef& defaultNamespaceDef = aSession.GetDefaultNamespaceDefL();
    CMdEObjectDef& imageObjDef = defaultNamespaceDef.GetObjectDefL(MdeConstants::Image::KImageObject); 

    TInt objectCount = aObjectIdArray.Count();   
    for (TInt i(0);i < objectCount;i++)
        {       
        TItemId objectId = aObjectIdArray[i];
        CMdEObject* addObject = aSession.GetObjectL(objectId, imageObjDef);
        if (addObject)
            {
            CleanupStack::PushL(addObject);
            CleanupStack::PushL(TCleanupItem(CMediaSyncDatabase::RollbackTable, &iBatched));
            iBatched.InsertL();
            iBatched.SetColL(1, (TUint32)objectId);
            iBatched.SetColL(2, KMssAddition);
            iBatched.SetColL(3, addObject->Uri());
            iBatched.PutL();
            CleanupStack::Pop(&iBatched);            
            __FLOG_VA((_L16("CMediaSyncDatabase::SaveAndCheckWithUriL - ObjectId:%u, Type:%u, URI:%S"), objectId, KMssAddition, &addObject->Uri()));
            CleanupStack::PopAndDestroy(addObject); 
            }                                 
        }      
    
    __FLOG(_L8("CMediaSyncDatabase::SaveAddNotificationsL - Exit"));
    }

void CMediaSyncDatabase::SaveAndCheckWithUriL(const RArray<TItemId>& aObjectIdArray, TUint aType, CMdESession& aSession)
    {
    __FLOG(_L8("CMediaSyncDatabase::SaveAndCheckWithUriL - Entry"));
    
    CMdENamespaceDef& defaultNamespaceDef = aSession.GetDefaultNamespaceDefL();
    CMdEObjectDef& imageObjDef = defaultNamespaceDef.GetObjectDefL(MdeConstants::Image::KImageObject); 
    CMdEPropertyDef& itemTypePropDef = imageObjDef.GetPropertyDefL(MdeConstants::Object::KItemTypeProperty);    

    TInt objectCount = aObjectIdArray.Count();   
    for (TInt i(0);i < objectCount;i++)
        {       
        TItemId objectId = aObjectIdArray[i];          
        CMdEObject* changeObject = aSession.GetObjectL(objectId, imageObjDef);
        if (changeObject)
            {
            CleanupStack::PushL(changeObject);            
            //only support jpeg format image files             
            CMdEProperty* itemType = NULL;
            TInt err = changeObject->Property(itemTypePropDef, itemType);
            
            if (err >= KErrNone && itemType != NULL && itemType->TextValueL().Compare(KJpegMime) == 0)
                {                        
                if (OptimizeL(objectId, aType, changeObject->Uri()))
                    {                    
                    CleanupStack::PushL(TCleanupItem(CMediaSyncDatabase::RollbackTable, &iBatched));
                    iBatched.InsertL();
                    iBatched.SetColL(1, (TUint32)objectId);
                    iBatched.SetColL(2, aType);                    
                    iBatched.SetColL(3, changeObject->Uri());
                    iBatched.PutL();                    
                    CleanupStack::Pop(&iBatched);
                    __FLOG_VA((_L16("CMediaSyncDatabase::SaveAndCheckWithUriL - ObjectId:%u, Type:%u, URI:%S"), objectId, aType, &changeObject->Uri()));
                    }
                }
            CleanupStack::PopAndDestroy(changeObject);            
            }
        }  
    
    __FLOG(_L8("CMediaSyncDatabase::SaveAndCheckWithUriL - Exit"));
    }

void CMediaSyncDatabase::SaveWithoutUriL(const RArray<TItemId>& aObjectIdArray, TUint aType)
    {
    TInt objectCount = aObjectIdArray.Count();   
    for (TInt i(0);i < objectCount;i++)
        {       
        TItemId objectId = aObjectIdArray[i];        
        if (OptimizeL(objectId, aType))
            {
            CleanupStack::PushL(TCleanupItem(CMediaSyncDatabase::RollbackTable, &iBatched));
            iBatched.InsertL();
            iBatched.SetColL(1, (TUint32)objectId);
            iBatched.SetColL(2, aType);
            iBatched.PutL();
            __FLOG_VA((_L8("CMediaSyncDatabase::SaveWithoutUriL - ObjectId:%u, Type: %u"), objectId, aType));
            CleanupStack::Pop(&iBatched);
            }
        }
    
    __FLOG(_L8("CMediaSyncDatabase::SaveWithoutUriL - Exit"));
    }

TBool CMediaSyncDatabase::UpdateUriColumnL(TItemId aObjectId, TUint aType, const TDesC& aUri)
    {
    __FLOG(_L8("CMediaSyncDatabase::UpdateUriColumnL - Entry"));
    
    TBool update = EFalse;
    
    iBatched.SetIndex(KSQLCombinedIndex);
    TDbSeekMultiKey<2> seekKey;
    seekKey.Add((TUint)aObjectId);
    seekKey.Add(aType);
    if (iBatched.SeekL(seekKey))
        {
        CleanupStack::PushL(TCleanupItem(CMediaSyncDatabase::RollbackTable, &iBatched));
        iBatched.UpdateL();                 
        iBatched.SetColL(3, aUri);
        iBatched.PutL();        
        CleanupStack::Pop(&iBatched);
        update = ETrue;
        __FLOG_VA((_L16("CMediaSyncDatabase::UpdateUriColumnL - ObjectId:%u, Type:%u, URI:%S"), aObjectId, aType, &aUri));
        }    
    
    __FLOG(_L8("CMediaSyncDatabase::UpdateUriColumnL - Exit"));
    return update;
    }

void CMediaSyncDatabase::RemoveAllNotificationsL()
    {        
    _LIT(KSQLDeleteAllNotifications, "DELETE FROM ImageStore");
  
    User::LeaveIfError(iDatabase.Execute(KSQLDeleteAllNotifications));    
    iDatabase.Compact();    
    iSavePosition = EFalse;    
    
    __FLOG_VA((_L8("CMediaSyncDatabase::RemoveAllNotificationsL")));
    }

TBool CMediaSyncDatabase::RemoveNotificationL(TItemId aObjectId, TUint aType)
    {
    TBool remove = EFalse;
    
    iBatched.SetIndex(KSQLCombinedIndex);
    TDbSeekMultiKey<2> seekKey;
    seekKey.Add((TUint)aObjectId);
    seekKey.Add(aType);
    if (iBatched.SeekL(seekKey))
        {
        iBatched.DeleteL();
        CompactDatabase();
        iSavePosition = EFalse;
        remove = ETrue;
        __FLOG_VA((_L8("CMediaSyncDatabase::RemoveNotificationL - ObjectId:%u, Type: %u"), aObjectId, aType));
        }    
    return remove;
    }

void CMediaSyncDatabase::CompactDatabase()
    {
    if (++iCompactCounter > KCompactThreshold)
        {
        iDatabase.Compact();
        iCompactCounter = 0;
        }    
    }

void CMediaSyncDatabase::FetchNotificationsL(CMediaSyncDataWriter& aResulWriter, TInt aMaxtFetchCount, TBool& aIsFinished)
    {
    __FLOG(_L8("CMediaSyncDatabase::FetchNotificationsL - Entry"));
    
    _LIT(KSQLQuery, "SELECT ObjectId, NotificationType, URI FROM ImageStore");
    
    RDbView view;
    CleanupClosePushL(view);
    
    view.Prepare(iDatabase, TDbQuery(KSQLQuery));
    view.EvaluateAll();
    
    //goto the last fetch position
    if (iSavePosition)
        {
        view.GotoL(iBookmark);
        }
    else        
        {
        view.FirstL();   
        }
    
    TInt entrySize = 0;
    //tranvers records
    while (view.AtRow() && (aMaxtFetchCount > 0))
        {
        view.GetL();        
        TPtrC16 uri = view.ColDes16(3);
        
        entrySize = uri.Size();
        entrySize += sizeof(TUint32);//object id size
        entrySize += sizeof(TUint8);//type size 
        entrySize += sizeof(TUint8);//uri size
        
        if (entrySize > aResulWriter.FreeSpaceBytes())
            {
            //there is no enought space to save entry
            break;
            }
        else
            {
            aResulWriter.AppendEntryL(view.ColUint32(1), (TUint8)view.ColUint32(2), uri);
            view.NextL();
            --aMaxtFetchCount;
            }                                       
        }
    
    //save current fetch position
    if (view.AtEnd())
        {
        iSavePosition = EFalse;
        aIsFinished = ETrue;
        }
    else
        {
        iBookmark = view.Bookmark();
        iSavePosition = ETrue;
        aIsFinished = EFalse;
        }
    CleanupStack::PopAndDestroy(&view);
    
    __FLOG(_L8("CMediaSyncDatabase::FetchNotificationsL - Exit"));
    }

void CMediaSyncDatabase::RollbackTable(TAny* aTable)
    {
    reinterpret_cast<RDbTable*> (aTable)->Cancel();
    }
