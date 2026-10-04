#pragma once



namespace Offsets
{
	// Constant Updates
	int Uworld = 0x19D0D570; // Updated from enum: GWorld = 0x1995B6F0
	int GNames = 0x1794C800;
	int Actors = 0xA0; // Updated from enum: Actors = 0xA0
	int Levels = 0x1E0; // Updated from enum: Levels = 0x1E0
	int GameState = 0x1C8; // Updated from enum: GameState = 0x1C8
	int OverlappingBuildings = 0x1D48;
	int GameInstance = 0x240; // Updated from enum: OwningGameInstance = 0x240
	int CameraLocation = 0x180;
	int CameraRotation = 0x190;
	int BoundScale = 0x310;
	int AmmoCount = 0x3f8;
	int MouseSensitivityX = 0x7c4;
	int MouseSensitivityY = 0x7c8;
	int ProjectileSpeed = 0x1d0;
	int ProjectileGravity = ProjectileSpeed + 0x4;
	int seconds = Offsets::CameraRotation + 0x10;
	int playeraimoffset = 0x26c0;
	int additionalaimoffset = 0x2a20;
	int bShouldDrawNativeReticle = 0x9c8;
	int ViewState = 0xD0; // Updated from enum: ViewState = 0xD0
	int weaponrecoiloffset = 0x2a20;
	int TotalPlayerDamageTaken = 0x1528;
	int TotalPlayerDamageDealt = 0x1524;
	//chams
	int CustomDepthComponent = 0x5018;

	// dont know
	int bShouldUsePerfectAimWhenTargetingMinSpread = 0x4d0;
	int LastRenderTime = 0x330; // Updated from enum: LastRenderTime = 0x330 (from u_skeletal_mesh_component)
	int lastrendertime_2 = 0x198;

	// Less Common
	int CurrentWeapon = 0x990; // From enum: CurrentWeapon = 0x990
	int WeaponData = 0x678;
	int WeaponList = 0x9c8;
	int WeaponCoreAnimation = 0x16D8; // Updated from enum: WeaponCoreAnimation = 0x16D8
	int TriggerType = 0x29c;
	int Rarity = 0xAA;
	int RankedProgress = 0xD8; // From enum: RankedProgress = 0xD8
	int KillScore = 0x11C8;
	int SeasonLevelUIDisplay = 0x11C4;
	int userplus = 0x8;
	int Username = 0xA08; // Updated from enum: PlayerNamePrivate = 0xA08
	int TeamIndex = 0x1211; // Updated from enum: TeamIndex = 0x1211
	int SquadId = 0x1344;
	int bIsDBNO = 0x841; // From enum: bIsDBNO = 0x841
	int bIsDying = 0x728; // From enum: bIsDying = 0x728
	int bAlreadySearched = 0xd22;
	int ComponentVelocity = 0x188;
	int RelativeRotation = 0x158; // From enum: RelativeRotation = 0x158
	int RelativeLocation = 0x140; // From enum: RelativeLocation = 0x140
	int PlayerArray = 0x2C8; // From enum: PlayerArray = 0x2C8
	int PawnPrivate = 0x328; // From enum: PawnPrivate = 0x328
	int Component_To_World = 0x1E0; // From enum: ComponentToWorld = 0x1E0
	int bIsReloadingWeapon = 0x3d1;
	int CurrentReloadDuration = 0x1438;
	int Bisabot = 0x2BA; // From enum: bIsABot = 0x2BA
	int Spectators = 0xaa0;
	int TargetedFortPawn = 0x1840; // Updated from enum: TargetedFortPawn = 0x1840
	int LastFiredDirection = 0x5bc8;
	int LastFiredLocation = 0x5bb0;
	int CurrentProjectedImpactDistance = 0x1380;
	int LastFireTimeVerified = 0x13a0;

	// Rare
	int FOV_Camera = 0x740; // From enum: FOV = 0x740
	int FOVAngle = 0x2D0;
	int LocalPlayers = 0x38; // From enum: LocalPlayers = 0x38
	int Projection = 0x940; // From enum: Projection = 0x940
	int AcknowledgedPawn = 0x358; // From enum: AcknowledgedPawn = 0x358
	int PlayerCameraManager = 0x368;
	int RotationInput = 0x530;
	int MyHud = 0x360;
	int RootComponent = 0x1B0; // From enum: RootComponent = 0x1B0
	int PlayerState = 0x2D0; // From enum: PlayerState = 0x2D0
	int Mesh = 0x330; // From enum: Mesh = 0x330
	int HabaneroComponent = 0x948; // From enum: HabaneroComponent = 0x948
	int Platform = 0x440; // From enum: Platform = 0x440
	int Bone_Array = 0x628; // From enum: ComponentSpaceTransformsArray = 0x628
	int Bone_Cache = Bone_Array + 0x10;
	int PlayerController = 0x30; // From enum: PlayerController = 0x30
	int PresistentLevel = 0x40;

	// Additional from enum not in original
	int DeltaTimeSeconds = 0x88C;
	int TimeSeconds = 0x868;
	int ServerWorldTimeSecondsDelta = 0x2E8;
	int bHidden = 0x58;
	int bDestroyed = 0x47E;
	int StaticMesh = 0x738;
	int bNoWeaponCollision = 0x749;
	int CachedWorldOrLocalSpaceBounds = 0x850;
	int BodySetup = 0x230;
	int AggGeom = 0x30;
	int ConvexElems = 0x30;
	int ViewMatrix = 0x8;
	int ReviveFromDBNOTime = 0x4F58;
}