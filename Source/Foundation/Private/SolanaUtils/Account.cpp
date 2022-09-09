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
Contributers: Daniele Calanna, Riccardo Torrisi
*/

#include "SolanaUtils/Account.h"

#include "SolanaUtils/Utils/Types.h"
#include "Crypto/Base58.h"
#include "Crypto/CryptoUtils.h"

FAccount::FAccount()
{
	PublicKeyData.SetNum(PublicKeySize);
	PrivateKeyData.SetNum(PrivateKeySize);
}

TArray<uint8> FAccount::Sign(const TArray<uint8>& Transaction)
{
	TArray<uint8> Signature;
	Signature.SetNum(PrivateKeySize);
	TArray<uint8> Message;
	FCryptoUtils::SignMessage(Signature, Transaction, PrivateKeyData);
	return Signature;
}

void FAccount::Verify(const TArray<uint8>& Transaction, const TArray<uint8>& Signature)
{
	FCryptoUtils::VerifyMessage(Signature, Transaction,PublicKeyData);
}

FAccount FAccount::FromSeed( const TArray<uint8>& Seed )
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FAccount::FromSeed)
	
	FAccount newAccount;

	FCryptoUtils::GenerateKeyPair(Seed,newAccount.PublicKeyData, newAccount.PrivateKeyData );

	newAccount.PublicKey = FBase58::EncodeBase58(newAccount.PublicKeyData.GetData(), newAccount.PublicKeyData.Num());
	newAccount.PrivateKey = FBase58::EncodeBase58(newAccount.PrivateKeyData.GetData(), newAccount.PrivateKeyData.Num());

	return newAccount;
}

FAccount FAccount::FromPrivateKey(const FString& privateKey)
{
	FAccount newAccount;

	newAccount.PrivateKey = privateKey;
	newAccount.PrivateKeyData = FBase58::DecodeBase58(privateKey);

	for(int i = 0; i < PublicKeySize; i++)
	{
		newAccount.PublicKeyData[i] = newAccount.PrivateKeyData[i + PublicKeySize];
	}

	newAccount.PublicKey = FBase58::EncodeBase58(newAccount.PublicKeyData.GetData(), newAccount.PublicKeyData.Num());

	return newAccount;
}

FAccount FAccount::FromPrivateKey( const TArray<uint8>& PrivateKey )
{
	FAccount newAccount;

	newAccount.PrivateKeyData = PrivateKey;
	for(int i = 0; i < PublicKeySize; i++)
	{
		newAccount.PublicKeyData[i] = PrivateKey[i + PublicKeySize];
	}

	newAccount.PublicKey = FBase58::EncodeBase58(newAccount.PublicKeyData.GetData(), newAccount.PublicKeyData.Num());
	newAccount.PrivateKey = FBase58::EncodeBase58(newAccount.PrivateKeyData.GetData(), newAccount.PrivateKeyData.Num());

	return newAccount;
}

FAccount FAccount::FromPublicKey(const FString& publicKey)
{
	FAccount newAccount;

	newAccount.PublicKeyData = FBase58::DecodeBase58(publicKey);
	newAccount.PublicKey = publicKey;

	return newAccount;
}

FAccount FAccount::FromPublicKey(const TArray<uint8>& publicKey)
{
	FAccount newAccount;

	newAccount.PublicKeyData = publicKey;
	newAccount.PublicKey = FBase58::EncodeBase58(newAccount.PublicKeyData.GetData(), newAccount.PublicKeyData.Num());

	return newAccount;
}

bool FAccount::IsBase58PrivateKey(const FString& Key)
{
	if( Key.Len() == Base58PrKeySize && !Key.Contains(" ") && !Key.Contains(",") )
	{
		TArray<uint8> privateKey = FBase58::DecodeBase58(Key);
		return ( privateKey.Num() == PrivateKeySize );
	}
	return false;
}

bool FAccount::IsBytePrivateKey(const FString& Key)
{
	TArray<uint8> key = FStringToByteKey(Key);
	return ( key.Num() == PrivateKeySize );
}

TArray<uint8> FAccount::FStringToByteKey(const FString& Key)
{
	TArray<uint8> result;

	if( Key.StartsWith("[") )
	{
		FString key = Key.LeftChop(1);
        key = key.RightChop(1);

		TArray<FString> charNumbers;
		FString right = FString("temp");
		FString left;

		while(!right.IsEmpty())
		{
			key.Split(",", &left, &right);
			if( key == right )
			{
				result.Add(FCString::Atoi(*right));
				break;
			}
			if( !left.IsEmpty() )
			{
				result.Add(FCString::Atoi(*left));
			}

			key = right;
		}
	}

	return result;
}

FString FAccount::GetShortDisplayablePublicKey(const FString& PublicKey, int32 InitialCharsCount, int32 FinalCharsCount)
{
	if (!ensureAlwaysMsgf(PublicKey.Len() >= InitialCharsCount + FinalCharsCount,
		TEXT("PublicKey has not enought chars for the specified parameters.")))
	{
		return PublicKey;
	}
	
	return FString::Format(TEXT("{0}...{1}"),
		{ PublicKey.Left(InitialCharsCount), PublicKey.Right(FinalCharsCount) }
	);
}
