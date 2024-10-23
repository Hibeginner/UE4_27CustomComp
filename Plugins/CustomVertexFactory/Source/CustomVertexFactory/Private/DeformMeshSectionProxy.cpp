// Fill out your copyright notice in the Description page of Project Settings.


#include "DeformMeshSectionProxy.h"

FDeformMeshSectionProxy::FDeformMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel): Material(nullptr),
	VertexFactory(InFeatureLevel), bSectionVisible(true)
{
}

FDeformMeshSectionProxy::~FDeformMeshSectionProxy()
{
}
