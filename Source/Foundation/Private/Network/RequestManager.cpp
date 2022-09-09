/*
Copyright 2022 ATMTA, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author: Jon Sawler
Contributers: Daniele Calanna, Riccardo Torrisi
*/


#include "Network/RequestManager.h"

#include "HttpModule.h"
#include "Network/RequestUtils.h"
#include "Interfaces/IHttpResponse.h"

#include "FoundationSettings.h"

DECLARE_LOG_CATEGORY_CLASS(RequestManager, Log, All);

static int64 LastMessageID = 0;
static TArray<FRequestData*> PendingRequests;

int64 FRequestManager::GetNextMessageID()
{
	return LastMessageID++;
}

int64 FRequestManager::GetLastMessageID()
{
	return LastMessageID;
}

void FRequestManager::SendRequest(FRequestData* RequestData)
{
	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	FString Url = GetDefault<UFoundationSettings>()->GetNetworkURL();
	if( Url.IsEmpty() )
	{
		Url = GetDefault<UFoundationSettings>()->GetNetwork() == ESolanaNetwork::DevNet ? "https://api.devnet.solana.com" : "https://api.mainnet-beta.solana.com";
	}
	Request->SetURL(Url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(RequestData->Body);

	Request->OnProcessRequestComplete().BindStatic(&FRequestManager::OnResponse);
	Request->ProcessRequest();

	PendingRequests.Push(RequestData);
}

void FRequestManager::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	FString response = Response->GetContentAsString();
	if (!bSuccess)
	{
		FRequestUtils::DisplayError("Http Request Failed");
		return;
	}

	TSharedPtr<FJsonObject> ParsedJSON;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<>::Create(Response.Get()->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, ParsedJSON))
	{
		const TSharedPtr<FJsonObject>* outObject;
		if(!ParsedJSON->TryGetObjectField("error", outObject))
		{
			int id = ParsedJSON->GetIntegerField("id");
            if( PendingRequests.Num() > 0 )
            {
            	FRequestData* request = *PendingRequests.FindByPredicate([&](FRequestData* data){return data->Id == id;});
            	if(request)
            	{
            		request->Callback.Execute(*ParsedJSON);
            		PendingRequests.Remove(request);
            		delete request;
            	}
            }
		}
		else
		{
			const TSharedPtr<FJsonObject> error = ParsedJSON->GetObjectField("error");
			FRequestUtils::DisplayError(error->GetStringField("message"));
		}
	}
	else
	{
		FRequestUtils::DisplayError("Failed to parse Response from the server");
	}
}

void FRequestManager::CancelRequest(FRequestData* RequestData)
{
	if (RequestData)
	{
		RequestData->Callback.Unbind();
		RequestData->ErrorCallback.Unbind();
	}
}
