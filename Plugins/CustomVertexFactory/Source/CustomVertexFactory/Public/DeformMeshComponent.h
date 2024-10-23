// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeformMeshSection.h"
#include "UObject/Object.h"
#include "DeformMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMVERTEXFACTORY_API UDeformMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
private:
	/** Array of sections of mesh */
	UPROPERTY()
	TArray<FDeformMeshSection> DeformMeshSections;

	/** Local space bounds of mesh */
	UPROPERTY()
	FBoxSphereBounds LocalBounds;

private:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	void UpdateLocalBounds();
public:
	void CreateMeshSection(int32 SectionIndex, UStaticMesh* Mesh, const FTransform& Transform);
	void UpdateMeshSectionTransform(int32 SectionIndex, const FTransform& Transform);
	void FinishTransformsUpdate();
	void ClearMeshSection(int32 SectionIndex);
	void ClearAllMeshSections();
	void SetMeshSectionVisible(int32 SectionIndex, bool bNewVisibility);
	bool IsMeshSectionVisible(int32 SectionIndex) const;
	int32 GetNumSections() const;
	FDeformMeshSection* GetDeformMeshSection(int32 SectionIndex);
	void SetDeformMeshSection(int32 SectionIndex, const FDeformMeshSection& Section);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;


	friend class FDeformMeshSceneProxy;
};
