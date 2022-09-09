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

#include "SolanaWallet.h"

#include "SolanaWalletManager.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "Crypto/CryptoUtils.h"
#include "Crypto/FEd25519Bip39.h"

#include "WalletAccount.h"
#include "Network/RequestManager.h"
#include "Network/RequestUtils.h"
#include "SolanaUtils/Account.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformApplicationMisc.h"
#endif

TArray<uint32> FDerivationPath::GetDerivationPathSegments(uint32 Index)
{
	TArray<uint32> Result = Segments;
	if (Result.Num() > IncreasingIndex)
	{
		Result[IncreasingIndex] = Index;
	}
	return Result;
}

bool FDerivationPath::operator==(const FDerivationPath& Other) const
{
	return Segments == Other.Segments && DerivationPath.Equals(DerivationPath);
}

FText USolanaWallet::WalletLockedText = NSLOCTEXT("Foundation", "SolanaWallet_WalletLocked", "Wallet is locked.");
FText USolanaWallet::InvalidMnemonic = NSLOCTEXT("Foundation", "SolanaWallet_InvalidMnemonic", "Invalid Mnemonic.");

USolanaWallet::USolanaWallet()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		CurrentSaveData = NewObject<UWalletData>();
	}
}

bool USolanaWallet::SetSaveSlotName(FString NewSaveSlotName)
{
	if (IsWalletLocked()) { return false; }
	if (SaveSlotName == NewSaveSlotName) { return false; }

	bool bDeleted = false;
	if (!SaveSlotName.IsEmpty())
	{
		bDeleted = UGameplayStatics::DeleteGameInSlot(USolanaWalletManager::GetSlotNamePath(SaveSlotName), 0);
	}

	SaveSlotName = NewSaveSlotName;

	const bool bDoesWalletSaveExits = DoesWalletSaveExist();
	bLocked = bDoesWalletSaveExits;

	if (bDeleted)
	{
		SaveWallet();
	}

	if (bDoesWalletSaveExits && !bDeleted)
	{
		const UWalletSaveData* SaveData = Cast<UWalletSaveData>(UGameplayStatics::LoadGameFromSlot(USolanaWalletManager::GetSlotNamePath(SaveSlotName), 0));
		if (!ensureAlways(IsValid(SaveData))) { return false; }

		PublicKeys = SaveData->PublicKeys;
		for (const auto& PublicKey : PublicKeys)
		{
			UWalletAccount* Account = NewObject<UWalletAccount>(this);
			Account->AccountData.PublicKey = PublicKey;
			Accounts.Add(PublicKey, Account);
		}
	}
	return true;
}

bool USolanaWallet::DoesWalletSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(USolanaWalletManager::GetSlotNamePath(SaveSlotName), 0);
}

bool USolanaWallet::GenerateMnemonic(FString& MnemonicString)
{
	if (IsWalletLocked())
	{
		MnemonicString = TEXT("");
		return false;
	}

	// If we already have a mnemonic or the wallet exists on disk we don't generate a new mnemonic.
	if (!Mnemonic.Mnemonic.IsEmpty())
	{
		MnemonicString = Mnemonic.Mnemonic;
		return false;
	}

	InitMnemonic(FMnemonic(24));
	MnemonicString = Mnemonic.Mnemonic;
	return true;
}

bool USolanaWallet::RestoreMnemonic(FString InMnemonic, FText& FailReason)
{
	FailReason = FText::GetEmpty();
	if (IsWalletLocked())
	{
		FailReason = WalletLockedText;
		return false;
	}

	// If we already have a mnemonic or the wallet exists on disk we don't restore a mnemonic.
	if (!Mnemonic.Mnemonic.IsEmpty())
	{
		FailReason = InvalidMnemonic;
		return false;
	}

	if (!IsMnemonicValid(InMnemonic))
	{
		FailReason = InvalidMnemonic;
		return false;
	}

	InitMnemonic(InMnemonic);
	return true;
}

bool USolanaWallet::IsMnemonicValid(FString Mnemonic)
{
	return FMnemonic::IsMnemonic(Mnemonic);
}

FString USolanaWallet::GetMnemonicString() const
{
	return Mnemonic.Mnemonic;
}

bool USolanaWallet::SetPassword(FString NewPassword)
{
	if (IsWalletLocked())
	{
		return false;
	}

	// TODO: check password length and symbols

	CurrentPassword = NewPassword;

	// Save the wallet with the new password.
	SaveWallet();

	return true;
}

bool USolanaWallet::SaveWallet()
{
	if (SaveSlotName.IsEmpty()) { return false; }
	if (IsWalletLocked()) { return false; }
	if (!IsValid(CurrentSaveData)) { return false; }

	CurrentSaveData->bLoaded = true;

	CurrentSaveData->Accounts.Empty();
	for (auto& [PublicKey, Account] : Accounts)
	{
		CurrentSaveData->Accounts.Add(Account->AccountData);
	}

	UWalletSaveData* SaveData = NewObject<UWalletSaveData>();

	FMemoryWriter MemWriter(SaveData->Data);

	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;

	CurrentSaveData->Serialize(Ar);
	SaveData->Data = FCryptoUtils::EncryptAES128GCM(SaveData->Data, CurrentPassword);
	SaveData->PublicKeys = PublicKeys;
	return UGameplayStatics::SaveGameToSlot(SaveData, USolanaWalletManager::GetSlotNamePath(SaveSlotName), 0);
}

bool USolanaWallet::UnlockWallet(FString Password, FText& FailReason)
{
	FailReason = FText::GetEmpty();
	
	if (!IsWalletLocked())
	{
		if (CurrentPassword != Password)
		{
			FailReason = NSLOCTEXT("Foundation", "SolanaWallet_UnlockWallet_WrongPassword", "Wrong Password.");
		}
		
		return CurrentPassword == Password;
	}

	if (!DoesWalletSaveExist())
	{
		FailReason = NSLOCTEXT("Foundation", "SolanaWallet_UnlockWallet_MissingSaveSlot", "Missing Wallet Save File.");
		return false;
	}

	UWalletSaveData* SaveData = Cast<UWalletSaveData>(UGameplayStatics::LoadGameFromSlot(USolanaWalletManager::GetSlotNamePath(SaveSlotName), 0));
	if (!ensureAlways(IsValid(SaveData)))
	{
		FailReason = NSLOCTEXT("Foundation", "SolanaWallet_UnlockWallet_InvalidData", "Invalid Data.");
		return false;
	}

	SaveData->Data = FCryptoUtils::DecryptAES128GCM(SaveData->Data, Password);

	FMemoryReader MemReader(SaveData->Data);

	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;

	UWalletData* WalletSaveData = NewObject<UWalletData>();
	WalletSaveData->Serialize(Ar);
	if (!WalletSaveData->bLoaded)
	{
		FailReason = NSLOCTEXT("Foundation", "SolanaWallet_UnlockWallet_InvalidPassword", "Invalid Password");
		return false;
	}

	CurrentPassword = Password;
	CurrentSaveData = WalletSaveData;

	PublicKeys.Empty();
	for (int32 i = 0; i < CurrentSaveData->Accounts.Num(); ++i)
	{
		const FAccount& AccountData = CurrentSaveData->Accounts[i];
		const FString& PublicKey = AccountData.PublicKey;
		UWalletAccount* Account;
		if (UWalletAccount* const* AccountPtr = Accounts.Find(PublicKey))
		{
			Account = *AccountPtr;
		}
		else
		{
			Account = NewObject<UWalletAccount>(this);
			Accounts.Add(PublicKey, Account);
		}
		Account->AccountData = AccountData;
		PublicKeys.Add(Account->AccountData.PublicKey);
	}

	Mnemonic = WalletSaveData->Mnemonic;
	OnMnemonicUpdated.Broadcast(Mnemonic.Mnemonic);

	bLocked = false;
	OnWalletUnlocked.Broadcast(this);

	return true;
}

void USolanaWallet::LockWallet(bool bSaveWallet)
{
	if (IsWalletLocked()) { return; }

	if (bSaveWallet)
	{
		SaveWallet();
	}

	if (IsValid(CurrentSaveData))
	{
		CurrentSaveData->MarkAsGarbage();
		CurrentSaveData = nullptr;
	}

	CurrentPassword.Empty();

	Mnemonic = FMnemonic();

	for (auto& [PublicKey, Account] : Accounts)
	{
		// TODO Account->TokenAccounts.Empty();
		Account->AccountData = FAccount::FromPublicKey(PublicKey);
	}

	bLocked = true;
	OnWalletLocked.Broadcast(this);
}

void USolanaWallet::WipeWallet()
{
	LockWallet(false);
	UGameplayStatics::DeleteGameInSlot(USolanaWalletManager::GetSlotNamePath(SaveSlotName), 0);
	bLocked = false;
	OnWalletWiped.Broadcast(this);
}

bool USolanaWallet::IsWalletLocked() const
{
	return bLocked;
}

bool USolanaWallet::SetDerivationPath(const FDerivationPath& DerivationPath)
{
	if (IsWalletLocked()) { return false; }

	CurrentSaveData->SelectedDerivationPath = DerivationPath;
	return true;
}

TArray<FDerivationPath> USolanaWallet::GetDerivationPaths()
{
	static FDerivationPath FirstDerivationPath("m/44'/501'/0'", {44, 501, 0});
	static FDerivationPath SecondDerivationPath("m/44'/501'/0'/0'", {44, 501, 0, 0});
	static TArray<FDerivationPath> DerivationPaths = {
		FirstDerivationPath,
		SecondDerivationPath
	};
	return DerivationPaths;
}

bool USolanaWallet::GetAccountsFromPath(FDerivationPath Path, int32 NumAccounts, TArray<FAccount>& OutAccounts) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(USolanaWallet::GetAccountsFromPath)

	OutAccounts.Empty(NumAccounts);
	OutAccounts.AddDefaulted(NumAccounts);

	if (Mnemonic.Mnemonic.IsEmpty()) { return false; }

	ParallelFor(NumAccounts, [&](int32 Index)
	{
		FEd25519Bip39 Keypair(Mnemonic.DeriveSeed());
		OutAccounts[Index] = FAccount::FromSeed(Keypair.DeriveAccountPath(Path.GetDerivationPathSegments(Index)));
		OutAccounts[Index].GenIndex = Index;
	});

	return true;
}

UWalletAccount* USolanaWallet::GetAccountFromGenIndex(int32 GenIndex) const
{
	if (GenIndex < 0)
	{
		return nullptr;
	}
	for (auto& [PublicKey, Account] : Accounts)
	{
		if (Account->AccountData.GenIndex == GenIndex)
		{
			return Account;
		}
	}
	return nullptr;
}

UWalletAccount* USolanaWallet::GenerateAccountFromGenIndex(int32 GenIndex)
{
	UWalletAccount* Account = GetAccountFromGenIndex(GenIndex);
	if (Account)
	{
		return Account;
	}
	Account = NewObject<UWalletAccount>(this);
	FEd25519Bip39 keypair(Mnemonic.DeriveSeed());
	FAccount AccountData = FAccount::FromSeed(keypair.DeriveAccountPath(CurrentSaveData->SelectedDerivationPath.GetDerivationPathSegments(GenIndex)));
	AccountData.Name = FString::Printf(TEXT("Wallet %i"), Accounts.Num() + 1);
	AccountData.GenIndex = GenIndex;
	Account->AccountData = AccountData;
	Accounts.Add(AccountData.PublicKey, Account);
	PublicKeys.Add(Account->AccountData.PublicKey);
	return Account;
}

uint32 USolanaWallet::GetNextAccountIndexToGenerate() const
{
	if (Mnemonic.Mnemonic.IsEmpty())
	{
		return 0;
	}

	if (Accounts.Num() == 0)
	{
		return 0;
	}

	TArray<uint32> GeneratedIndices;
	for (auto& [PublicKey, Account] : Accounts)
	{
		GeneratedIndices.AddUnique(Account->AccountData.GenIndex);
	}
	GeneratedIndices.Sort();
	for (int32 i = 0; i < GeneratedIndices.Num() - 1; ++i)
	{
		if (GeneratedIndices[i] + 1 < GeneratedIndices[i + 1])
		{
			return GeneratedIndices[i] + 1;
		}
	}
	return GeneratedIndices[GeneratedIndices.Num() - 1] + 1;
}

UWalletAccount* USolanaWallet::GenerateNewAccount()
{
	if (IsWalletLocked()) { return nullptr; }
	return GenerateAccountFromGenIndex(GetNextAccountIndexToGenerate());
}

UWalletAccount* USolanaWallet::ImportAccountFromPrivateKey(FString PrivateKey)
{
	PrivateKey.RemoveSpacesInline();
	UWalletAccount* Account = NewObject<UWalletAccount>(this);

	TArray<uint8> byteKey = FAccount::FStringToByteKey(PrivateKey);
	if( byteKey.Num() == PrivateKeySize )
	{
		Account->AccountData = FAccount::FromPrivateKey(byteKey);
	}
	else
	{
		Account->AccountData = FAccount::FromPrivateKey(PrivateKey);
	}
	
	Accounts.Add(Account->AccountData.PublicKey, Account);
	PublicKeys.Add(Account->AccountData.PublicKey);
	return Account;
}

UWalletAccount* USolanaWallet::ImportAccountFromPublicKey(FString PublicKey)
{
	UWalletAccount* Account = NewObject<UWalletAccount>(this);
	Account->AccountData = FAccount::FromPublicKey(PublicKey);
	Accounts.Add(PublicKey, Account);
	PublicKeys.Add(PublicKey);
	return Account;
}

void USolanaWallet::RemoveAccount(UWalletAccount* Account)
{
	if (!ensureAlways(IsValid(Account)))
	{
		return;
	}
	
	Accounts.Remove(Account->AccountData.PublicKey);
	PublicKeys.Remove(Account->AccountData.PublicKey);
}

void USolanaWallet::RemoveAllAccounts()
{
	for (UWalletAccount* Account : GetAccounts())
	{
		RemoveAccount(Account);
	}
}

TArray<UWalletAccount*> USolanaWallet::GetAccounts() const
{
	TArray<UWalletAccount*> Output;
	Accounts.GenerateValueArray(Output);
	return Output;
}

int32 USolanaWallet::GetAccountsCount() const
{
	if (!IsWalletLocked())
	{
		return Accounts.Num();
	}
	else
	{
		return PublicKeys.Num();
	}
}

UWalletAccount* USolanaWallet::GetAccountFromPublicKey(FString PublicKey) const
{
	if (UWalletAccount* const* AccountPtr = Accounts.Find(PublicKey))
	{
		return *AccountPtr;
	}
	return nullptr;
}

void USolanaWallet::UpdateAccounts()
{
	if (IsWalletLocked())
    {
    	return;
    }

	FRequestData* Request = FRequestUtils::RequestMultipleAccounts(GetPublicKeys());
	TArray<UWalletAccount*> CurrentAccounts = GetAccounts();
	Request->Callback.BindLambda([this, CurrentAccounts](const FJsonObject& JsonResponse)
	{
		const TArray<FAccountInfoJson> Response = FRequestUtils::ParseMultipleAccountsResponse(JsonResponse);
		for (int32 Index = 0; Index < Response.Num(); ++Index)
		{
			const FAccountInfoJson& AccountInfoJson = Response[Index];
			CurrentAccounts[Index]->UpdateFromAccountInfoJson(AccountInfoJson);
		}
		OnAccountsUpdated.Broadcast();
	});
	FRequestManager::SendRequest(Request);
}

void USolanaWallet::UpdateTokenAccounts()
{
	if (IsWalletLocked())
	{
		return;
	}

	for (UWalletAccount* Account : GetAccounts())
	{
		Account->UpdateTokenAccounts();
	}
}

void USolanaWallet::ClipboardCopy(FString String)
{
#if PLATFORM_WINDOWS
	FPlatformApplicationMisc::ClipboardCopy(*String);
#endif
}

void USolanaWallet::InitMnemonic(const FMnemonic& InMnemonic)
{
	Mnemonic = InMnemonic;
	PublicKeys.Empty();
	Accounts.Empty();
	CurrentSaveData->Mnemonic = Mnemonic.Mnemonic;
	SetDerivationPath(GetDerivationPaths()[0]);
	OnMnemonicUpdated.Broadcast(Mnemonic.Mnemonic);
}
