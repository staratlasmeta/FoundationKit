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
#pragma once

#include "CoreMinimal.h"

struct FRequestData;
struct FAccountInfoJson;
struct FBalanceResultJson;
struct FTokenAccountArrayJson;
struct FProgramAccountJson;

class FOUNDATION_API FRequestUtils
{
public:
	
	static FRequestData* RequestAccountInfo(const FString& pubKey);
	static FAccountInfoJson ParseAccountInfoResponse(const FJsonObject& data);

	static FRequestData* RequestAccountBalance(const FString& pubKey);
	static double ParseAccountBalanceResponse(const FJsonObject& data);

	static FRequestData* RequestTokenAccount(const FString& pubKey, const FString& mint);
	static FString ParseTokenAccountResponse(const FJsonObject& data);

	static FRequestData* RequestAllTokenAccounts(const FString& pubKey, const FString& programID);
	static FTokenAccountArrayJson ParseAllTokenAccountsResponse(const FJsonObject& data);

	static FRequestData* RequestProgramAccounts(const FString& programID, const UINT& size, const FString& pubKey);
	static TArray<FProgramAccountJson> ParseProgramAccountsResponse(const FJsonObject& data);

	static FRequestData* RequestMultipleAccounts(const TArray<FString>& pubKey);
	static TArray<FAccountInfoJson> ParseMultipleAccountsResponse(const FJsonObject& data);
	
	static FRequestData* RequestBlockHash();
	static FString ParseBlockHashResponse(const FJsonObject& data);

	static FRequestData* GetTransactionFeeAmount(const FString& transaction);
	static int ParseTransactionFeeAmountResponse(const FJsonObject& data);
	
	static FRequestData* SendTransaction(const FString& transaction);
	static FString ParseTransactionResponse(const FJsonObject& data);
	
	static FRequestData* RequestAirDrop(const FString& pubKey);

	static void DisplayError(const FString& error);
	static void DisplayInfo(const FString& info);
};
