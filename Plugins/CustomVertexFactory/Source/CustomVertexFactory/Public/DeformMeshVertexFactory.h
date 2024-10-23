// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LocalVertexFactory.h"
#include "MeshMaterialShader.h"
// #include "DeformMeshVertexFactory.generated.h"

class FDeformMeshSceneProxy;
/**
 * 
 */
class CUSTOMVERTEXFACTORY_API FDeformMeshVertexFactory : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FDeformMeshVertexFactory)
private:
	uint16 TransformIndex;
	FDeformMeshSceneProxy * SceneProxy;
	
public:
	FDeformMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel);

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	virtual void InitRHI() override;

	void SetTransformIndex(uint16 Index);
	void SetSceneProxy(FDeformMeshSceneProxy * Proxy);

	
	friend class FDeformMeshVertexFactoryShaderParameters;
};

