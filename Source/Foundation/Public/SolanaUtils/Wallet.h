
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
Contributers: Daniele Calanna, Federico Arona
*/
#pragma once

#include "CoreMinimal.h"
#include "SolanaUtils/Account.h"
#include "SolanaUtils/Mnemonic.h"
#include "UObject/NoExportTypes.h"
#include "Wallet.generated.h"

typedef TFunctionRef<void()> WalletUpdateCB;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWalletDelegate, class UWallet*, Wallet);

USTRUCT(BlueprintType)
struct FAccountData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString Pubkey;
	
	UPROPERTY(BlueprintReadOnly)
	FString Mint;
	
	UPROPERTY(BlueprintReadOnly)
	int64 Balance = 0;
};

UCLASS()
class FOUNDATION_API UWallet : public UObject
{
	GENERATED_BODY()

public:

	UWallet();
	virtual ~UWallet() override;

	UFUNCTION(BlueprintCallable)
	void UpdateWalletData();
	UFUNCTION(BlueprintCallable)
	void UpdateWalletBalance();
	UFUNCTION(BlueprintCallable)
	void UpdateAccountBalance(const FString& pubKey);
	UFUNCTION(BlueprintCallable)
	void UpdateAllTokenAccounts();
	UFUNCTION(BlueprintCallable)
	void CheckPossibleAccounts(const TArray<FString>& pubKeys);

	void SendSOL(const FAccount& from, const FAccount& to, int64 amount) const;
	void SendSOLEstimate(const FAccount& from, const FAccount& to, int64 amount) const;

	void SendToken(const FAccount& from, const FAccount& to, const FString& mint, int64 amount) const;
	void SendTokenEstimate(const FAccount& from, const FAccount& to, const FString& mint, int64 amount) const;

	UFUNCTION(BlueprintCallable)
	void SetPublicKey(const FString& pubKey);

	static bool IsValidPublicKey( const FString& pubKey);
	static bool IsValidPublicKey( const TArray<FString>& pubKeys );

	const FAccountData* GetAccountByMint(const FString& mint);

	UPROPERTY(BlueprintReadOnly)
	FString PublicKey;

	UPROPERTY(BlueprintReadOnly)
	double SOLBalance = 0;

	UPROPERTY(BlueprintAssignable)
	FWalletDelegate OnWalletUpdated;

	UPROPERTY(BlueprintAssignable)
	FWalletDelegate OnAccountsUpdated;

	FAccount Account;

	FMnemonic Mnemonic;

private:

	TArray<FAccountData> TokenAccounts;
};
