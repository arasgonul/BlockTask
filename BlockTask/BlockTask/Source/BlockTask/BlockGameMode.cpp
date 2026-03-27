// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockGameMode.h"
#include "BlockCharacter.h"
ABlockGameMode::ABlockGameMode()
{
	DefaultPawnClass = ABlockCharacter::StaticClass();
}
