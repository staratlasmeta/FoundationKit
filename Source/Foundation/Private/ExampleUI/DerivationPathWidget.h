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
#include "DerivationPathWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Blueprintable)
class FOUNDATION_API UDerivationPathWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ButtonContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* AddressTextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* DerivationPathTextBlock;
};
