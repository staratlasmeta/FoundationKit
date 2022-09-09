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

#include "FEd25519Bip39.h"

#include "CryptoUtils.h"

const uint32 HardenedOffset = 0x80000000;
const FString Curve = "ed25519 seed";

FEd25519Bip39::FEd25519Bip39(const TArray<uint8>& seed)
{
    TArray<uint8> hmac = FCryptoUtils::HMAC_SHA512(seed, Curve);
    for(int i = 0; i < 64; i++)
    {
        if( i < 32 )
        {
            KeyPair.MasterKey.Add(hmac[i]);
        }
        else
        {
            KeyPair.ChainCode.Add(hmac[i]);
        }
    }
}

TArray<uint8> FEd25519Bip39::DeriveAccountPath(uint32 index)
{
    //Bip39 Derivation Path = "m/44'/501'/index'/0'"
    TArray<uint32> segments = { 44, 501, index, 0 };

    Bip39KeyPair result = KeyPair;
    for(int i = 0; i < segments.Num(); i++)
    {
        result = GetChildKeyDerivation(result.MasterKey, result.ChainCode, segments[i] + HardenedOffset);
    }

    return result.MasterKey;
}

TArray<uint8> FEd25519Bip39::DeriveAccountPath(const TArray<uint32>& Segments)
{
    Bip39KeyPair Result = KeyPair;
    for(int i = 0; i < Segments.Num(); i++)
    {
        Result = GetChildKeyDerivation(Result.MasterKey, Result.ChainCode, Segments[i] + HardenedOffset);
    }

    return Result.MasterKey;
}

Bip39KeyPair FEd25519Bip39::GetChildKeyDerivation(const TArray<uint8>& key, const TArray<uint8>& chainCode, uint32 index)
{
    TArray<uint8> buffer;
    buffer.Add(0);
    buffer.Append(key);
    buffer.Append(FCryptoUtils::Int32ToDataArrayBE(index));

    TArray<uint8> hmac = FCryptoUtils::HMAC_SHA512( buffer, chainCode );

    Bip39KeyPair keypair;
    for(int i = 0; i < 64; i++)
    {
        if( i < 32 )
        {
            keypair.MasterKey.Add(hmac[i]);
        }
        else
        {
            keypair.ChainCode.Add(hmac[i]);
        }
    }

    return keypair;
}
