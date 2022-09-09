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
Contributers: Daniele Calanna, Riccardo Torrisi, Federico Arona
*/
#pragma once

#include "CoreMinimal.h"

/**
 * FMnemonic
 * 
 * Sequence of worlds representing an entropy to be used to generate private keys.
 * The structure is human readable and writable to avoid errors when writing down or restoring it.
 * 
 */
class FMnemonic
{
public:

	FMnemonic();
	FMnemonic(int wordCount);
	FMnemonic(const FString& words);

	FString Mnemonic;

	TArray<uint8> DeriveSeed() const;

	static TArray<uint8> DeriveSeed(const FString& mnemonic);

	static FString GenerateSentence(int wordCount);
	static TArray<uint8> GenerateEntropy();

	static bool IsMnemonic(const FString& MnemonicString);
};
