// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeformMeshVertexFactory.h"

/**
 * 
 */
class CUSTOMVERTEXFACTORY_API FDeformMeshSectionProxy
{
public:
	UMaterialInterface* Material;
	FRawStaticIndexBuffer IndexBuffer;
	FDeformMeshVertexFactory VertexFactory;
	bool bSectionVisible;
	uint32 MaxVertexIndex;
public:
	FDeformMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel);
	~FDeformMeshSectionProxy();
};
