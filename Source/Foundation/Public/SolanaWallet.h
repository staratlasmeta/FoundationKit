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
#pragma once

#include "GameFramework/SaveGame.h"
#include "SolanaUtils/Account.h"
#include "SolanaUtils/Mnemonic.h"
#include "SolanaWallet.generated.h"

class UWalletAccount;

/**
 * FDerivationPath
 * 
 * Path used by the key generation algorithm to generate new private keys in a deterministic way.
 * 
 */
USTRUCT(BlueprintType)
struct FDerivationPath
{
	GENERATED_BODY()

	FDerivationPath() {}
	FDerivationPath(const FString& InDerivationPath, const TArray<uint32>& InSegments)
	{
		DerivationPath = InDerivationPath;
		Segments = InSegments;
	}

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FString DerivationPath;

	UPROPERTY(SaveGame)
	TArray<uint32> Segments;

	int32 IncreasingIndex = 2;

	TArray<uint32> GetDerivationPathSegments(uint32 Index);

	FString ToString() const { return DerivationPath; }

	bool operator==(const FDerivationPath& Other) const;
};

/**
 * UWalletSaveData
 * 
 * Utility class to save wallet data.
 * 
 */
UCLASS()
class UWalletData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(SaveGame)
	FString Mnemonic;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FDerivationPath SelectedDerivationPath;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	TArray<FAccount> Accounts;

	UPROPERTY(SaveGame)
	bool bLoaded = false;
};

UCLASS()
class UWalletSaveData : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<uint8> Data;

	UPROPERTY()
	TArray<FString> PublicKeys;
};


/**
 * USolanaWallet
 * 
 * This class abstract a wallet for the solana network and it is made up of:
 * - a mnemonic phrase to generate new accounts;
 * - a derivation path to generate new accounts;
 * - a save slot name to save the wallet on disk;
 * - a password to encrypt the wallet on disk;
 * - a list of accounts either generated from the mnemonic phrase or imported from a public or private key;
 * 
 */
UCLASS(BlueprintType)
class FOUNDATION_API USolanaWallet : public UObject
{
	GENERATED_BODY()

public:

	USolanaWallet();

	// Set the name of the file used to load or save this wallet.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	bool SetSaveSlotName(FString NewSaveSlotName);

	// Get the name of the file used to load or save this wallet.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	const FString& GetSaveSlotName() const { return SaveSlotName; }

	// Check if there is an existing file for this wallet.
	UFUNCTION(BlueprintPure, Category="Wallet")
	bool DoesWalletSaveExist() const;

	// Generate a mnemonic if no mnemonic exists in this wallet.
	UFUNCTION(BlueprintCallable, Category="Mnemonic")
	bool GenerateMnemonic(FString& MnemonicString);

	// Restore a mnemonic if no mnemonic exists in this wallet.
	UFUNCTION(BlueprintCallable, Category="Mnemonic")
	bool RestoreMnemonic(FString InMnemonic, FText& FailReason);

	// Check if a Mnemonic string is valid.
	UFUNCTION(BlueprintPure, Category="Mnemonic")
	static bool IsMnemonicValid(FString Mnemonic);

	// Get the Mnemonic string of this wallet.
	UFUNCTION(BlueprintPure, Category="Mnemonic")
	FString GetMnemonicString() const;

	// Called when mnemonic is set, loaded or erased;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMnemonicUpdated, FString, Mnemonic);
	UPROPERTY(BlueprintAssignable, Category="Mnemonic")
	FOnMnemonicUpdated OnMnemonicUpdated;

	// Set or change the password.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	bool SetPassword(FString NewPassword);

	// Save this wallet to disk to reload it later.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	bool SaveWallet();

	// Load and unlock this wallet from disk if password is correct.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	bool UnlockWallet(FString Password, FText& FailReason);

	// Lock the wallet, deleting mnemonic and private keys from memory.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	void LockWallet(bool bSaveWallet);

	// Called the wallet get locked.
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWalletLocked, USolanaWallet*, Wallet);
	UPROPERTY(BlueprintAssignable, Category="Wallet")
	FOnWalletLocked OnWalletLocked;

	// Called the wallet get unlocked using the right password.
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWalletUnlocked, USolanaWallet*, Wallet);
	UPROPERTY(BlueprintAssignable, Category="Wallet")
	FOnWalletUnlocked OnWalletUnlocked;

	// Wipe the wallet from both memory and disk.
	UFUNCTION(BlueprintCallable, Category="Wallet")
	void WipeWallet();

	// Called when the wallet get wiped;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWalletWiped, USolanaWallet*, Wallet);
	UPROPERTY(BlueprintAssignable, Category="Wallet")
	FOnWalletWiped OnWalletWiped;

	// Whether the wallet is locked or not.
	UFUNCTION(BlueprintPure, Category="Wallet")
	bool IsWalletLocked() const;

	// Set the derivation path for this wallet to derive new wallet address.
	UFUNCTION(BlueprintCallable, Category="Derivation Path")
	bool SetDerivationPath(const FDerivationPath& DerivationPath);

	// Get all available derivation paths.
	UFUNCTION(BlueprintCallable, Category="Derivation Path")
	static TArray<FDerivationPath> GetDerivationPaths();

	// Get accounts for a specific derivation path.
	UFUNCTION(BlueprintCallable, Category="Account")
	bool GetAccountsFromPath(FDerivationPath Path, int32 NumAccounts, TArray<FAccount>& OutAccounts) const;

	// Get the account corresponding to the given generation index if it has been already generated.
	UFUNCTION(BlueprintPure, Category="Account")
	UWalletAccount* GetAccountFromGenIndex(int32 GenIndex) const;

	// Get account from a generation index using the selected derivation path. 
	UFUNCTION(BlueprintCallable, Category="Account")
	UWalletAccount* GenerateAccountFromGenIndex(int32 GenIndex);

	// Get the index of the next account to generate.
	uint32 GetNextAccountIndexToGenerate() const;

	// Generate a new account increasing the generation index.
	UFUNCTION(BlueprintCallable, Category="Account")
	UWalletAccount* GenerateNewAccount();

	// Create an account from a private key.
	UFUNCTION(BlueprintCallable, Category="Account")
	UWalletAccount* ImportAccountFromPrivateKey(FString PrivateKey);

	// Create an account from a public key.
	UFUNCTION(BlueprintCallable, Category="Account")
	UWalletAccount* ImportAccountFromPublicKey(FString PublicKey);

	// Remove an account from this wallet.
	UFUNCTION(BlueprintCallable, Category="Account")
	void RemoveAccount(UWalletAccount* Account);

	// Remove an account from this wallet.
	UFUNCTION(BlueprintCallable, Category="Account")
	void RemoveAllAccounts();
	
	// Get all accounts in this wallet.
	UFUNCTION(BlueprintPure, Category="Account")
	TArray<UWalletAccount*> GetAccounts() const;

	// Get the number of accounts within the wallet.
	UFUNCTION(BlueprintPure, Category="Account")
	int32 GetAccountsCount() const;

	// Get the account from the public key.
	UFUNCTION(BlueprintPure, Category="Account")
	UWalletAccount* GetAccountFromPublicKey(FString PublicKey) const;

	// Get public keys of the accounts in this wallet.
	UFUNCTION(BlueprintPure, Category="Account")
	TArray<FString> GetPublicKeys() const { return PublicKeys; }

	// Update balance and info for all accounts
	UFUNCTION(BlueprintCallable, Category="Account")
	void UpdateAccounts();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAccountsUpdated);
	UPROPERTY(BlueprintAssignable, Category="Account")
	FOnAccountsUpdated OnAccountsUpdated;

	UFUNCTION(BlueprintCallable, Category="Account")
	void UpdateTokenAccounts();

	// Copy the string parameter to the system clipboard.
	UFUNCTION(BlueprintCallable)
	static void ClipboardCopy(FString String);

private:

	bool bLocked = false;

	void InitMnemonic(const FMnemonic& InMnemonic);

	FMnemonic Mnemonic;

	UPROPERTY()
	TMap<FString, UWalletAccount*> Accounts;

	UPROPERTY()
	UWalletData* CurrentSaveData;

	UPROPERTY()
	FString SaveSlotName = TEXT("");

	UPROPERTY()
	FString CurrentPassword;

	UPROPERTY()
	TArray<FString> PublicKeys;

	static FText WalletLockedText;
	static FText InvalidMnemonic;
};
