// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeformMeshComponent.h"
#include "DeformMeshSectionProxy.h"

/**
 * 
 */
class CUSTOMVERTEXFACTORY_API FDeformMeshSceneProxy : public FPrimitiveSceneProxy
{
private:
	TArray<FDeformMeshSectionProxy*> Sections;
	FMaterialRelevance MaterialRelevance;
	TArray<FMatrix> DeformTransforms;
	FStructuredBufferRHIRef DeformTransformsSB;
	FShaderResourceViewRHIRef DeformTransformsSRV;
	bool bDeformTransformsDirty;

public:
	FDeformMeshSceneProxy(UDeformMeshComponent* Component);
	virtual ~FDeformMeshSceneProxy() override;

	/* Update the transforms structured buffer using the array of deform transform, this will update the array on the GPU*/
	void UpdateDeformTransformsSB_RenderThread();

	/* Update the deform transform that is being used to deform this mesh section, this will just update this section's entry in the CPU array*/
	void UpdateDeformTransform_RenderThread(int32 SectionIndex, FMatrix Transform);

	/* Update the mesh section's visibility*/
	void SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility);

	/* Given the scene views and the visibility map, we add to the collector the relevant dynamic meshes that need to be rendered by this component*/
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
	                                    uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual bool CanBeOccluded() const override;

	virtual uint32 GetMemoryFootprint(void) const override;

	uint32 GetAllocatedSize(void) const;

	//Getter to the SRV of the transforms structured buffer
	FShaderResourceViewRHIRef& GetDeformTransformsSRV();
	virtual SIZE_T GetTypeHash() const override;
};
