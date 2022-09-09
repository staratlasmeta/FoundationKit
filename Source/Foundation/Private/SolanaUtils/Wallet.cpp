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
Contributers: Daniele Calanna, Federico Arona
*/

#include "SolanaUtils/Wallet.h"

#include "Network/RequestManager.h"
#include "JsonObjectConverter.h"
#include "Network/RequestUtils.h"
#include "Utils/TransactionUtils.h"
#include "SolanaUtils/Utils/Types.h"

UWallet::UWallet()
{
}

UWallet::~UWallet()
{
	TokenAccounts.Empty();
}

void UWallet::SetPublicKey( const FString& pubKey )
{
	PublicKey = pubKey;
	Account.PublicKey = pubKey;
}

bool UWallet::IsValidPublicKey( const FString& pubKey )
{
	return pubKey.Len() > 0;
}

bool UWallet::IsValidPublicKey( const TArray<FString>& pubKeys )
{
	for(FString key: pubKeys)
	{
		if( !IsValidPublicKey(key) )
			return false;
	}
	return true;
}

const FAccountData* UWallet::GetAccountByMint( const FString& mint)
{
	return TokenAccounts.FindByPredicate([mint](const FAccountData data){return data.Mint == mint;});
}

void UWallet::UpdateWalletData()
{
	if( IsValidPublicKey(PublicKey) )
	{
		FRequestData* request = FRequestUtils::RequestAccountInfo(PublicKey);
		request->Callback.BindLambda( [this](const FJsonObject& data)
		{
			const FAccountInfoJson response = FRequestUtils::ParseAccountInfoResponse(data);
			//AccountData->data = response.data.data;
			//AccountData->encoding = response.data.encoding;
			SOLBalance = response.lamports / 10;

			OnWalletUpdated.Broadcast(this);
		});
		FRequestManager::SendRequest(request);
	}
}

void UWallet::UpdateWalletBalance()
{
	if( IsValidPublicKey(PublicKey) )
	{
		FRequestData* request = FRequestUtils::RequestAccountBalance(PublicKey);
		request->Callback.BindLambda([this](const FJsonObject& data)
		{
			SOLBalance = FRequestUtils::ParseAccountBalanceResponse(data);

			OnWalletUpdated.Broadcast(this);
		});
		FRequestManager::SendRequest(request);
	}
}

void UWallet::UpdateAccountBalance(const FString& pubKey)
{
	if( IsValidPublicKey(pubKey) )
	{
		FRequestData* request = FRequestUtils::RequestAccountBalance(pubKey);
		request->Callback.BindLambda([this, pubKey](const FJsonObject& data)
		{
			double balance = FRequestUtils::ParseAccountBalanceResponse(data);
  
			FAccountData* accountData = TokenAccounts.FindByPredicate([this, pubKey](FAccountData account){return account.Pubkey == pubKey;});
			if( accountData )
			{
				accountData->Balance = balance;
			}
		});
		FRequestManager::SendRequest(request);
	}
}

void UWallet::UpdateAllTokenAccounts()
{
	if( IsValidPublicKey(PublicKey) )
	{
		FRequestData* request = FRequestUtils::RequestAllTokenAccounts(PublicKey, TokenProgramId);
		request->Callback.BindLambda([this](const FJsonObject& data)
		{
			TokenAccounts.Empty();
			if(TSharedPtr<FJsonObject> result = data.GetObjectField("result"))
			{
				FTokenAccountArrayJson jsonData;
				FJsonObjectConverter::JsonObjectToUStruct(result.ToSharedRef(), &jsonData);

				if(!jsonData.value.IsEmpty())
				{
					for( FTokenBalanceDataJson entry: jsonData.value)
					{
						FTokenInfoJson info = entry.account.data.parsed.info;

						FAccountData account;
						account.Pubkey = entry.pubkey;
						account.Balance = info.tokenAmount.uiAmount;
						account.Mint = info.mint;

						TokenAccounts.Add(account);
					}
					OnAccountsUpdated.Broadcast(this);
				}
			}
		});
		FRequestManager::SendRequest(request);
	}
}

void UWallet::CheckPossibleAccounts(const TArray<FString>& pubKeys)
{
	if( IsValidPublicKey(pubKeys) )
	{
		FRequestData* request = FRequestUtils::RequestMultipleAccounts(pubKeys);
		request->Callback.BindLambda([this](const FJsonObject& data)
		{
			const TArray<FAccountInfoJson> response = FRequestUtils::ParseMultipleAccountsResponse(data);
			//TODO: stuff?			
		});
		FRequestManager::SendRequest(request);
	}
}

void UWallet::SendSOL(const FAccount& from, const FAccount& to, int64 amount) const
{
	FRequestData* request = FRequestUtils::RequestBlockHash();
	request->Callback.BindLambda([this, from, to, amount](const FJsonObject& data)
	{
		FString blockHash = FRequestUtils::ParseBlockHashResponse(data);

		TArray<uint8> tranaction = FTransactionUtils::TransferSOLTransaction(from, to, amount, blockHash);
		
		FRequestData* transaction = FRequestUtils::SendTransaction( FBase64::Encode(tranaction));
		transaction->Callback.BindLambda([this, from, to, amount](const FJsonObject& data)
		{
			FString transactionID = FRequestUtils::ParseTransactionResponse(data);
			FRequestUtils::DisplayInfo(FString::Printf(TEXT("Transaction Id: %s"), *transactionID));
		});
		FRequestManager::SendRequest(transaction);
	});
	FRequestManager::SendRequest(request);
}

void UWallet::SendSOLEstimate(const FAccount& from, const FAccount& to, int64 amount) const
{
	FRequestData* request = FRequestUtils::RequestBlockHash();
	request->Callback.BindLambda([this, from, to, amount](const FJsonObject& data)
	{
		FString blockHash = FRequestUtils::ParseBlockHashResponse(data);

		TArray<uint8> transaction = FTransactionUtils::TransferSOLTransaction(from, to, amount, blockHash);
	
		FRequestData* feeRequest = FRequestUtils::GetTransactionFeeAmount( FBase64::Encode(transaction));
		feeRequest->Callback.BindLambda([this, from, to, amount](const FJsonObject& data)
		{
			int fee = FRequestUtils::ParseTransactionFeeAmountResponse(data);
			FRequestUtils::DisplayInfo(FString::Printf(TEXT("Estimate Id: %i"), fee));
		});
		FRequestManager::SendRequest(feeRequest);

	});
	FRequestManager::SendRequest(request);
}

void UWallet::SendTokenEstimate(const FAccount& from, const FAccount& to, const FString& mint, int64 amount) const
{
	FRequestData* accountRequest = FRequestUtils::RequestTokenAccount(to.PublicKey, mint);
	accountRequest->Callback.BindLambda([this, from, to, amount, mint](const FJsonObject& data)
	{
		FString existingAccount = FRequestUtils::ParseTokenAccountResponse(data);
		if(!existingAccount.IsEmpty())
		{
			FRequestData* blockhashRequest = FRequestUtils::RequestBlockHash();
			blockhashRequest->Callback.BindLambda([this, from, to, amount, mint, existingAccount](const FJsonObject& data)
			{
				FString blockHash = FRequestUtils::ParseBlockHashResponse(data);

				const TArray<uint8> transaction = FTransactionUtils::TransferTokenTransaction(from, to, Account, amount, mint, blockHash, existingAccount);

				FRequestData* sendTransaction = FRequestUtils::GetTransactionFeeAmount(FBase64::Encode(transaction));
				sendTransaction->Callback.BindLambda([this](FJsonObject& data)
				{
					int fee = FRequestUtils::ParseTransactionFeeAmountResponse(data);
					FRequestUtils::DisplayInfo(FString::Printf(TEXT("Estimate Id: %i"), fee));
				});
				FRequestManager::SendRequest(sendTransaction);
			});
			FRequestManager::SendRequest(blockhashRequest);
		}
	});
	FRequestManager::SendRequest(accountRequest);
}

void UWallet::SendToken(const FAccount& from, const FAccount& to, const FString& mint, int64 amount) const
{
	FRequestData* accountRequest = FRequestUtils::RequestTokenAccount(to.PublicKey, mint);
	accountRequest->Callback.BindLambda([this, from, to, amount, mint](const FJsonObject& data)
	{
		FString existingAccount = FRequestUtils::ParseTokenAccountResponse(data);
		if(!existingAccount.IsEmpty())
		{
			FRequestData* blockhashRequest = FRequestUtils::RequestBlockHash();
			blockhashRequest->Callback.BindLambda([this, from, to, amount, mint, existingAccount](FJsonObject& data)
			{
				FString blockHash = FRequestUtils::ParseBlockHashResponse(data);

				const TArray<uint8> transaction = FTransactionUtils::TransferTokenTransaction(from, to, Account, amount, mint, blockHash, existingAccount);

				FRequestData* sendTransaction = FRequestUtils::SendTransaction(FBase64::Encode(transaction));
				sendTransaction->Callback.BindLambda([this](FJsonObject& data)
				{
					const FString transactionID = FRequestUtils::ParseTransactionResponse(data);
					FRequestUtils::DisplayInfo(FString::Printf(TEXT("Transaction Id: %s"), *transactionID));
				});
				FRequestManager::SendRequest(sendTransaction);
			});
			FRequestManager::SendRequest(blockhashRequest);
		}
	});
	FRequestManager::SendRequest(accountRequest);
}
