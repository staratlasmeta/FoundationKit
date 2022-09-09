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
#include "GenerateSecretRecoveryPhraseWidget.generated.h"

class UCheckBox;

class UButton;
class UMultiLineEditableTextBox;

UCLASS(Blueprintable)
class FOUNDATION_API UGenerateSecretRecoveryPhraseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetMnemonic(FString Mnemonic);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnSecretSavedCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnCopyButtonClicked();

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ContinueButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CopyButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* SecretSavedCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMultiLineEditableTextBox* RecoveryPhraseTextBox;
};
