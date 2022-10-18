#pragma once

#include "CoreMinimal.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

DECLARE_DELEGATE_OneParam( RequestCallback, FJsonObject&);
DECLARE_DELEGATE_OneParam( RequestErrorCallback, const FText& FailureReason);

typedef TFunctionRef<void(FJsonObject&)> RequestCB;

struct FOUNDATION_API FRequestData
{
	FRequestData() {}
	FRequestData( UINT id ) { Id = id; }

	UINT Id;
	FString Body;
	RequestCallback Callback;
	RequestErrorCallback ErrorCallback;
};

class FOUNDATION_API FRequestManager_WB{
    public:
        TSharedPtr<IWebSocket> WebSocket;
        virtual void Init() override;
        virtual void Shutdown() override;

        static int64 GetNextMessageID();
	    static int64 GetLastMessageID();
        static void SendRequest(FRequestData* RequestData);
        static void CancelRequest(FRequestData* RequestData);
    private:
	    static void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
}
