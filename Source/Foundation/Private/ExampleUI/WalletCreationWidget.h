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

Author: Riccardo Torrisi
Contributers: Daniele Calanna, Federico Arona
*/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WalletCreationWidget.generated.h"

struct FDerivationPath;

class USolanaWallet;
class USolanaWalletManager;

class UHomeWalletCreationWidget;
class UWidgetSwitcher;
class UCreatePasswordWidget;
class USelectDerivationPathWidget;
class UGenerateSecretRecoveryPhraseWidget;
class UUseSecretRecoveryPhraseWidget;

UCLASS(Blueprintable)
class FOUNDATION_API UWalletCreationWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnCreateNewWalletButtonClicked();

	UFUNCTION()
	void OnUseSecretRecoveryPhraseButtonClicked();

	UFUNCTION()
	void OnMnemonicSubmitted(const FString& Mnemonic);

	UFUNCTION()
	void OnDerivationPathSelected(const FDerivationPath& Path);

	UFUNCTION()
	void OnPasswordConfirmed(const FString& Password);

	UFUNCTION()
	void OnUserSavedMnemonic();

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHomeWalletCreationWidget* HomeWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UGenerateSecretRecoveryPhraseWidget* GenerateSecretRecoveryPhraseWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UUseSecretRecoveryPhraseWidget* UseSecretRecoveryPhraseWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USelectDerivationPathWidget* SelectDerivationPath;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCreatePasswordWidget* CreatePasswordWidget;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWalletCreated);
	UPROPERTY(BlueprintAssignable)
	FOnWalletCreated OnWalletCreated;

protected:
	
	UPROPERTY(Transient)
	USolanaWalletManager* SolanaWalletManager;

	UPROPERTY(BlueprintReadOnly)
	USolanaWallet* SolanaWallet;
};
