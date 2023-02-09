// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	// 定义 Widget 参数的类型和名称，在构造时传入
	SLATE_ARGUMENT(FString, TestString)
	
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
};
