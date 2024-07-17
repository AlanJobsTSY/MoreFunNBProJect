// Copyright Epic Games, Inc. All Rights Reserved.

#include "TSY_ProjectGameMode.h"
#include "TSY_ProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATSY_ProjectGameMode::ATSY_ProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
