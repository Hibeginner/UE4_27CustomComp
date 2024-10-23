// Fill out your copyright notice in the Description page of Project Settings.
#include "DeformMeshVertexFactory.h"

FDeformMeshVertexFactory::FDeformMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel): FLocalVertexFactory(
	InFeatureLevel, "FDeformMeshVertexFactory")
{
	bSupportsManualVertexFetch = false;
}

bool FDeformMeshVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	if ((Parameters.MaterialParameters.MaterialDomain == MD_Surface && Parameters.MaterialParameters.ShadingModels ==
		MSM_Unlit) || Parameters.MaterialParameters.bIsDefaultMaterial)
	{
		return true;
	}
	return false;
}

void FDeformMeshVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters,
                                                            FShaderCompilerEnvironment& OutEnvironment)
{
	const bool ContainsManualVertexFetch = OutEnvironment.GetDefinitions().Contains("MANUAL_VERTEX_FETCH");
	if (!ContainsManualVertexFetch)
	{
		OutEnvironment.SetDefine(TEXT("MANUAL_VERTEX_FETCH"), TEXT("0"));
	}

	OutEnvironment.SetDefine(TEXT("DEFORM_MESH"), TEXT("1"));
}

void FDeformMeshVertexFactory::InitRHI()
{
	check(HasValidFeatureLevel());

	FVertexDeclarationElementList Elements;
	FVertexDeclarationElementList PosOnlyElements;

	if (Data.PositionComponent.VertexBuffer != nullptr)
	{
		Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
		PosOnlyElements.Add(AccessStreamComponent(Data.PositionComponent, 0, EVertexInputStreamType::PositionOnly));
	}

	InitDeclaration(PosOnlyElements, EVertexInputStreamType::PositionOnly);

	if (Data.TextureCoordinates.Num())
	{
		const int32 BaseTexCoordAttribute = 4;
		for (int32 CoordinateIndex = 0; CoordinateIndex < Data.TextureCoordinates.Num(); CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(Data.TextureCoordinates[CoordinateIndex],
			                                   BaseTexCoordAttribute + CoordinateIndex));
		}

		for (int32 CoordinateIndex = Data.TextureCoordinates.Num(); CoordinateIndex < MAX_STATIC_TEXCOORDS / 2;
		     CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
				BaseTexCoordAttribute + CoordinateIndex
			));
		}
	}

	check(Streams.Num() > 0);

	InitDeclaration(Elements);
	check(IsValidRef(GetDeclaration()));
}

void FDeformMeshVertexFactory::SetTransformIndex(uint16 Index)
{
	TransformIndex = Index;
}

void FDeformMeshVertexFactory::SetSceneProxy(FDeformMeshSceneProxy * Proxy)
{
	SceneProxy = Proxy;
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FDeformMeshVertexFactory, "/CustomVertexFactory/LocalVertexFactory.ush", true, true, true, true, true);