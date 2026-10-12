#include "Player/CXPawn.h"
#include "ChatX/ChatX.h"

ACXPawn::ACXPawn()
{

	PrimaryActorTick.bCanEverTick = false;

}

void ACXPawn::BeginPlay()
{
	Super::BeginPlay();

	FString NetRoleString = ChatXFunctionLibrary::GetNetModeString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::BeginPlay() %s [%s]"), *ChatXFunctionLibrary::GetNetModeString(this), *NetRoleString);
	ChatXFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
	
}

void ACXPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	FString NetRoleString = ChatXFunctionLibrary::GetNetModeString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::PossessedBy() %s [%s]"), *ChatXFunctionLibrary::GetNetModeString(this), *NetRoleString);
	ChatXFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}


