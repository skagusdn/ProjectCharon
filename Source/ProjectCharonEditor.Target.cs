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
		DefaultBuildSettings = BuildSettingsVersion.V6; // 5.7 버전 기준 V6로 업데이트
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		// ✨ 이 줄을 추가하세요!
		//BuildEnvironment = TargetBuildEnvironment.Unique;
		///////////

		ExtraModuleNames.AddRange( new string[] { "ProjectCharon" } );
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] {"ProjectCharonEditor"});
	}
}
