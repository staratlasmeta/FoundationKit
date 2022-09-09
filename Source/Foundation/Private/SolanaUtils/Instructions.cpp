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

#include "Instructions.h"

#include "SolanaUtils/Account.h"
#include "Crypto/Base58.h"
#include "Crypto/CryptoUtils.h"
#include "SolanaUtils/Utils/Types.h"

constexpr int32 SystemProgramIndex_CreateAccount = 0;
constexpr int32 SystemProgramIndex_Transfer = 2;

constexpr int32 TokenProgramIndex_InitializeAccount = 1;
constexpr int32 TokenProgramIndex_Transfer = 3;

const FString SysvarRentPublicKey = "SysvarRent111111111111111111111111111111111";

FInstructionData FInstruction::TransferLamports(const FAccount& from, const FAccount& to, int64 lamports)
{
	FInstructionData result;

	TArray<uint8> SystemProgramId;
	SystemProgramId.SetNumZeroed(PublicKeySize);
	
	result.ProgramId.Append(SystemProgramId);
	
	result.Keys.Add(FAccountMeta( from.PublicKeyData, true, true));
	result.Keys.Add(FAccountMeta( to.PublicKeyData, false, true));

	result.Keys.Add(FAccountMeta( result.ProgramId, false, false));
	
	result.Data.Append(FCryptoUtils::Int32ToDataArray(SystemProgramIndex_Transfer));
	result.Data.Append(FCryptoUtils::Int64ToDataArray(lamports));
	
	return result;
}

FInstructionData FInstruction::CreateAccount(const FAccount& from, const FAccount& newAccount, int64 rent)
{
	FInstructionData result;

	TArray<uint8> systemProgramId;
	systemProgramId.SetNumZeroed(PublicKeySize);
	
	result.ProgramId.Append(systemProgramId);
	
	result.Keys.Add(FAccountMeta( from.PublicKeyData, true, true));
	result.Keys.Add(FAccountMeta( newAccount.PublicKeyData, true, true));
	
	result.Keys.Add(FAccountMeta( result.ProgramId, false, false));
	
	result.Data.Append(FCryptoUtils::Int32ToDataArray(SystemProgramIndex_CreateAccount));
	result.Data.Append(FCryptoUtils::Int64ToDataArray(rent));
	result.Data.Append(FCryptoUtils::Int64ToDataArray(AccountDataSize));
	result.Data.Append(FBase58::DecodeBase58(TokenProgramId));
	
	return result;
}

FInstructionData FInstruction::InitializeTokenAccount(const FAccount& account, const TArray<uint8>& mint, const FAccount& owner)
{
	FInstructionData result;

	result.ProgramId.Append(FBase58::DecodeBase58(TokenProgramId));
	
	result.Keys.Add(FAccountMeta( account.PublicKeyData, false, true));
	result.Keys.Add(FAccountMeta( mint, false, false));
	result.Keys.Add(FAccountMeta( owner.PublicKeyData, false, false));
	result.Keys.Add(FAccountMeta( FBase58::DecodeBase58(SysvarRentPublicKey), false, false));

	result.Keys.Add(FAccountMeta( result.ProgramId, false, false));
	
	result.Data.Add(TokenProgramIndex_InitializeAccount);
	
	return result;
}

FInstructionData FInstruction::TransferTokens(const FAccount& from, const FAccount& to, const FAccount& owner, int64 amount)
{
	FInstructionData result;
	
	result.ProgramId.Append(FBase58::DecodeBase58(TokenProgramId));
	
	result.Keys.Add(FAccountMeta( from.PublicKeyData, false, true));
	result.Keys.Add(FAccountMeta( to.PublicKeyData, false, true));
	result.Keys.Add(FAccountMeta( owner.PublicKeyData, true, false));
	
	result.Keys.Add(FAccountMeta( result.ProgramId, false, false));

	//Finish this
	result.Data.Add(TokenProgramIndex_Transfer);
	result.Data.Append(FCryptoUtils::Int64ToDataArray(amount));
	
	return result;
}
