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
#include "CryptoUtils.h"

#include "Crypto/ed25519/ed25519.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include <openssl/aes.h>
#include <openssl/evp.h>
#include "openssl/pem.h"
#include "openssl/rand.h"
#include "openssl/sha.h"
THIRD_PARTY_INCLUDES_END
#undef UI

#include <string>
#include <vector>

using std::string;
using std::vector;

TArray<uint8> FCryptoUtils::SHA256_Digest(const uint8* Data, uint32 Size)
{
	TArray<uint8> hash;
	hash.SetNum(32);
	SHA256(Data, Size, hash.GetData());
	return hash;
}

TArray<uint8> FCryptoUtils::SHA512_Digest(const uint8* Data, uint32 Size)
{
	TArray<uint8> Hash;
	Hash.SetNum(64);
	ed25519_sha512(Data, Size,Hash.GetData());
	return Hash;
}

TArray<uint8> FCryptoUtils::HMAC_SHA512(const TArray<uint8>& Data, const FString& Key)
{
	TArray<uint8> key;
	
	for(int i = 0; i < Key.Len(); i++)
	{
		FString str = Key.Mid(i,1);
		key.Add(*TCHAR_TO_UTF8(*str));
	}

	return HMAC_SHA512(Data, key);
}

TArray<uint8> FCryptoUtils::HMAC_SHA512(const TArray<uint8>& Data, const TArray<uint8>& Key)
{
	EVP_MD_CTX* mdctx = NULL;
	EVP_PKEY* pkey = NULL;
	
	TArray<uint8> Hash;
	Hash.SetNum(EVP_MAX_MD_SIZE);

	size_t md_len = 0; 
	const EVP_MD* evp = EVP_sha512();
	mdctx = EVP_MD_CTX_create();

	if (mdctx)
	{
		pkey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, Key.GetData(), Key.Num());
	};
	
	if (pkey)
	{
		if (1 == EVP_DigestSignInit(mdctx, NULL, evp, NULL, pkey))
		{
			if (1 == EVP_DigestSignUpdate(mdctx, Data.GetData(), Data.Num()))
			{
				if (1 == EVP_DigestSignFinal(mdctx, Hash.GetData(), &md_len))
				{
					Hash.SetNum(md_len);		
				}
			};
		};
	}

	EVP_PKEY_free(pkey);
	
	return Hash;
}

TArray<uint8> FCryptoUtils::GenerateSeed(const char* Mnemonic, int MnemonicSize, const unsigned char* Salt, int SaltSize)
{
	TArray<uint8> seed;
	seed.SetNumZeroed(64);
	PKCS5_PBKDF2_HMAC(Mnemonic, MnemonicSize, Salt, SaltSize, 2048u, EVP_sha512() , 64, seed.GetData());
	return seed;
}

void FCryptoUtils::GenerateKeyPair(const TArray<uint8>& Seed, TArray<uint8>& OutPublicKey, TArray<uint8>& OutPrivateKey)
{
	ed25519_create_keypair(OutPublicKey.GetData(), OutPrivateKey.GetData(), Seed.GetData());
}

void FCryptoUtils::SignMessage(TArray<uint8>& Signature, const TArray<uint8>& Message, const TArray<uint8>& PrivateKey)
{
	ed25519_sign(Signature.GetData(), Message.GetData(), Message.Num(), PrivateKey.GetData());
}

void FCryptoUtils::VerifyMessage(const TArray<uint8>& Signature, const TArray<uint8>& Message,TArray<uint8>& PublicKey)
{
	ed25519_verify(Signature.GetData(), Message.GetData(), Message.Num(), PublicKey.GetData());
}

bool FCryptoUtils::RandomBytes(TArray<uint8>& Salt, int32 Length)
{
	if (Length > 0)
	{
		Salt.SetNum(Length);
		int res = RAND_bytes(Salt.GetData(), Length);
		return res == 1;		
	}

	return (Length == 0);
}

TArray<uint8> FCryptoUtils::Int32ToDataArray(int32 integer)
{
	TArray<uint8> result;
	result.Add((integer & 0xff));
	result.Add((integer >> 8) & 0xff);
	result.Add((integer >> 16) & 0xff);
	result.Add((integer >> 24) & 0xff);
	return result;
}

TArray<uint8> FCryptoUtils::Int64ToDataArray(int64 integer)
{
	TArray<uint8> result;
	result.Add((integer & 0xff));
	result.Add((integer >> 8) & 0xff);
	result.Add((integer >> 16) & 0xff);
	result.Add((integer >> 24) & 0xff);
	result.Add((integer >> 32) & 0xff);
	result.Add((integer >> 40) & 0xff);
	result.Add((integer >> 48) & 0xff);
	result.Add((integer >> 56) & 0xff);
	return result;
}

TArray<uint8> FCryptoUtils::Int32ToDataArrayBE(int32 integer)
{
	TArray<uint8> result;
	result.Add((integer >> 24) & 0xff);
	result.Add((integer >> 16) & 0xff);
	result.Add((integer >> 8) & 0xff);
	result.Add((integer & 0xff));
	return result;
}

TArray<uint32> FCryptoUtils::SplitBytesByBits(const TArray<uint8>& Data, int BitIncrements)
{
	TArray<uint32> Result;
	const TBitArray<> BitArray = BytesToBitArray(Data);
	const int ResultNum = (Data.Num() * 8) / BitIncrements;
	for (size_t Index = 0; Index < ResultNum; Index++)
	{
		uint32 CurrentWord = 0;
		for (int32 BitIndex = 0; BitIndex < BitIncrements; ++BitIndex)
		{
			uint32 DataBit = 0;
			BitArray.GetRange(Index * BitIncrements + BitIndex, 1, &DataBit);
			const bool Bit = DataBit != 0;
			if (Bit)
			{
				CurrentWord = CurrentWord | (1 << ((BitIncrements - 1) - BitIndex));
			}
		}
		Result.Add(CurrentWord);
	}
	return Result;
}
 
TBitArray<> FCryptoUtils::BytesToBitArray(const TArray<uint8>& Data)
{
	TBitArray<> Result;
 
	for (int32 Index = 0; Index < Data.Num(); ++Index)
	{
		const uint8& Word = Data[Index];
		for (int32 BitIndex = 7; BitIndex >= 0; --BitIndex)
		{
			const bool Bit = Word & (1 << BitIndex);
			Result.Add(Bit);
		}
	}
 
	return Result;
}

TArray<uint8> FCryptoUtils::FStringToUint8(const FString& string)
{
	TArray<uint8> result;

	for( int i = 0; i < string.Len(); i++)
	{
		const uint8* character = (uint8*)TCHAR_TO_UTF8(*string.Mid(i,1));
		result.Add( *character );
	}
	
	return result;
}

TArray<uint8> FCryptoUtils::ShortVectorEncodeLength(int32 len)
{
	TArray<uint8> output;
	output.SetNum(10);

	int32 remLen = len;
	int32 cursor = 0;

	for (;;)
	{
		int32 elem = remLen & 0x7f;
		remLen >>= 7;
		if (remLen == 0)
		{
			output[cursor] = elem;
			break;
		}
		elem |= 0x80;
		output[cursor] = elem;
		cursor += 1;
	}

	TArray<uint8> bytes;
	for (int i = 0; i < cursor + 1; i++)
	{
		bytes.Add(output[i]);

	}
	return bytes;
}

std::vector<unsigned char> aes_128_gcm_encrypt(const unsigned char* data, uint32 lenght, std::string key)
{
	size_t enc_length = lenght*3;
	std::vector<unsigned char> output;
	output.resize(enc_length,'\0');

	unsigned char tag[AES_BLOCK_SIZE];
	unsigned char iv[AES_BLOCK_SIZE];
	RAND_bytes(iv, sizeof(iv));
	std::copy( iv, iv+16, output.begin()+16);

	std::vector<unsigned char> pass(key.begin(), key.end());
	pass.resize(16, '\0');
	
	int actual_size=0, final_size=0;
	EVP_CIPHER_CTX* e_ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_SET_IVLEN, 16, nullptr);
	EVP_EncryptInit(e_ctx, EVP_aes_128_gcm(), &pass[0], iv);
	EVP_EncryptUpdate(e_ctx, &output[32], &actual_size, data, lenght);
	EVP_EncryptFinal(e_ctx, &output[32+actual_size], &final_size);
	EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
	std::copy( tag, tag+16, output.begin() );
	std::copy( iv, iv+16, output.begin()+16);
	output.resize(32 + actual_size+final_size);
	EVP_CIPHER_CTX_free(e_ctx);
	return output;
}

std::vector<unsigned char> aes_128_gcm_decrypt(std::vector<unsigned char> ciphertext, std::string key)
{
	unsigned char tag[AES_BLOCK_SIZE];
	unsigned char iv[AES_BLOCK_SIZE];
	std::copy( ciphertext.begin(),    ciphertext.begin()+16, tag);
	std::copy( ciphertext.begin()+16, ciphertext.begin()+32, iv);
	std::vector<unsigned char> plaintext;
	plaintext.resize(ciphertext.size(), '\0');
	
	std::vector<unsigned char> pass(key.begin(), key.end());
	pass.resize(16, '\0');
	
	int actual_size=0, final_size=0;
	EVP_CIPHER_CTX *d_ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit(d_ctx, EVP_aes_128_gcm(), &pass[0], iv);
	EVP_DecryptUpdate(d_ctx, &plaintext[0], &actual_size, &ciphertext[32], ciphertext.size()-32 );
	EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
	EVP_DecryptFinal(d_ctx, &plaintext[actual_size], &final_size);
	EVP_CIPHER_CTX_free(d_ctx);
	plaintext.resize(actual_size + final_size, '\0');

	return plaintext;
}

TArray<uint8> FCryptoUtils::EncryptAES128GCM(const TArray<uint8>& Data, const FString& Password)
{
	const string key = std::string(TCHAR_TO_UTF8(*Password));
	const vector<unsigned char> ciphertext = aes_128_gcm_encrypt(Data.GetData(), Data.Num(), key);
	TArray<uint8> EncryptedData(ciphertext.data(), ciphertext.size());
	
	FString EncryptedBase16 = "";
	for (const uint8& Char : EncryptedData)
	{
		static const char* HexChars = "0123456789ABCDEF";
		EncryptedBase16.AppendChar(HexChars[Char / 16]);
		EncryptedBase16.AppendChar(HexChars[Char % 16]);
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *EncryptedBase16);
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, EncryptedBase16);

	return EncryptedData;
}

TArray<uint8> FCryptoUtils::DecryptAES128GCM(const TArray<uint8>& EncryptedData, const FString& Password)
{
	const string key = std::string(TCHAR_TO_UTF8(*Password));
	const vector<unsigned char> ciphertext(EncryptedData.GetData(), EncryptedData.GetData() + EncryptedData.Num());

	FString EncryptedBase16 = "";
	for (const uint8& Char : EncryptedData)
	{
		static const char* HexChars = "0123456789ABCDEF";
		EncryptedBase16.AppendChar(HexChars[Char / 16]);
		EncryptedBase16.AppendChar(HexChars[Char % 16]);
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *EncryptedBase16);
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, EncryptedBase16);

	vector<unsigned char> plain = aes_128_gcm_decrypt(ciphertext,key);

	TArray<uint8> DecryptedData(plain.data(), plain.size());
	return DecryptedData;
}
