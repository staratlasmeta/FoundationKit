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
#pragma once

struct FAccount;

class FTransactionUtils
{
public:
	
	static TArray<uint8> TransferTokenTransaction(const FAccount& from, const FAccount& to, const FAccount& owner, int64 amount, const FString& mint, const FString& blockHash, const FString& existingAccount);
	static TArray<uint8> TransferSOLTransaction(const FAccount& from, const FAccount& to, int64 amount, const FString& blockHash);
};
