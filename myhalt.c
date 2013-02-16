


#ifdef WITHMAIN
#include <stdio.h> 
#endif
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>

static OSStatus SendAppleEventToSystemProcess(AEEventID EventToSend);

void ShutdownNow(void)
{

    OSStatus error = noErr;
    error = SendAppleEventToSystemProcess(kAEShutDown);
    //if (error == noErr)
    //{printf("Computer is going to restart!\n");}
    //else
    //{printf("Computer wouldn't restart\n");}
    return;
}

#ifdef WITHMAIN
int main(void)
{
    const int bufferSize = 256;
    OSStatus error = noErr;
    char select [bufferSize];

    printf("1: Restart computer\n");
    printf("2: Shutdown computer\n");
    printf("3: Logout computer\n");
    printf("4: Sleep computer\n");
    printf("q: quit program\n");

    printf("please enter choice:\n");fflush(stdout);
    fgets(select, bufferSize, stdin);

    switch (select[0])
    {
        case '1':
            //sending restart event to system
            error = SendAppleEventToSystemProcess(kAERestart);
            if (error == noErr)
            {printf("Computer is going to restart!\n");}
            else
            {printf("Computer wouldn't restart\n");}
            break;
        case '2':
            //sending shutdown event to system
            error = SendAppleEventToSystemProcess(kAEShutDown);
            if (error == noErr)
            {printf("Computer is going to shutdown!\n");}
            else
            {printf("Computer wouldn't shutdown\n");}
            break;
        case '3':
            //sending logout event to system
            error = SendAppleEventToSystemProcess(kAEReallyLogOut);
            if (error == noErr)
            {printf("Computer is going to logout!\n");}
            else
            {printf("Computer wouldn't logout");}
            break;
        case '4':
            //sending sleep event to system
            error = SendAppleEventToSystemProcess(kAESleep);
            if (error == noErr)
            {printf("Computer is going to sleep!\n");}
            else
            {printf("Computer wouldn't sleep");}
    };

    return(0);
}
#endif

OSStatus SendAppleEventToSystemProcess(AEEventID EventToSend)
{
    AEAddressDesc targetDesc;
    static const ProcessSerialNumber kPSNOfSystemProcess = { 0, kSystemProcess };
    AppleEvent eventReply = {typeNull, NULL};
    AppleEvent appleEventToSend = {typeNull, NULL};

    OSStatus error = noErr;

    error = AECreateDesc(typeProcessSerialNumber, &kPSNOfSystemProcess, 
            sizeof(kPSNOfSystemProcess), &targetDesc);

    if (error != noErr)
    {
        return(error);
    }

    error = AECreateAppleEvent(kCoreEventClass, EventToSend, &targetDesc, 
            kAutoGenerateReturnID, kAnyTransactionID, &appleEventToSend);

    AEDisposeDesc(&targetDesc);
    if (error != noErr)
    {
        return(error);
    }

    error = AESend(&appleEventToSend, &eventReply, kAENoReply, 
            kAENormalPriority, kAEDefaultTimeout, NULL, NULL);

    AEDisposeDesc(&appleEventToSend);
    if (error != noErr)
    {
        return(error);
    }

    AEDisposeDesc(&eventReply);

    return(error); 
}


