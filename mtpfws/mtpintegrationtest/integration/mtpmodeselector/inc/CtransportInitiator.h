// This material, including documentation and any related computer
// programs, is protected by copyright controlled by Nokia. All
// rights are reserved. Copying, including reproducing, storing
// adapting or translating, any or all of this material requires the
// prior written consent of Nokia. This material also contains
// confidential information which may not be disclosed to others
// without the prior written consent of Nokia.



/**
 @file CtransportInitiator.h
*/
 #ifndef __CTRANSPORTINITIATOR_H__
 #define __CTRANSPORTINITIATOR_H__
 #include <e32base.h>
 #include <f32file.h>
 #include <bautils.h> 
 #include <mtp/mtpdataproviderapitypes.h>
 
 
 _LIT(KCMTPTest, "c:\\mtptest\\");
_LIT(KZMTPTest, "z:\\mtptest\\");
/**
 class CtransportInitiator
 Descibes creation of subset drives 
 Running and stopping the transport as wished by the user
*/ 
 class CtransportInitiator: public CBase
	 {
	 public :
	      static CtransportInitiator* NewL();
	      ~CtransportInitiator();
	      
	 private:
	     void ConstructL();	 
	     CtransportInitiator();
	 
	 public :
	     void SetModeL(TMTPOperationalMode aMode);
	     void RunMtpL();
	     TInt DeleteMTPTestStorages();
	     TInt CreateMTPTestStoragesL();    	
	 };
	 
#endif	 

	 
 
 
 
 
 
