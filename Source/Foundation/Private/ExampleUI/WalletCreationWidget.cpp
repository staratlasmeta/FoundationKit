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

#include "ExampleUI/WalletCreationWidget.h"

#include "SolanaWalletManager.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

#include "ExampleUI/HomeWalletCreationWidget.h"
#include "ExampleUI/SelectDerivationPathWidget.h"
#include "ExampleUI/GenerateSecretRecoveryPhraseWidget.h"
#include "ExampleUI/UseSecretRecoveryPhraseWidget.h"
#include "ExampleUI/CreatePasswordWidget.h"

void UWalletCreationWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SolanaWalletManager = GetGameInstance()->GetSubsystem<USolanaWalletManager>();
}

void UWalletCreationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Switcher->SetActiveWidget(HomeWidget);
	HomeWidget->CreateNewWalletButton->OnClicked.AddUniqueDynamic(this, &UWalletCreationWidget::OnCreateNewWalletButtonClicked);
	HomeWidget->UseSecretRecoveryPhrase->OnClicked.AddUniqueDynamic(this, &UWalletCreationWidget::OnUseSecretRecoveryPhraseButtonClicked);
}

void UWalletCreationWidget::NativeDestruct()
{
	HomeWidget->CreateNewWalletButton->OnClicked.RemoveDynamic(this, &UWalletCreationWidget::OnCreateNewWalletButtonClicked);
	HomeWidget->UseSecretRecoveryPhrase->OnClicked.RemoveDynamic(this, &UWalletCreationWidget::OnUseSecretRecoveryPhraseButtonClicked);
	Super::NativeDestruct();
}

void UWalletCreationWidget::OnCreateNewWalletButtonClicked()
{
	HomeWidget->CreateNewWalletButton->OnClicked.RemoveDynamic(this, &UWalletCreationWidget::OnCreateNewWalletButtonClicked);
	HomeWidget->UseSecretRecoveryPhrase->OnClicked.RemoveDynamic(this, &UWalletCreationWidget::OnUseSecretRecoveryPhraseButtonClicked);

	SolanaWallet = SolanaWalletManager->GetOrCreateWallet("ExampleUIWallet");

	FString Mnemonic;
	SolanaWallet->GenerateMnemonic(Mnemonic);
	
	GenerateSecretRecoveryPhraseWidget->SetMnemonic(Mnemonic);
	
	Switcher->SetActiveWidget(GenerateSecretRecoveryPhraseWidget);
	GenerateSecretRecoveryPhraseWidget->ContinueButton->OnClicked.AddUniqueDynamic(this, &UWalletCreationWidget::OnUserSavedMnemonic);
}

void UWalletCreationWidget::OnUseSecretRecoveryPhraseButtonClicked()
{
	SolanaWallet = SolanaWalletManager->GetOrCreateWallet("ExampleUIWallet");
	
	HomeWidget->UseSecretRecoveryPhrase->OnClicked.RemoveDynamic(this, &UWalletCreationWidget::OnUseSecretRecoveryPhraseButtonClicked);
	Switcher->SetActiveWidget(UseSecretRecoveryPhraseWidget);
	UseSecretRecoveryPhraseWidget->OnMnemonicSubmitted.AddUniqueDynamic(this, &UWalletCreationWidget::OnMnemonicSubmitted);
}

void UWalletCreationWidget::OnMnemonicSubmitted(const FString& Mnemonic)
{
	if (SolanaWallet->IsMnemonicValid(Mnemonic))
	{
		FText FailureReason;
		SolanaWallet->RestoreMnemonic(Mnemonic, FailureReason);
		UseSecretRecoveryPhraseWidget->OnMnemonicSubmitted.RemoveDynamic(this, &UWalletCreationWidget::OnMnemonicSubmitted);
		Switcher->SetActiveWidget(SelectDerivationPath);
		SelectDerivationPath->OnDerivationPathSelected.AddUniqueDynamic(this, &UWalletCreationWidget::OnDerivationPathSelected);
	}
}

void UWalletCreationWidget::OnDerivationPathSelected(const FDerivationPath& Path)
{
	SelectDerivationPath->OnDerivationPathSelected.RemoveDynamic(this, &UWalletCreationWidget::OnDerivationPathSelected);
	SolanaWallet->SetDerivationPath(Path);
	Switcher->SetActiveWidget(CreatePasswordWidget);
	CreatePasswordWidget->OnPasswordConfirmed.AddUniqueDynamic(this, &UWalletCreationWidget::OnPasswordConfirmed);
}

void UWalletCreationWidget::OnPasswordConfirmed(const FString& Password)
{
	CreatePasswordWidget->OnPasswordConfirmed.RemoveDynamic(this, &UWalletCreationWidget::OnPasswordConfirmed);
	
	SolanaWallet->SetPassword(Password);
	
	if (SolanaWallet->GetAccounts().IsEmpty())
	{
		SolanaWallet->GenerateNewAccount();
	}
	
	SolanaWallet->SaveWallet();
	OnWalletCreated.Broadcast();
}

void UWalletCreationWidget::OnUserSavedMnemonic()
{
	OnDerivationPathSelected(SolanaWallet->GetDerivationPaths()[0]);
}
