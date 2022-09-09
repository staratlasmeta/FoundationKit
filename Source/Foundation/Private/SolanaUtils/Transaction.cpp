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

#include "Transaction.h"

#include "SolanaUtils/Account.h"
#include "Instructions.h"
#include "Crypto/Base58.h"
#include "Crypto/CryptoUtils.h"

FTransaction::FTransaction(const FString& currentBlockHash)
{
	BlockHash = currentBlockHash;

	RequiredSignatures = 0;
	ReadOnlySignedAccounts = 0;
	ReadOnlyUnsignedAccounts = 0;
}

void FTransaction::AddInstruction(const FInstructionData& instruction)
{
	Instructions.Add(instruction);
	for( const FAccountMeta& data: instruction.Keys)
	{
		int index = AccountList.IndexOfByPredicate([data](const FAccountMeta& entry){ return data.PublicKeyData == entry.PublicKeyData; } );
		if(index == INDEX_NONE)
		{
			AccountList.Add(data);
		}
		else
		{
			if( data.Writable && !AccountList[index].Writable )
			{
				AccountList[index] = data;
			}
		}
	}
}

void FTransaction::AddInstructions(const TArray<FInstructionData>& instructions)
{
	Instructions.Append(instructions);
	for(const FInstructionData& instruction: instructions)
	{
		AddInstruction(instruction);
	}
}

uint8 FTransaction::GetAccountIndex(const FString& key) const
{
	return AccountList.IndexOfByPredicate([key](const FAccountMeta& data)
	{
		return data.PublicKey == key;
	});
}

TArray<uint8> FTransaction::Build(const FAccount& signer)
{
	TArray<FAccount> signers;
	signers.Add(signer);
	return Build(signers);
}

TArray<uint8> FTransaction::Build(const TArray<FAccount>& signers)
{
	UpdateAccountList(signers);

	const TArray<uint8> message = BuildMessage();

	TArray<uint8> result = Sign(message, signers);
	result.Append(message);
	
	return result;
}

void FTransaction::UpdateAccountList(const TArray<FAccount>& signers)
{
	for (const FAccount& account : signers)
	{
		int index = 0;
		while( index != INDEX_NONE )
		{
			index = AccountList.IndexOfByPredicate([account](const FAccountMeta& entry){ return account.PublicKeyData == entry.PublicKeyData; } );
			if( index != INDEX_NONE )
			{
				AccountList.RemoveAt(index);
			}
		}
	}

	//Sort writables to the top of list before readding Signers at the very top
	AccountList.Sort([](const FAccountMeta& A, const FAccountMeta& B) { return A.Writable && !B.Writable; });

	for (int i = 0; i < signers.Num(); i++)
	{
		AccountList.Insert( FAccountMeta( signers[i].PublicKeyData, true, true), i);
	}
}

TArray<uint8> FTransaction::BuildMessage()
{
    TArray<uint8> accountKeysBuffer;
    for (FAccountMeta& accountMeta : AccountList)
    {
    	//Need to remove feepayer here????
        accountKeysBuffer.Append(accountMeta.PublicKeyData);
		UpdateHeaderInfo(accountMeta);
    }

	TArray<uint8> buffer;
	buffer.Add(RequiredSignatures);
	buffer.Add(ReadOnlySignedAccounts);
	buffer.Add(ReadOnlyUnsignedAccounts);
	
	buffer.Append(FCryptoUtils::ShortVectorEncodeLength(AccountList.Num()));
	buffer.Append(accountKeysBuffer);

	buffer.Append( FBase58::DecodeBase58(BlockHash));

	TArray<uint8> compiledInstructions = CompileInstructions();
	buffer.Append(FCryptoUtils::ShortVectorEncodeLength(Instructions.Num()));
	buffer.Append(compiledInstructions);

	return buffer;
}

TArray<uint8> FTransaction::CompileInstructions()
{
	TArray<uint8> result;
	
	for (FInstructionData& instruction: Instructions)
	{
		const int keyCount = instruction.Keys.Num() - 1;
		TArray<uint8> keyIndices;
		keyIndices.SetNum(keyCount);
	
		for (int i = 0; i < keyCount; i++)
		{
			keyIndices[i] = GetAccountIndex(instruction.Keys[i].PublicKey);
		}

		result.Add(GetAccountIndex(FBase58::EncodeBase58(instruction.ProgramId.GetData(),instruction.ProgramId.Num())));
		result.Append(FCryptoUtils::ShortVectorEncodeLength(keyCount));
		result.Append(keyIndices);
		result.Append(FCryptoUtils::ShortVectorEncodeLength(instruction.Data.Num()));
		result.Append(instruction.Data);
	}

	return result;
}

void FTransaction::UpdateHeaderInfo(const FAccountMeta& accountMeta)
{
	if (accountMeta.Signer)
	{
		RequiredSignatures += 1;
		if (!accountMeta.Writable)
			ReadOnlySignedAccounts += 1;
	}
	else 
	{
		if (!accountMeta.Writable)
			ReadOnlyUnsignedAccounts += 1;
	}
}

TArray<uint8> FTransaction::Sign(const TArray<uint8>& message, const TArray<FAccount>& signers)
{
	TArray<uint8> signatures;

	signatures.Append(FCryptoUtils::ShortVectorEncodeLength(signers.Num()));
	
	for(FAccount signer: signers)
	{
		signatures.Append( signer.Sign(message) );
	}

	return signatures;
}