#pragma once

#include "CoreMinimal.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

DECLARE_DELEGATE_OneParam(FJsonObject&);

typedef TFunctionRef<void(FJsonObject&)> RequestCB;

struct FOUNDATION_API FRequestData
{
	FRequestData() {}
	FRequestData( UINT id ) { Id = id; }

	UINT Id;
	FString Body;
};

class FOUNDATION_API FRequestManager_WB:  public UObject{
    GENERATED_BODY()
public:
    DECLARE_EVENT(FRequestManager_WB, FSocketConnected);
    TSharedPtr<IWebSocket> WebSocket;
    virtual void Init() override;
    virtual void Shutdown() override;

    int64 GetNextMessageID();
    int64 GetLastMessageID();

    void SendRequest(FRequestData* RequestData);
    void CancelRequest(FRequestData* RequestData);

private:
    FSocketConnected OnConnected;
    void OnResponse(const FString &Response);
    void OnConnected_Helper();
}
