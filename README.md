# FoundationKit

## What is it

The **Foundation Kit** is an **Unreal Engine Plugin** created for `Unreal Engine 4` and `Unreal Engine 5`
allowing `Unreal Game Clients` to connect and interact with the **Solana Blockchain**.

It consists of 2 layers, the `Core SDK` and the `Wallet and Blueprint interface` that uses the SDK.

The `Core SDK` contains all of the tools necessary to connect to the Solana Blockchain. From generating mnemonics and key pairs, importing private keys, checking account data, sending and receiving transactions, to interacting with on-chain programs. 

The second layer is the `Wallet and  Blueprint interface`. The wallet manages multiple accounts, encrypting and saving locally your key pairs. All that is needed is for the user to develop their own custom UI and hook it up. Either via `C++` or by using the `Blueprint interface`.

## Documentation
You can find the documentation on how to use the FoundationKit [here](https://github.com/staratlasmeta/FoundationKit/tree/main/Documentation/).
