// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	// InArgs._TestString;
	// 接收参数
	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString(InArgs._TestString))
	];
}
