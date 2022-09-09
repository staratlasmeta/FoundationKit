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
*/

#include "Network/RequestUtils.h"

#include "JsonObjectConverter.h"
#include "Network/RequestManager.h"
#include "Misc/MessageDialog.h"
#include "SolanaUtils/Utils/Types.h"

static FText ErrorTitle = FText::FromString("Error");
static FText InfoTitle = FText::FromString("Info");

FRequestData* FRequestUtils::RequestAccountInfo(const FString& pubKey)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());

	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%u,"method":"getAccountInfo","params":["%s",{"encoding": "base58"}]})")
		,request->Id, *pubKey );

	return request;
}

FAccountInfoJson FRequestUtils::ParseAccountInfoResponse(const FJsonObject& data)
{
	FAccountInfoJson jsonData;
	if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
	{
		const TSharedPtr<FJsonObject> resultData = result->GetObjectField("value");

		FString OutputString;
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(resultData.ToSharedRef(), Writer);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, OutputString);

		FJsonObjectConverter::JsonObjectToUStruct(resultData.ToSharedRef(), &jsonData);

		FString Out = FString::Printf(TEXT("lamports: %f, owner: %s, rentEpoch: %i"), jsonData.lamports, *jsonData.owner, jsonData.rentEpoch);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, Out);

	}
	return jsonData;
}

FRequestData* FRequestUtils::RequestAccountBalance(const FString& pubKey)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method":"getBalance","params":["%s",{"commitment": "processed"}]})")
			,request->Id , *pubKey );
	
	return request;
}


double FRequestUtils::ParseAccountBalanceResponse(const FJsonObject& data)
{
	FBalanceResultJson jsonData;
	if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
	{
		FJsonObjectConverter::JsonObjectToUStruct(result.ToSharedRef(), &jsonData);
		return jsonData.value;
	}
	return -1;
}

FRequestData* FRequestUtils::RequestTokenAccount(const FString& pubKey, const FString& mint)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method":"getTokenAccountsByOwner","params":["%s",{"mint": "%s"},{"encoding": "jsonParsed"}]})")
			,request->Id, *pubKey, *mint );
	
	return request;
}

FString FRequestUtils::ParseTokenAccountResponse(const FJsonObject& data)
{
	FTokenAccountArrayJson jsonData = ParseAllTokenAccountsResponse(data);

	FString result;
	if( !jsonData.value.IsEmpty() )
	{
		result = jsonData.value[0].pubkey;
	}
	return result;
}

FRequestData* FRequestUtils::RequestAllTokenAccounts(const FString& pubKey, const FString& programID)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method":"getTokenAccountsByOwner","params":["%s",{"programId": "%s"},{"encoding": "jsonParsed"}]})")
			,request->Id, *pubKey, *programID );
	
	return request;
}

FTokenAccountArrayJson FRequestUtils::ParseAllTokenAccountsResponse(const FJsonObject& data)
{
	FTokenAccountArrayJson jsonData;
	if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
	{
		FJsonObjectConverter::JsonObjectToUStruct(result.ToSharedRef(), &jsonData);
	}
	return jsonData;
}

FRequestData* FRequestUtils::RequestProgramAccounts(const FString& programID, const UINT& size, const FString& pubKey)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method":"getProgramAccounts","params":["%s",{"encoding":"base64","filters":[{"dataSize":%d},{"memcmp":{"offset":8,"bytes":"%s"}}]}]})")
			,request->Id, *programID, size,*pubKey );
		
	return request;
}

TArray<FProgramAccountJson> FRequestUtils::ParseProgramAccountsResponse(const FJsonObject& data)
{
	TArray<FProgramAccountJson> list;
	TArray<TSharedPtr<FJsonValue>> dataArray = data.GetArrayField("result");
	for(const TSharedPtr<FJsonValue> entry:  dataArray )
	{
		const TSharedPtr<FJsonObject> entryObject = entry->AsObject();
		if( TSharedPtr<FJsonObject> account = entryObject->GetObjectField("account") )
		{
			FProgramAccountJson accountData;
			FJsonObjectConverter::JsonObjectToUStruct( account.ToSharedRef() , &accountData);
			list.Add(accountData);
		}
		FString pubKey = entryObject->GetStringField("pubkey");
	}
	
	return list;
}

FRequestData* FRequestUtils::RequestMultipleAccounts(const TArray<FString>& pubKey)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	FString list;
	for( FString key: pubKey )
	{
		list.Append(FString::Printf(TEXT(R"("%s")"),*key));

		if(key != pubKey.Last())
		{
			list.Append(",");
		}
	}
		
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method": "getMultipleAccounts","params":[[%s],{"dataSlice":{"offset":0,"length":0}}]})")
			,request->Id , *list );
	
	return request;
}

TArray<FAccountInfoJson> FRequestUtils::ParseMultipleAccountsResponse(const FJsonObject& data)
{
	TArray<FAccountInfoJson> jsonData;
	if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
	{
		FJsonObjectConverter::JsonArrayToUStruct( result->GetArrayField("value") , &jsonData);
	}
	return jsonData;
}

FRequestData* FRequestUtils::SendTransaction(const FString& transaction)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method":"sendTransaction","params":["%s",{"encoding": "base64"}]})")
			,request->Id, *transaction );
	
	return request;
}

FString FRequestUtils::ParseTransactionResponse(const FJsonObject& data)
{
	return data.GetStringField("result");
}

FRequestData* FRequestUtils::RequestBlockHash()
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"id":%d,"jsonrpc":"2.0","method":"getRecentBlockhash","params":[{"commitment":"processed"}]})")
				,request->Id );
	
	return request;
}

FString FRequestUtils::ParseBlockHashResponse(const FJsonObject& data)
{
	FString hash;
	if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
	{
		const TSharedPtr<FJsonObject> value = result->GetObjectField("value");
		hash = value->GetStringField("blockhash");
	}
	return hash;
}

FRequestData* FRequestUtils::GetTransactionFeeAmount(const FString& transaction)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d,"method":"getFeeForMessage", "params":[%s,{"commitment":"processed"}]})")
			,request->Id, *transaction );
	
	return request;
}

int FRequestUtils::ParseTransactionFeeAmountResponse(const FJsonObject& data)
{
	int fee = 0;
	if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
	{
		const TSharedPtr<FJsonObject> value = result->GetObjectField("value");
		fee = value->GetNumberField("value");
	}
	return fee;
}

FRequestData* FRequestUtils::RequestAirDrop(const FString& pubKey)
{
	FRequestData* request = new FRequestData(FRequestManager::GetNextMessageID());
	
	request->Body =
		FString::Printf(TEXT(R"({"jsonrpc":"2.0","id":%d, "method":"requestAirdrop", "params":["%s", 1000000000]})")
				,request->Id, *pubKey );
	
	return request;
}

void FRequestUtils::DisplayError(const FString& error)
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(error), &ErrorTitle);
}

void FRequestUtils::DisplayInfo(const FString& info)
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(info), &InfoTitle);
}
