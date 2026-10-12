#include "Game/CXGameModeBase.h"
#include "Game/CXGameStateBase.h"
#include "Player/CXPlayerController.h"
#include "EngineUtils.h"
#include "Player/CXPlayerState.h"

void ACXGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer);
	if (IsValid(CXPlayerController) == true)
	{
		CXPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
	}
	if (IsValid(CXPlayerController) == true)
	{
		AllPlayerControllers.Add(CXPlayerController);

		ACXPlayerState* CXPS = CXPlayerController->GetPlayerState<ACXPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ACXGameStateBase* CXGameStateBase = GetGameState<ACXGameStateBase>();
		if (IsValid(CXGameStateBase) == true)
		{
			CXGameStateBase->MulticastRPCBroadcastLoginMessage(CXPS->PlayerNameString);
		}
	}
}

FString ACXGameModeBase::GenerateSecretNumber()
{
	TArray<int32>Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}
	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ACXGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	if (InNumberString.Len() != 3)
	{
		return false;
	}
	TSet<TCHAR> UniqueDigits;
	for (TCHAR Digit : InNumberString)
	{
		if (Digit < '1' || Digit > '9' || UniqueDigits.Contains(Digit))
		{
			return false;
		}
		UniqueDigits.Add(Digit);
	}
	return true;
}

FString ACXGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ACXGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *SecretNumberString);
}

void ACXGameModeBase::PrintChatMessageString(ACXPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	if (!HasAuthority() || !IsValid(InChattingPlayerController))
	{
		return;
	}
	ACXPlayerState* GuessingPlayerState = InChattingPlayerController->GetPlayerState<ACXPlayerState>();
	if (!IsValid(GuessingPlayerState))
	{
		return;
	}
	const FString GuessNumberString = InChatMessageString.TrimStartAndEnd();
	bool bIsNumericInput = !GuessNumberString.IsEmpty();
	for (TCHAR Character : GuessNumberString)
	{
		if (Character < '0' || Character > '9')
		{
			bIsNumericInput = false;
			break;
		}
	}
	if (bIsNumericInput)
	{
		FString ValidationMessage;
		if (GuessingPlayerState->CurrentGuessCount >= GuessingPlayerState->MaxGuessCount)
		{
			ValidationMessage = TEXT("No guesses remaining. Please wait for the next round.");
		}
		else if (GuessNumberString.Len() != 3)
		{
			ValidationMessage = TEXT("Please enter a 3-digit number.");
		}
		else if (GuessNumberString.Contains(TEXT("0")))
		{
			ValidationMessage = TEXT("Please use digits from 1 to 9.");
		}
		else if (!IsGuessNumberString(GuessNumberString))
		{
			ValidationMessage = TEXT("Please enter a number with no repeated digits.");
		}
		if (!ValidationMessage.IsEmpty())
		{
			InChattingPlayerController->NotificationText = FText::FromString(ValidationMessage);
			InChattingPlayerController->ClientRPCPrintChatMessageString(ValidationMessage);
			return;
		}
	}
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);

		IncreaseGuessCount(InChattingPlayerController);

		//채팅에 횟수 반영을 위한 추가
		ACXPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ACXPlayerState>();

		if (IsValid(CXPS) == false)
		{
			return;
		}
		
		FString UpdatedMessageString =
			CXPS->GetPlayerInfoString() + TEXT(": ") + GuessNumberString;

		for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
		{
			ACXPlayerController* CXPlayerController = *It;
			if (IsValid(CXPlayerController) == true)
			{
				//채팅에 횟수 반영을 위한 추가 UpdatedMessageString
				FString CombinedMessageString = UpdatedMessageString + TEXT(" -> ") + JudgeResultString;
				CXPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);


			}
		}
		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
	}
	else
	{
		for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
		{
			ACXPlayerController* CXPlayerController = *It;
			if (IsValid(CXPlayerController) == true)
			{
				CXPlayerController->ClientRPCPrintChatMessageString(GuessingPlayerState->GetPlayerInfoString() + TEXT(": ") + InChatMessageString);
			}
		}
	}
}

void ACXGameModeBase::IncreaseGuessCount(ACXPlayerController* InChattingPlayerController)
{
	ACXPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ACXPlayerState>();
	
	if (IsValid(CXPS) && CXPS->CurrentGuessCount < CXPS->MaxGuessCount)
	{
		CXPS->CurrentGuessCount++;
	}
}

void ACXGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (const auto& CXPlayerController : AllPlayerControllers)
	{
		ACXPlayerState* CXPS = CXPlayerController->GetPlayerState<ACXPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->CurrentGuessCount = 0;
		}
	}
}

void ACXGameModeBase::JudgeGame(ACXPlayerController* InChattingPlayerController, int InStrikeCount)
{
	FString ResultMessage;
	if (InStrikeCount == 3)
	{
		ACXPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ACXPlayerState>();
		if (IsValid(CXPS) == false)
		{
			return;
		}
		ResultMessage = CXPS->PlayerNameString + TEXT(" has won the game.");
	}
	else
	{
		for (const auto& CXPlayerController : AllPlayerControllers)
		{
			if (IsValid(CXPlayerController) == false)
			{
				continue;
			}
			ACXPlayerState* CXPS = CXPlayerController->GetPlayerState<ACXPlayerState>();
			if (IsValid(CXPS) == true && CXPS->CurrentGuessCount < CXPS->MaxGuessCount)
			{
				return;
			}
		}
		ResultMessage = TEXT("Draw...");
	}

	for (const auto& CXPlayerController : AllPlayerControllers)
	{
		if (IsValid(CXPlayerController) == true)
		{
			CXPlayerController->NotificationText = FText::FromString(ResultMessage);
		}
	}

	// Reset once, after notifying all players.
	ResetGame();
}
