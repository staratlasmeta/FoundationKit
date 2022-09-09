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

Author: Jon Sawler
*/
#pragma once
#include "Crypto/Base58.h"
#include "SolanaUtils/Utils/Types.h"

struct FAccount;

struct FAccountMeta
{
	FAccountMeta(const TArray<uint8>& publicKeyData, bool signer, bool writable)
		: PublicKey(FBase58::EncodeBase58(publicKeyData.GetData(),publicKeyData.Num())),
			PublicKeyData(publicKeyData), Signer(signer), Writable(writable)
	{
		if( PublicKeyData.Num() < PublicKeySize )
		{
			PublicKeyData.AddZeroed( PublicKeySize - PublicKeyData.Num() );
		}
	}
	
	FString PublicKey;
	TArray<uint8> PublicKeyData;
	bool Signer;
	bool Writable;
};

struct FInstructionData
{
	TArray<uint8> ProgramId;
	TArray<FAccountMeta> Keys;
	TArray<uint8> Data;
};

class FInstruction
{
public:

	static FInstructionData TransferLamports(const FAccount& from, const FAccount& to, int64 lamports);
	static FInstructionData CreateAccount(const FAccount& from, const FAccount& newAccount, int64 rent);

	static FInstructionData InitializeTokenAccount(const FAccount& account, const TArray<uint8>& mint, const FAccount& owner);
	static FInstructionData TransferTokens(const FAccount& from, const FAccount& to, const FAccount& owner, int64 amount);
};
