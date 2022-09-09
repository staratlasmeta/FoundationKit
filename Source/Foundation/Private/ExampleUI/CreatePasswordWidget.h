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
#include "CreatePasswordWidget.generated.h"

class UButton;
class UEditableTextBox;

UCLASS(Blueprintable)
class FOUNDATION_API UCreatePasswordWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPasswordConfirmed, const FString&, Password);
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnPasswordConfirmed OnPasswordConfirmed;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnPasswordChanged(const FText& Text);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool ArePasswordsValid() const;

	UFUNCTION()
	void OnPasswordConfirmedEvent();

	UPROPERTY(BlueprintReadOnly)
	int32 PasswordMinLength = 8;

	UPROPERTY(BlueprintReadOnly)
	int32 PasswordMaxLength = 16;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* PasswordEditableTextBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* ConfirmPasswordEditableTextBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ContinueButton;
};
