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

Author: Daniele Calanna
Contributers: Riccardo Torrisi, Federico Arona
*/
#pragma once

#include "SolanaUtils/Wallet.h"
#include "TokenAccount.generated.h"

/**
 * UTokenAccount
 * 
 * This class abstract a token account that belongs to a wallet account.
 * It has its own address and mint.
 * 
 */
UCLASS(BlueprintType)
class UTokenAccount : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FAccountData AccountData;

	void Update();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBalanceUpdated, UTokenAccount*, TokenAccount, float, Balance);
	UPROPERTY(BlueprintAssignable)
	FOnBalanceUpdated OnBalanceUpdated;

	UFUNCTION(BlueprintCallable)
	void Send(FString PublicKey, float Amount);

};
