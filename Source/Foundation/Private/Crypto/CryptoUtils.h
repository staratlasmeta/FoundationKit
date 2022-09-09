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
Contributers: Daniele Calanna
*/
#pragma once

class FCryptoUtils
{
public:

	static TArray<uint8> SHA256_Digest(const uint8* Data, uint32 Size);
	static TArray<uint8> SHA512_Digest(const uint8* Data, uint32 Size);

	static TArray<uint8> HMAC_SHA512(const TArray<uint8>& Data, const FString& Key);
	static TArray<uint8> HMAC_SHA512(const TArray<uint8>& Key, const TArray<uint8>& Data);

	static TArray<uint8> GenerateSeed(const char* Mnemonic, int MnemonicSize, const unsigned char*  Salt, int SaltSize);
	static void GenerateKeyPair(const TArray<uint8>& Seed, TArray<uint8>& OutPublicKey, TArray<uint8>& OutPrivateKey );

	static void SignMessage(TArray<uint8>& Signature, const TArray<uint8>& Message, const TArray<uint8>& PrivateKey);
	static void VerifyMessage(const TArray<uint8>& Signature, const TArray<uint8>& Message,TArray<uint8>& PublicKey);
	
	static bool RandomBytes(TArray<uint8>& Salt, int32 Length);

	static TArray<uint8> Int32ToDataArray(int32 integer);
	static TArray<uint8> Int64ToDataArray(int64 integer);

	static TArray<uint8> Int32ToDataArrayBE(int32 integer);
	static TArray<uint8> Int64ToDataArrayBE(int64 integer);

	static TArray<uint32> SplitBytesByBits(const TArray<uint8>& data, int bitIncrements);
	static TBitArray<> BytesToBitArray(const TArray<uint8>& data);
	
	static TArray<uint8> FStringToUint8(const FString& string);

	static TArray<uint8> ShortVectorEncodeLength(int32 len);

	static TArray<uint8> EncryptAES128GCM(const TArray<uint8>& Data, const FString& Password);
	static TArray<uint8> DecryptAES128GCM(const TArray<uint8>& EncryptedData, const FString& Password);
};
