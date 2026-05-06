#include "SheetSync.h"

#include "SheetSyncSettings.h"
#include "SheetSyncSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "FSheetSyncModule"

void FSheetSyncModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule =
	FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		USheetSyncSettings::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FSheetSyncSettingsCustomization::MakeInstance));
}

void FSheetSyncModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule =
			FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(
			USheetSyncSettings::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSheetSyncModule, SheetSync)