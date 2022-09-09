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

#include "ExampleUI/GenerateSecretRecoveryPhraseWidget.h"

#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformApplicationMisc.h"
#endif

void UGenerateSecretRecoveryPhraseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ContinueButton->SetIsEnabled(false);
	SecretSavedCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnSecretSavedCheckStateChanged);
	CopyButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCopyButtonClicked);
}

void UGenerateSecretRecoveryPhraseWidget::NativeDestruct()
{
	SecretSavedCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::OnSecretSavedCheckStateChanged);
	CopyButton->OnClicked.RemoveDynamic(this, &ThisClass::OnCopyButtonClicked);
	Super::NativeDestruct();
}

void UGenerateSecretRecoveryPhraseWidget::OnSecretSavedCheckStateChanged(bool bIsChecked)
{
	ContinueButton->SetIsEnabled(bIsChecked);
}

void UGenerateSecretRecoveryPhraseWidget::OnCopyButtonClicked()
{
	/*
	// TODO: Other platforms?
	#if PLATFORM_WINDOWS
		FPlatformApplicationMisc::ClipboardCopy(*RecoveryPhraseTextBox->GetText().ToString());
	#endif
	*/
}

void UGenerateSecretRecoveryPhraseWidget::SetMnemonic(FString Mnemonic)
{
	RecoveryPhraseTextBox->SetText(FText::FromString(Mnemonic));
}
