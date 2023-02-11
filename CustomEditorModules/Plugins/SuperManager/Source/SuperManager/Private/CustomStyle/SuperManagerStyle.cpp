// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FSuperManagerStyle::StyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

void FSuperManagerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
}

void FSuperManagerStyle::Shutdown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
	}
}

TSharedRef<FSlateStyleSet> FSuperManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomSlateStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));
	
	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("SuperManager"))->GetBaseDir() /"Resources";
	const FVector2d Icon16x16 (16.f, 16.f);
	
	CustomSlateStyleSet->SetContentRoot(IconDirectory);
	
	CustomSlateStyleSet->Set("ContentBrowser.DeleteUnusedAssets",
		new FSlateImageBrush(IconDirectory/"DeleteUnusedAsset.png", Icon16x16));
	
	CustomSlateStyleSet->Set("ContentBrowser.DeleteEmptyFolders",
		new FSlateImageBrush(IconDirectory/"DeleteEmptyFolders.png", Icon16x16));
	
	CustomSlateStyleSet->Set("ContentBrowser.AdvanceDeletion",
		new FSlateImageBrush(IconDirectory/"AdvanceDeletion.png", Icon16x16));
	
	return CustomSlateStyleSet;
}
