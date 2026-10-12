#include "Player/CXPlayerController.h"

#include"UI/CXChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ChatX/ChatX.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/CXGameModeBase.h"
#include "Player/CXPlayerState.h"
#include "Net/UnrealNetwork.h"

ACXPlayerController::ACXPlayerController()
{
  bReplicates = true;
}

void ACXPlayerController::BeginPlay()
{
  Super::BeginPlay();

  if (IsLocalController() == false)
  {
    return;
  }

  FInputModeUIOnly InputModeUIOnly;
  SetInputMode(InputModeUIOnly);

  if (IsValid(ChatInputWidgetClass) == true)
  {
    ChatInputWidgetInstance = CreateWidget<UCXChatInput>(this, ChatInputWidgetClass);
    if (IsValid(ChatInputWidgetInstance) == true)
    {
      ChatInputWidgetInstance->AddToViewport();
    }
  }

  if (IsValid(NotificationTextWidgetClass) == true)
  {
    NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
    if (IsValid(NotificationTextWidgetInstance) == true)
    {
      NotificationTextWidgetInstance->AddToViewport();
    }
  }
}

void ACXPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
  ChatMessageString = InChatMessageString;

  if (IsLocalController() == true)
  {
    ACXPlayerState* CXPS = GetPlayerState<ACXPlayerState>();
    if (IsValid(CXPS) == true)
    {
      ServerRPCPrintChatMessageString(InChatMessageString);
    }
  }
}

void ACXPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
  FString NetModeString = ChatXFunctionLibrary::GetNetModeString(this);
  FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
  ChatXFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);
}

void ACXPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
  PrintChatMessageString(InChatMessageString);
}

void ACXPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
  AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
  if (IsValid(GM) == true)
  {
    ACXGameModeBase* CXGM = Cast<ACXGameModeBase>(GM);
    if (IsValid(CXGM) == true)
    {
      CXGM->PrintChatMessageString(this, InChatMessageString);
    }
  }
}

void ACXPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ThisClass, NotificationText);
}