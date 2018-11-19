#pragma once

#include <cstdint>
#include <vector>

struct ItemType
{
	enum Enum
	{
		Vibroblade,
		FinesseVibroblade,
		Baton,
		HeavyVibroblade,
		Saberstaff,
		Polearm,
		TwinBlade,
		MartialArtWeapon,
		BlasterPistol,
		BlasterRifle,
		Lightsaber,

		EnumCount,
		Invalid = EnumCount
	};
};

struct CharBackground
{
	enum Enum
	{
		Freelancer,
		Smuggler,
		Sharpshooter,
		TerasKasi,
		SecurityOfficer,
		Berserker,
		Duelist,
		Soldier,
		Armorsmith,
		Weaponsmith,
		Chef,
		Engineer,
		Fabricator,
		Harvester,
		Scavenger,
		Medic,

		EnumCount,
		Invalid = EnumCount
	};
};

struct CharSkill
{
	enum Enum
	{
		Melee_OneHanded,
		Melee_TwoHanded,
		Melee_TwinBlades,
		Melee_MartialArts,
		Melee_Lightsabers,

		Ranged_Blasters,
		Ranged_Throwing,

		Defense_LightArmor,
		Defense_HeavyArmor,
		Defense_Shields,
		Defense_ForceArmor,

		Crafting_Weaponsmith,
		Crafting_Armorsmith,
		Crafting_Fabrication,
		Crafting_Cooking,
		Crafting_Engineering,

		Gathering_Harvesting,
		Gathering_Scavenging,

		Utility_Medicine,
		Utility_ItemRepair,
		Utility_Farming,

		Force_DarkSide,
		Force_LightSide,

		EnumCount,
		Invalid = EnumCount
	};
};

struct CharPerk
{
	enum Enum
	{
		BlasterPistol_Proficiency,
		BlasterPistol_ImprovedCrit,
		BlasterPistol_PlasmaCell,
		BlasterPistol_RapidShot,
		BlasterPistol_WeaponFocus,

		BlasterRifle_Proficiency,
		BlasterRifle_ImprovedCrit,
		BlasterRifle_PlasmaCell,
		BlasterRifle_RapidReload,
		BlasterRifle_WeaponFocus,

		Firearms_PointBlankShot,
		Firearms_ZenMarksmanship,

		MartialArts_ImprovedCrit,
		MartialArts_Proficiency,
		MartialArts_WeaponFocus,

		Baton_Proficiency,
		Baton_ImprovedCrit,
		Baton_WeaponFocus,

		FinesseVibroblade_Proficiency,
		FinesseVibroblade_ImprovedCrit,
		FinesseVibroblade_WeaponFinesse,
		FinesseVibroblade_WeaponFocus,

		OneHanded_DualWielding,

		Lightsaber_Proficiency,
		Lightsaber_ImprovedCrit,
		Lightsaber_WeaponFocus,

		Vibroblade_ImprovedCrit,
		Vibroblade_Proficiency,
		Vibroblade_WeaponFocus,

		Stance_ShieldOath,
		Stance_SwordOath,

		SaberStaff_ImprovedCrit,
		SaberStaff_Proficiency,
		SaberStaff_WeaponFocus,

		TwinVibroblade_ImprovedCrit,
		TwinVibroblade_Mastery,
		TwinVibroblade_Proficiency,
		TwinVibroblade_WeaponFocus,

		HeavyVibroblade_Proficiency,
		HeavyVibroblade_ImprovedCrit,
		HeavyVibroblade_WeaponFocus,

		Polearm_Proficiency,
		Polearm_ImprovedCrit,
		Polearm_WeaponFocus,

		EnumCount,
		Invalid = EnumCount
	};
};

struct LevelledSkill
{
	CharSkill::Enum skill;
	std::uint8_t level;
};

struct LevelledPerk
{
	CharPerk::Enum perk;
	std::uint8_t level;
};

struct CharLayout
{
	ItemType::Enum itemType;
	CharBackground::Enum background;
	std::vector<LevelledSkill> skills;
	std::vector<LevelledPerk> perks;
};

struct StatLayout
{
	std::uint8_t str;
	std::uint8_t dex;
	std::uint8_t con;
	std::uint8_t intelligence;
	std::uint8_t wis;
	std::uint8_t cha;

	std::uint8_t bab;
};

struct SimulationResult
{
	CharLayout charLayout;
	StatLayout statLayout;

	std::uint8_t ac;
	std::uint8_t ab;

	// Index refers to opponent AC
	float dmgLow[100];
	float dmgAvg[100];
	float dmgHigh[100];
};

constexpr int BONUS_NONE = 0;
constexpr int BONUS_STR = 1;
constexpr int BONUS_DEX = 2;
constexpr int BONUS_CON = 3;
constexpr int BONUS_INT = 4;
constexpr int BONUS_WIS = 5;
constexpr int BONUS_CHA = 6;

StatLayout BuildStats(const CharLayout& charLayout);
SimulationResult DoSimulation(CharLayout charLayout, StatLayout statLayout);

std::vector<CharPerk::Enum> GetPerksForItemType(ItemType::Enum itemType);
std::uint8_t GetSPCostForLevel(CharPerk::Enum perk, std::uint8_t level);
std::uint8_t GetMaxPerkLevel(CharPerk::Enum perk);
std::uint8_t GetMaxSkillLevel(CharSkill::Enum skill);
CharSkill::Enum GetGoverningSkill(ItemType::Enum type);
CharPerk::Enum GetWeaponSpecPerk(ItemType::Enum type);
CharPerk::Enum GetWeaponProficiencyPerk(ItemType::Enum type);
CharPerk::Enum GetWeaponCritPerk(ItemType::Enum type);
CharBackground::Enum GetBackgroundForBABBonus(ItemType::Enum type);
std::string ToString(ItemType::Enum type);
bool IsRanged(ItemType::Enum type);
int GetPerkLevel(const std::vector<LevelledPerk>* perks, CharPerk::Enum perk);

int GetPrimary(CharSkill::Enum skill);
int GetSecondary(CharSkill::Enum skill);
int GetTertiary(CharSkill::Enum skill);
