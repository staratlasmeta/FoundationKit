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

#include "ExampleUI/CreatePasswordWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void UCreatePasswordWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ContinueButton->OnClicked.AddUniqueDynamic(this, &UCreatePasswordWidget::OnPasswordConfirmedEvent);
}

void UCreatePasswordWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PasswordEditableTextBox->OnTextChanged.AddUniqueDynamic(this, &UCreatePasswordWidget::OnPasswordChanged);
	ConfirmPasswordEditableTextBox->OnTextChanged.AddUniqueDynamic(this, &UCreatePasswordWidget::OnPasswordChanged);
	ContinueButton->SetIsEnabled(false);
}

void UCreatePasswordWidget::NativeDestruct()
{
	PasswordEditableTextBox->OnTextChanged.RemoveDynamic(this, &UCreatePasswordWidget::OnPasswordChanged);
	ConfirmPasswordEditableTextBox->OnTextChanged.RemoveDynamic(this, &UCreatePasswordWidget::OnPasswordChanged);
	Super::NativeDestruct();
}

void UCreatePasswordWidget::OnPasswordChanged(const FText& Text)
{
	ContinueButton->SetIsEnabled(ArePasswordsValid());
}

bool UCreatePasswordWidget::ArePasswordsValid() const
{
	const FString Password = PasswordEditableTextBox->GetText().ToString();
	const FString ConfirmPassword = ConfirmPasswordEditableTextBox->GetText().ToString();
	if (Password.Len() < PasswordMinLength || Password.Len() > PasswordMaxLength)
	{
		return false;
	}
	return Password.Compare(ConfirmPassword) == 0;
}

void UCreatePasswordWidget::OnPasswordConfirmedEvent()
{
	OnPasswordConfirmed.Broadcast(PasswordEditableTextBox->GetText().ToString());
}
