// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/S_Actor.h"

// Sets default values
AS_Actor::AS_Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AS_Actor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AS_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

