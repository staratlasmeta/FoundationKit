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

#include "Subsystems/GameInstanceSubsystem.h"
#include "SolanaWalletManager.generated.h"

class USolanaWallet;

UCLASS()
class FOUNDATION_API USolanaWalletManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category="Wallet")
	static TArray<FString> GetSaveSlotList();

	UFUNCTION(BlueprintPure, Category="Wallet")
	static bool DoesWalletSaveSlotExist(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category="Wallet")
	void RegisterAllWalletsFromSaveSlotList();

	UFUNCTION(BlueprintCallable)
	USolanaWallet* CreateNewWallet();

	UFUNCTION(BlueprintCallable)
	USolanaWallet* GetOrCreateWallet(const FString& SlotName);

	UFUNCTION(BlueprintCallable)
	USolanaWallet* GetWallet(const FString& SlotName);
	
	UFUNCTION(BlueprintCallable)
	void RegisterWallet(USolanaWallet* Wallet);

	UFUNCTION(BlueprintCallable)
	void UnregisterWallet(USolanaWallet* Wallet);

	UFUNCTION(BlueprintCallable)
	const TMap<FString, USolanaWallet*>& GetAllRegisteredWallets() const;

	UFUNCTION(BlueprintPure)
	static FString GetSlotNamePath(FString SlotName);

	static FString SlotNamePrefix;

	UFUNCTION(BlueprintPure)
	int32 GetRegisteredWalletCount() const;

private:

	UPROPERTY()
	TMap<FString, USolanaWallet*> Wallets;
};
