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
#pragma once

#include "Account.generated.h"

USTRUCT(BlueprintType)
struct FOUNDATION_API FAccount
{
	GENERATED_BODY()

	FAccount();

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString Name;

	UPROPERTY(SaveGame,BlueprintReadOnly)
	int32 GenIndex = -1;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FString PublicKey;
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FString PrivateKey;

	TArray<uint8> PublicKeyData;
	TArray<uint8> PrivateKeyData;

	TArray<uint8> Sign(const TArray<uint8>& Transaction);
	void Verify(const TArray<uint8>& Transaction, const TArray<uint8>& Signature);

	static FAccount FromSeed(const TArray<uint8>& Seed);

	static FAccount FromPrivateKey(const FString& PrivateKey);
	static FAccount FromPrivateKey(const TArray<uint8>& PrivateKey);

	static FAccount FromPublicKey(const FString& PrivateKey);
	static FAccount FromPublicKey(const TArray<uint8>& PublicKey);

	static bool IsBase58PrivateKey(const FString& Key);
	static bool IsBytePrivateKey(const FString& Key);

	static TArray<uint8> FStringToByteKey(const FString& Key);

	static FString GetShortDisplayablePublicKey(const FString& PublicKey, int32 InitialCharsCount=6, int32 FinalCharsCount=4);
	
};
