// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include "Components/SphereComponent.h"
#include "MotionControllerComponent.h"
#include "VRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	SetRootComponent(MotionController);
	MotionController->bDisplayDeviceModel = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereCollision->SetupAttachment(GetRootComponent());
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

	HapticEffect = CreateDefaultSubobject<UHapticFeedbackEffect_Base>("HapticEffect");
}

// Called when the game starts or when spawned
void AHandController::BeginPlay()
{
	Super::BeginPlay();

	// Binding overlap events to function delegates
	OnActorBeginOverlap.AddDynamic(this, &AHandController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AHandController::ActorEndOverlap);
}

void AHandController::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	bool bNewCanClimb = CanClimb();
	if (!bCanClimb && bNewCanClimb)
	{
		APlayerController* Controller = Cast<APlayerController>(UGameplayStatics::GetPlayerController(this, 0));
		if (Controller == nullptr) return;
		Controller->PlayHapticEffect(HapticEffect, MotionController->GetTrackingSource());
		
		
		UE_LOG(LogTemp, Warning, TEXT("Can Climb"));	
	}
	bCanClimb = bNewCanClimb;
}

void AHandController::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	bCanClimb = CanClimb();
}

bool AHandController::CanClimb() const
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverlappingActor is %s"), *OverlappingActor->GetName());
		if (OverlappingActor->ActorHasTag("Climbable"))
		{
			UE_LOG(LogTemp, Warning, TEXT("OverlappingActor %s has tag Climbable"), *OverlappingActor->GetName());
			return true;
		}
	}
	return false;
}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsClimbing)
	{
		FVector HandControllerDelta = GetActorLocation() - ClimbingStartLocation;
		
		GetOwner()->AddActorWorldOffset(-HandControllerDelta);
	}
}

void AHandController::PairController(AHandController* Controller)
{
	OtherController = Controller;
	OtherController->OtherController = this; //Other controller's other OtherController variable is US. Therefore I use this keyword.
}

void AHandController::Grip()
{
	if(!bCanClimb) return;

	if (!bIsClimbing)
	{
		bIsClimbing = true;
		ClimbingStartLocation = GetActorLocation();

		OtherController->bIsClimbing = false;

		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if(Character != nullptr)
		{
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		}
	}
}

void AHandController::Release()
{
	if(bIsClimbing)
	{
		bIsClimbing = false;

		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if(Character != nullptr)
		{
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}

