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

#include "SolanaWalletManager.h"

#include "SolanaWallet.h"

FString USolanaWalletManager::SlotNamePrefix = TEXT("Wallets/");

template <class FunctorType>
class PlatformFileFunctor : public IPlatformFile::FDirectoryVisitor //GenericPlatformFile.h
{
public:
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		return Functor(FilenameOrDirectory, bIsDirectory);
	}

	PlatformFileFunctor(FunctorType&& FunctorInstance)
		: Functor(MoveTemp(FunctorInstance))
	{
	}

private:
	FunctorType Functor;
};

template <class Functor>
PlatformFileFunctor<Functor> MakeDirectoryVisitor(Functor&& FunctorInstance)
{
	return PlatformFileFunctor<Functor>(MoveTemp(FunctorInstance));
}

static bool GetFiles(const FString& FullPathOfBaseDir, TArray<FString>& FilenamesOut, bool Recursive = false,
                     const FString& FilterByExtension = "")
{
	//Format File Extension, remove the "." if present
	const FString FileExt = FilterByExtension.Replace(TEXT("."),TEXT("")).ToLower();

	FString Str;
	auto FilenamesVisitor = MakeDirectoryVisitor(
		[&](const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			//Files
			if (! bIsDirectory)
			{
				//Filter by Extension
				if (FileExt != "")
				{
					Str = FPaths::GetCleanFilename(FilenameOrDirectory);

					//Filter by Extension
					if (FPaths::GetExtension(Str).ToLower() == FileExt)
					{
						if (Recursive)
						{
							FilenamesOut.Push(FilenameOrDirectory); //need whole path for recursive
						}
						else
						{
							FilenamesOut.Push(Str);
						}
					}
				}

				//Include All Filenames!
				else
				{
					//Just the Directory
					Str = FPaths::GetCleanFilename(FilenameOrDirectory);

					if (Recursive)
					{
						FilenamesOut.Push(FilenameOrDirectory); //need whole path for recursive
					}
					else
					{
						FilenamesOut.Push(Str);
					}
				}
			}
			return true;
		}
	);
	if (Recursive)
	{
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(
			*FullPathOfBaseDir, FilenamesVisitor);
	}
	else
	{
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*FullPathOfBaseDir, FilenamesVisitor);
	}
}

void USolanaWalletManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterAllWalletsFromSaveSlotList();
}

TArray<FString> USolanaWalletManager::GetSaveSlotList()
{
	TArray<FString> FileNames;
	const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + "SaveGames/" + SlotNamePrefix;
	GetFiles(Path, FileNames, false, ".sav");
	return FileNames;
}

bool USolanaWalletManager::DoesWalletSaveSlotExist(const FString& SlotName)
{
	const TArray<FString> SlotNames = GetSaveSlotList();
	const FString& SlotNameExt = FString::Printf(TEXT("%s.sav"), *SlotName);
	return SlotNames.Contains(SlotNameExt);
}

void USolanaWalletManager::RegisterAllWalletsFromSaveSlotList()
{
	TArray<FString> SaveSlotList = GetSaveSlotList();

	for (const FString& SaveSlotName : SaveSlotList)
	{
		const FString SaveSlotNameNoPath = FPaths::GetBaseFilename(SaveSlotName);
		GetOrCreateWallet(SaveSlotNameNoPath);
	}
}

USolanaWallet* USolanaWalletManager::CreateNewWallet()
{
	return NewObject<USolanaWallet>(this);
}

USolanaWallet* USolanaWalletManager::GetOrCreateWallet(const FString& SlotName)
{
	USolanaWallet** WalletPtr = Wallets.Find(SlotName);

	if (WalletPtr && IsValid(*WalletPtr))
	{
		return *WalletPtr;
	}

	USolanaWallet* Wallet = NewObject<USolanaWallet>(this);
	if (Wallet->SetSaveSlotName(SlotName))
	{
		Wallets.Add(SlotName, Wallet);
	}

	return Wallet;
}

USolanaWallet* USolanaWalletManager::GetWallet(const FString& SlotName)
{
	USolanaWallet** WalletPtr = Wallets.Find(SlotName);

	if (ensureAlways(WalletPtr && IsValid(*WalletPtr)))
	{
		return *WalletPtr;
	}
	
	return nullptr;
}

void USolanaWalletManager::RegisterWallet(USolanaWallet* Wallet)
{
	if (!ensureAlways(IsValid(Wallet))) { return; }

	const FString& CurrentSaveSlotName = Wallet->GetSaveSlotName();
	if (!ensureAlways(!CurrentSaveSlotName.IsEmpty())) { return; }
	if (!ensureAlways(!Wallets.Contains(CurrentSaveSlotName)))
	{
		return;
	}

	Wallets.Add(CurrentSaveSlotName, Wallet);
}

void USolanaWalletManager::UnregisterWallet(USolanaWallet* Wallet)
{
	if (!ensureAlways(IsValid(Wallet))) { return; }

	const FString& CurrentSaveSlotName = Wallet->GetSaveSlotName();
	if (!ensureAlways(!CurrentSaveSlotName.IsEmpty())) { return; }
	if (!ensureAlways(Wallets.Contains(CurrentSaveSlotName)))
	{
		return;
	}

	Wallets.Remove(Wallet->GetSaveSlotName());
}

const TMap<FString, USolanaWallet*>& USolanaWalletManager::GetAllRegisteredWallets() const
{
	return Wallets;
}

FString USolanaWalletManager::GetSlotNamePath(FString SlotName)
{
	return SlotNamePrefix + SlotName;
}

int32 USolanaWalletManager::GetRegisteredWalletCount() const
{
	const TMap<FString, USolanaWallet*>& RegisteredWallets = GetAllRegisteredWallets();

	TArray<FString> WalletSlots;
	RegisteredWallets.GetKeys(WalletSlots);

	return WalletSlots.Num();
}
