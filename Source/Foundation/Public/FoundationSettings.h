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

*/
#pragma once

#include "CoreMinimal.h"
#include "FoundationSettings.generated.h"

UENUM()
enum class ESolanaNetwork : uint8
{
	None,
	MainNetBeta,
	DevNet,
	// add above
	Count UMETA(Hidden)
};

UCLASS(Config = Foundation, DefaultConfig, BlueprintType, meta = (DisplayName = "Foundation Settings"))
class FOUNDATION_API UFoundationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Foundation"); }

	UFUNCTION(BlueprintPure)
	ESolanaNetwork GetNetwork() const;

	UFUNCTION(BlueprintCallable)
	void SetNetwork(ESolanaNetwork SolanaNetwork) { Network = SolanaNetwork; }

	UFUNCTION(BlueprintPure)
	FString GetNetworkURL() const;

protected:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Config)
	TMap<ESolanaNetwork, FString> NetworkURLs;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Config)
	ESolanaNetwork Network = ESolanaNetwork::DevNet;
};