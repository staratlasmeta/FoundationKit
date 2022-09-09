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

#include "SolanaUtils/Mnemonic.h"

#include "SolanaUtils/Utils/Types.h"
#include "Crypto/CryptoUtils.h"
#include "SolanaUtils/Utils/HardcodedWordList.h"

#include <bitset>

const int WordCount = 24;
const int EntrophyValue = 256 / 8;

FMnemonic::FMnemonic()
{

}

FMnemonic::FMnemonic(const FString& words)
{
	if( IsMnemonic(words) )
	{
		Mnemonic = words;
	}
	else
	{
		//Error
	}
}

FMnemonic::FMnemonic(int wordCount)
{
	Mnemonic = GenerateSentence(wordCount);
}

TArray<uint8> FMnemonic::GenerateEntropy()
{
	TArray<uint8> entropy;
	entropy.SetNum(PublicKeySize);

	if(!FCryptoUtils::RandomBytes(entropy, PublicKeySize))
	{
		//Error
	}

	TArray<uint8> checksum = FCryptoUtils::SHA256_Digest(entropy.GetData(),entropy.Num());
	entropy.Add(checksum[0]);

	return entropy;
}

FString FMnemonic::GenerateSentence(int wordCount)
{
	FString sentence;

	TArray<uint8> entropy = GenerateEntropy();
	TArray<uint32> indices = FCryptoUtils::SplitBytesByBits(entropy, 11);

	TArray<FString> wordList;
	FHardcodedWordList::GetWordList().ParseIntoArray(wordList,TEXT("\n"));

	for(int i = 0; i < wordCount; i++)
	{
		sentence.Append(wordList[indices[i]]);
		if( i < wordCount - 1 )
		{
			sentence.Append(" ");
		}
	}

	return sentence;
}

TArray<uint8> FMnemonic::DeriveSeed() const
{
	return DeriveSeed(Mnemonic);
}

TArray<uint8> FMnemonic::DeriveSeed(const FString& mnemonic)
{
	TArray<uint8> string = FCryptoUtils::FStringToUint8(mnemonic);
	TArray<uint8> salt = { 'm','n','e','m','o','n','i','c' };
	return FCryptoUtils::GenerateSeed((char*)string.GetData(), string.Num() , salt.GetData(), salt.Num());
}

bool FMnemonic::IsMnemonic(const FString& MnemonicString)
{
	TArray<FString> MnemonicWords;
	const int32 NumWords = MnemonicString.ParseIntoArrayWS(MnemonicWords, nullptr, true);

	if (!(NumWords >= 12 && NumWords <= 24))
	{
		return false;
	}

	if (NumWords % 3 != 0)
	{
		return false;
	}

	TArray<FString> wordList;
	FHardcodedWordList::GetWordList().ParseIntoArray(wordList,TEXT("\n"));

	const uint32 NumBits = NumWords * 11;
	uint32 EntropyLen = NumBits / 8;
	uint32 ChecksumNumBits = NumBits % 8;
	if (ChecksumNumBits == 0)
	{
		--EntropyLen;
		ChecksumNumBits = 8;
	}

	TArray<uint8> Entropy;
	Entropy.SetNum(EntropyLen);

	FString BitString = FString();
	for (const FString& MnemonicWord : MnemonicWords)
	{
		const int32 Index = wordList.Find(MnemonicWord);
		if (Index == INDEX_NONE)
		{
			return false;
		}
		BitString += UTF8_TO_TCHAR(std::bitset<11>(Index).to_string().c_str());
	}

	for (int32 Index = 0; Index < Entropy.Num(); ++Index)
	{
		const FString SubString = BitString.Mid(Index * 8, 8);
		Entropy[Index] = std::bitset<8>(TCHAR_TO_UTF8(*SubString)).to_ulong();
	}

	TArray<uint8> CalculatedChecksum = FCryptoUtils::SHA256_Digest(Entropy.GetData(), Entropy.Num());
	const FString ExpectedChecksumString = FString(UTF8_TO_TCHAR(std::bitset<8>(CalculatedChecksum[0]).to_string().c_str())).Left(ChecksumNumBits);

	const FString InputChecksumString = BitString.Right(ChecksumNumBits);

	return InputChecksumString == ExpectedChecksumString;
}
