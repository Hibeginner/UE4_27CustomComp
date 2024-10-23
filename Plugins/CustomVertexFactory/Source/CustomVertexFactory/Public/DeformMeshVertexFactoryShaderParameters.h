// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class CUSTOMVERTEXFACTORY_API FDeformMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FDeformMeshVertexFactoryShaderParameters, NonVirtual);

private:
	LAYOUT_FIELD(FShaderParameter, TransformIndex);
	LAYOUT_FIELD(FShaderResourceParameter, TransformsSRV);

public:
	FDeformMeshVertexFactoryShaderParameters();
	~FDeformMeshVertexFactoryShaderParameters();

	void Bind(const FShaderParameterMap& ParameterMap);
	void GetElementShaderBindings(
		const class FSceneInterface* Scene,
		const class FSceneView* View,
		const class FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const class FVertexFactory* VertexFactory,
		const struct FMeshBatchElement& BatchElement,
		class FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const;
};
