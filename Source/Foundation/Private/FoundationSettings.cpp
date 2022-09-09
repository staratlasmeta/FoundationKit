
#include "FoundationSettings.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarSolanaNetworkOverride(
	TEXT("Solana.NetworkOverride"),
	0,
	TEXT("NetworkOverride:\n")
	TEXT("<=0: off (use the one selected in project settings)\n")
	TEXT("  1: MainNetBeta\n")
	TEXT("  2: DevNet"),
	ECVF_Cheat);
#endif

ESolanaNetwork UFoundationSettings::GetNetwork() const
{
#if !UE_BUILD_SHIPPING
	const ESolanaNetwork Override = static_cast<ESolanaNetwork>(CVarSolanaNetworkOverride.GetValueOnGameThread());
	if (Override != ESolanaNetwork::None)
	{
		return Override;
	}
#endif
	return Network;
}

FString UFoundationSettings::GetNetworkURL() const
{
	FString NetworkURL;
	if (const FString* NetworkURLPtr = NetworkURLs.Find(GetNetwork()))
	{
		NetworkURL = *NetworkURLPtr;
	}
	return NetworkURL;
}
