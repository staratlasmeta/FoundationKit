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
#include "Crypto/Base58.h"

#include "CryptoUtils.h"
#include "Math/BigInt.h"

const char ALPHABET[58] = {
	'1', '2', '3', '4', '5', '6', '7', '8',
	'9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
	'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
	'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z'
};

TArray<uint8> FBase58::DecodeBase58(const FString& encoded)
{
	TArray<uint8> result;
	
	char* s = TCHAR_TO_UTF8(*encoded);
	FString alphabet = ALPHABET;
	
	TBigInt<1024> intData;
	for (int i = 0; i < encoded.Len(); i++)
	{
		int digit = -1;
		unsigned char temp = s[i];
		if (!alphabet.FindChar(temp, digit) || digit < 0 )
		{
			//Error
		}
		intData = intData * 58 + digit;
	}
	
	uint32* bits = intData.GetBits();
	const int bitEndIndex = (1024 / 32) - 1;
	for(int k = bitEndIndex; k >= 0 ; k--)
	{
		if(bits[k] != 0)
		{
			result.Append(FCryptoUtils::Int32ToDataArrayBE(bits[k]));
		}
	}

	return result;
}

FString FBase58::EncodeBase58(const uint8* data, int len)
{
	TBigInt<1024> intData = 0;
	for (int i = 0; i < len; i++)
	{
		intData = intData * 256 + data[i];
	}

	std::string result = "";
	while (intData.IsGreater(0))
	{
		TBigInt<1024> remainder = intData % 58;
		intData /= 58;
		result = ALPHABET[remainder.ToInt()] + result;
	}
	
	for (int i = 0; i < len && data[i] == 0; i++)
	{
		result = '1' + result;
	}
	
	return result.c_str();
}
