// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeformMeshSection.generated.h"

/**
 * 
 */
USTRUCT()
struct CUSTOMVERTEXFACTORY_API FDeformMeshSection
{
	GENERATED_BODY()
public:
	/** The static mesh that holds the mesh data for this section */
	UPROPERTY()
	UStaticMesh* StaticMesh;

	/** The secondary transform matrix that we'll use to deform this mesh section*/
	UPROPERTY()
	FMatrix DeformTransform;

	/** Local bounding box of section */
	UPROPERTY()
	FBox SectionLocalBox;

	/** Should we display this section */
	UPROPERTY()
	bool bSectionVisible;

	FDeformMeshSection()
		: SectionLocalBox(ForceInit)
		, bSectionVisible(true)
	{}

	/** Reset this section, clear all mesh info. */
	void Reset()
	{
		StaticMesh = nullptr;
		SectionLocalBox.Init();
		bSectionVisible = true;
	}
};
