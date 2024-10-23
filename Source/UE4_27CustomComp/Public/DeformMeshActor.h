// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeformMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DeformMeshActor.generated.h"

UCLASS()
class UE4_27CUSTOMCOMP_API ADeformMeshActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UDeformMeshComponent* DeformMeshComp;

	//We're creating a mesh section from this static mesh
	UPROPERTY(EditAnywhere)
	UStaticMesh* TestMesh;

	// We're using the transform of this actor as a deform transform
	UPROPERTY(EditAnywhere)
	AActor* Controller;

	
public:
	// Sets default values for this actor's properties
	ADeformMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
