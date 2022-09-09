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

struct FAccount;
struct FAccountMeta;
struct FInstructionData;

class FTransaction
{
public:

	FTransaction(const FString& currentBlockHash);

	void AddInstruction(const FInstructionData& instruction);
	void AddInstructions(const TArray<FInstructionData>& instructions);
	
	TArray<uint8> Build(const FAccount& signer);
	TArray<uint8> Build(const TArray<FAccount>& signers);

	static TArray<uint8> Sign(const TArray<uint8>& message, const TArray<FAccount>& signers);

private:

	TArray<uint8> BuildMessage();
	TArray<uint8> CompileInstructions();

	void UpdateAccountList(const TArray<FAccount>& signers);
	void UpdateHeaderInfo(const FAccountMeta& accountMeta);

	uint8 GetAccountIndex(const FString& key) const;

	TArray<FInstructionData> Instructions;
	TArray<FAccountMeta> AccountList;

	FString BlockHash;

	uint8 RequiredSignatures;
	uint8 ReadOnlySignedAccounts;
	uint8 ReadOnlyUnsignedAccounts;
};
