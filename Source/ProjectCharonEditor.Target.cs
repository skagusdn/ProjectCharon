// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectCharonEditorTarget : TargetRules
{
	public ProjectCharonEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		//DefaultBuildSettings = BuildSettingsVersion.V5;
		
		/////////
		//DefaultBuildSettings = BuildSettingsVersion.V6; // 5.7 버전 기준 V6로 업데이트
		DefaultBuildSettings = BuildSettingsVersion.V7; // 5.8 버전 기준 V7로 업데이트
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		

		ExtraModuleNames.AddRange( new string[] { "ProjectCharon" } );
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] {"ProjectCharonEditor"});
	}
}
