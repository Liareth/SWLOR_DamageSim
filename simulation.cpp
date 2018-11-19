#include "simulation.h"

#include <algorithm>
#include <cassert>
#include <numeric>

inline int fast_rand()
{
	static int s_Seed = 1052;
	s_Seed = (214013 * s_Seed + 2531011);
	return (s_Seed >> 16) & 0x7FFF;
}

int Roll(int dice, int sides)
{
	int value = 0;

	while (dice-- > 0)
	{
		value += fast_rand() % sides + 1;
	}

	return value;
}


StatLayout BuildStats(const CharLayout& charLayout)
{
    StatLayout statLayout;

    statLayout.str = 10;
    statLayout.dex = 10;
    statLayout.con = 10;
    statLayout.intelligence = 10;
    statLayout.wis = 10;
    statLayout.cha = 10;
    statLayout.bab = 1;

    int strBonus = 0;
    int dexBonus = 0;
    int conBonus = 0;
    int intBonus = 0;
    int wisBonus = 0;
    int chaBonus = 0;
    int babBonus = 0;

    for (const LevelledSkill& skill : charLayout.skills)
    {
        constexpr float primaryMultiplier = 0.2f;
        constexpr float secondaryMultiplier = 0.1f;
        constexpr float tertiaryMultiplier = 0.05f;
        constexpr int maxAttributeBonus = 70;

        // Primary
        switch (GetPrimary(skill.skill))
        {
            case BONUS_STR: strBonus += static_cast<int>(skill.level * primaryMultiplier); break;
            case BONUS_DEX: dexBonus += static_cast<int>(skill.level * primaryMultiplier); break;
            case BONUS_CON: conBonus += static_cast<int>(skill.level * primaryMultiplier); break;
            case BONUS_INT: intBonus += static_cast<int>(skill.level * primaryMultiplier); break;
            case BONUS_WIS: wisBonus += static_cast<int>(skill.level * primaryMultiplier); break; 
            case BONUS_CHA: chaBonus += static_cast<int>(skill.level * primaryMultiplier); break;
        }

        // Secondary
        switch (GetSecondary(skill.skill))
        {
            case BONUS_STR: strBonus += static_cast<int>(skill.level * secondaryMultiplier); break;
            case BONUS_DEX: dexBonus += static_cast<int>(skill.level * secondaryMultiplier); break;
            case BONUS_CON: conBonus += static_cast<int>(skill.level * secondaryMultiplier); break;
            case BONUS_INT: intBonus += static_cast<int>(skill.level * secondaryMultiplier); break;
            case BONUS_WIS: wisBonus += static_cast<int>(skill.level * secondaryMultiplier); break; 
            case BONUS_CHA: chaBonus += static_cast<int>(skill.level * secondaryMultiplier); break;
        }

        // Tertiary
        switch (GetTertiary(skill.skill))
        {
            case BONUS_STR: strBonus += static_cast<int>(skill.level * tertiaryMultiplier); break;
            case BONUS_DEX: dexBonus += static_cast<int>(skill.level * tertiaryMultiplier); break;
            case BONUS_CON: conBonus += static_cast<int>(skill.level * tertiaryMultiplier); break;
            case BONUS_INT: intBonus += static_cast<int>(skill.level * tertiaryMultiplier); break;
            case BONUS_WIS: wisBonus += static_cast<int>(skill.level * tertiaryMultiplier); break; 
            case BONUS_CHA: chaBonus += static_cast<int>(skill.level * tertiaryMultiplier); break;
        }

        // BAB
        if (skill.skill == GetGoverningSkill(charLayout.itemType))
        {
            babBonus += skill.level / 10;
        }
    }

    constexpr int maxAttributeBonus = 70;

    statLayout.str += std::min(maxAttributeBonus, strBonus);
    statLayout.dex += std::min(maxAttributeBonus, dexBonus);
    statLayout.con += std::min(maxAttributeBonus, conBonus);
    statLayout.intelligence += std::min(maxAttributeBonus, intBonus);
    statLayout.wis += std::min(maxAttributeBonus, wisBonus);
    statLayout.cha += std::min(maxAttributeBonus, chaBonus);

	if (charLayout.background == GetBackgroundForBABBonus(charLayout.itemType))
	{
		babBonus += 2;
	}

	int proficiencyBonus = GetPerkLevel(&charLayout.perks, GetWeaponProficiencyPerk(charLayout.itemType));

	babBonus += proficiencyBonus;

	// PROPOSED CHANGES
	if (charLayout.itemType == ItemType::BlasterRifle || charLayout.itemType == ItemType::BlasterPistol)
	{
		babBonus += proficiencyBonus / 2;
	}

    statLayout.bab += babBonus;

    return statLayout;
}

SimulationResult DoSimulation(CharLayout charLayout, StatLayout statLayout)
{
    SimulationResult result;

	int mainHandAb = statLayout.bab;

    if (IsRanged(charLayout.itemType))
    {
        if (GetPerkLevel(&charLayout.perks, CharPerk::Firearms_ZenMarksmanship))
        {
			mainHandAb += (statLayout.wis - 10) / 2;
        }
        else
        {
			mainHandAb += (statLayout.dex - 10) / 2;
        }
    }
    else
    {
        if (charLayout.itemType == ItemType::FinesseVibroblade && GetPerkLevel(&charLayout.perks, CharPerk::FinesseVibroblade_WeaponFinesse))
        {
			mainHandAb += (statLayout.dex - 10) / 2;
        }
        else
        {
			mainHandAb += (statLayout.str - 10) / 2;
        }
    }

	int dualWieldingLevel = 0;
	if (charLayout.itemType == ItemType::TwinBlade)
	{
		dualWieldingLevel = GetPerkLevel(&charLayout.perks, CharPerk::TwinVibroblade_Mastery);
	}
	else
	{
		dualWieldingLevel = GetPerkLevel(&charLayout.perks, CharPerk::OneHanded_DualWielding);
	}

    int regularApr = statLayout.bab > 15 ? 4 : statLayout.bab > 10 ? 3 : statLayout.bab > 5 ? 2 : 1;
    int specialApr = 0;
	int offhandApr = dualWieldingLevel == 3 ? 2 : dualWieldingLevel > 0 ? 1 : 0;

    if (charLayout.itemType == ItemType::BlasterPistol && GetPerkLevel(&charLayout.perks, CharPerk::BlasterPistol_RapidShot))
    {
        specialApr += 1;
		mainHandAb -= 2;
    }

    if (charLayout.itemType == ItemType::BlasterRifle && !GetPerkLevel(&charLayout.perks, CharPerk::BlasterRifle_RapidReload))
    {
        regularApr = 1;
    }

	int offHandAb = mainHandAb;

	if (dualWieldingLevel == 1)
	{
		mainHandAb -= 4;
		offHandAb -= 8;
	}
	else if (dualWieldingLevel > 1)
	{
		mainHandAb -= 4;
		offHandAb -= 4;
	}

	if (dualWieldingLevel && charLayout.itemType == ItemType::FinesseVibroblade)
	{
		mainHandAb += 2;
		offHandAb += 2;
	}

	int weaponSpecLevels = GetPerkLevel(&charLayout.perks, GetWeaponSpecPerk(charLayout.itemType));
	if (weaponSpecLevels)
	{
		mainHandAb += 1;
		offHandAb += 1;
	}

    CharSkill::Enum governingSkill = GetGoverningSkill(charLayout.itemType);

	int plasmaCellLevel = 0;
	if (charLayout.itemType == ItemType::BlasterPistol)
	{
		plasmaCellLevel = GetPerkLevel(&charLayout.perks, CharPerk::BlasterPistol_PlasmaCell);
	}
	else if (charLayout.itemType == ItemType::BlasterRifle)
	{
		// PROPOSED CHANGES
		plasmaCellLevel = GetPerkLevel(&charLayout.perks, CharPerk::BlasterRifle_PlasmaCell);
	}

    bool swordOath = GetPerkLevel(&charLayout.perks, CharPerk::Stance_SwordOath);
    bool shieldOath = GetPerkLevel(&charLayout.perks, CharPerk::Stance_ShieldOath);

	// PROPOSED CHANGES
	swordOath = swordOath && IsRanged(charLayout.itemType);

	int weaponCritLevels = GetPerkLevel(&charLayout.perks, GetWeaponCritPerk(charLayout.itemType));

	float mainHandStrMultiplier = 1.0f;
	if (governingSkill == CharSkill::Melee_TwoHanded)
	{
		mainHandStrMultiplier = 1.5f;
	}

	float offHandStrMultiplier = 0.5f;

	int critRange = 1;
	int critMultiplier = 1;

	switch (charLayout.itemType)
	{
		case ItemType::Vibroblade: critRange = 2; critMultiplier = 2; break; // 19-20 x2 katana
		case ItemType::FinesseVibroblade: critRange = 3; critMultiplier = 2; break; // 18-20 x2 rapier
		case ItemType::Baton: critRange = 1; critMultiplier = 2; break; // 20 x2 morningstar
		case ItemType::HeavyVibroblade: critRange = 2; critMultiplier = 2; break; // 19-20 x2 greatsword
		case ItemType::Saberstaff: critRange = 2; critMultiplier = 2; break; // 19-20 x2 saberstaff
		case ItemType::Polearm: critRange = 1; critMultiplier = 3; break; // 20 x3 halberd
		case ItemType::TwinBlade: critRange = 2; critMultiplier = 2; break; // 19-20 x2 double weapon
		case ItemType::MartialArtWeapon: break; // Unknown?
		case ItemType::BlasterPistol: critRange = 1; critMultiplier = 2; break; // 20 x2 d20 pistol
		case ItemType::BlasterRifle: critRange = 1; critMultiplier = 2; break; // 20 x2 d20 rifle
		case ItemType::Lightsaber: critRange = 2; critMultiplier = 2; break; // 19-20 x2 lightsaber
	}

	if (weaponCritLevels)
	{
		critRange *= 2;
	}

	constexpr static int simAttempts = 2500;

	std::vector<float> damages;
	damages.reserve(simAttempts);

	for (int opponentAc = 0; opponentAc < 100; ++opponentAc)
	{
		damages.clear();

		for (int attempt = 0; attempt < simAttempts; ++attempt)
		{
			auto simAttack = [&](int ab, float strMultiplier = 1.0f)
			{
				int hitRoll = Roll(1, 20);

				if (ab + hitRoll < opponentAc && hitRoll != 20)
				{
					return 0.0f; // Miss
				}

				bool crit = false;
				if (ab + 20 >= opponentAc && hitRoll > 20 - critRange)
				{
					crit = ab + Roll(1, 20) >= opponentAc; // Crit confirmation roll
				}

				float baseDamage = 0.0f;
				if (weaponSpecLevels == 2)
				{
					baseDamage += 2.0f;
				}

				switch (charLayout.itemType)
				{
					case ItemType::Vibroblade: baseDamage += Roll(1, 10); break; // 1d10 katana
					case ItemType::FinesseVibroblade: baseDamage += Roll(1, 6); break; // 1d6 rapier
					case ItemType::Baton: baseDamage += Roll(1, 8); break; // 1d8 morningstar
					case ItemType::HeavyVibroblade: baseDamage += Roll(2, 6); break; // 2d6 greatsword
					case ItemType::Saberstaff: baseDamage += Roll(2, 6); break; // 2d6
					case ItemType::Polearm: baseDamage += Roll(1, 10); break; // 1d10 halberd
					case ItemType::TwinBlade: baseDamage += Roll(1, 8); break; // 1d8
					case ItemType::MartialArtWeapon: break; // Unknown ?
					case ItemType::BlasterPistol: baseDamage += Roll(2, 6); break; // 2d6
					case ItemType::BlasterRifle: baseDamage += Roll(2, 8); break; // 2d8
					case ItemType::Lightsaber: baseDamage += Roll(2, 6) + Roll(1, 4); break; // 2d6 + 1d4
				}

				float strDamage = 0.0f;
				float specialDamage = 0.0f;

				if (!IsRanged(charLayout.itemType))
				{
					strDamage += (statLayout.str - 10) / 2 * strMultiplier;
				}

				if (plasmaCellLevel)
				{
					int rollAttempts = 1 + (plasmaCellLevel / 2);
					int chance = (plasmaCellLevel / 2) * 10;

					for (int i = 0; i < rollAttempts; ++i)
					{
						if (Roll(1, 100) < chance)
						{
							specialDamage += Roll(1, 6);
						}
					}
				}

				if (crit)
				{
					baseDamage *= critMultiplier;
					strDamage *= critMultiplier;
				}

				// PROPOSED CHANGES
				if (IsRanged(charLayout.itemType))
				{
					specialDamage += (statLayout.dex - 10) / 2 * 0.25f;
				}

				// PROPOSED CHANGES
				if (charLayout.itemType == ItemType::Lightsaber || charLayout.itemType == ItemType::Saberstaff)
				{
					specialDamage += (statLayout.cha - 10) / 2 * 0.25f;
				}

				float totalDamage = baseDamage + strDamage + specialDamage;

				if (swordOath)
				{
					totalDamage += totalDamage / 100.0f * 20.0f;
				}
				else if (shieldOath)
				{
					totalDamage -= totalDamage / 100.0f * 30.0f;
				}

				return totalDamage;
			};

			float dmgThisRound = 0.0f;

			for (int i = 0; i < regularApr; ++i)
			{
				dmgThisRound += simAttack(mainHandAb - (i * 5), mainHandStrMultiplier);
			}

			for (int i = 0; i < offhandApr; ++i)
			{
				dmgThisRound += simAttack(offHandAb - (i * 5), offHandStrMultiplier);
			}

			for (int i = 0; i < specialApr; ++i)
			{
				dmgThisRound += simAttack(mainHandAb);
			}

			damages.emplace_back(dmgThisRound);
		}

		std::sort(std::begin(damages), std::end(damages));

		// 25th percentile
		int lowIndex = static_cast<int>(std::ceil(0.25f * simAttempts));
		result.dmgLow[opponentAc] = damages[lowIndex];

		// avg
		result.dmgAvg[opponentAc] = std::accumulate(std::begin(damages), std::end(damages), 0.0f) / simAttempts;

		// 75th percentile
		int highIndex = static_cast<int>(std::ceil(0.75f * simAttempts));
		result.dmgHigh[opponentAc] = damages[highIndex];
	}

    result.charLayout = std::move(charLayout);
    result.statLayout = std::move(statLayout);

    return result;
}

std::vector<CharPerk::Enum> GetPerksForItemType(ItemType::Enum itemType)
{
    switch (itemType)
    {
        case ItemType::Vibroblade: return
        { 
            CharPerk::Vibroblade_ImprovedCrit,
            CharPerk::Vibroblade_Proficiency,
            CharPerk::Vibroblade_WeaponFocus
        };

        case ItemType::FinesseVibroblade: return 
        { 
            CharPerk::FinesseVibroblade_Proficiency,
            CharPerk::FinesseVibroblade_ImprovedCrit,
            CharPerk::FinesseVibroblade_WeaponFinesse,
            CharPerk::FinesseVibroblade_WeaponFocus 
        };

        case ItemType::Baton: return
        {
            CharPerk::Baton_Proficiency,
            CharPerk::Baton_ImprovedCrit,
            CharPerk::Baton_WeaponFocus
        };

        case ItemType::HeavyVibroblade: return 
        {
            CharPerk::HeavyVibroblade_Proficiency,
            CharPerk::HeavyVibroblade_ImprovedCrit,
            CharPerk::HeavyVibroblade_WeaponFocus
        };

        case ItemType::Saberstaff: return
        {
            CharPerk::SaberStaff_ImprovedCrit,
            CharPerk::SaberStaff_Proficiency,
            CharPerk::SaberStaff_WeaponFocus
        };

        case ItemType::Polearm: return
        {
            CharPerk::Polearm_Proficiency,
            CharPerk::Polearm_ImprovedCrit,
            CharPerk::Polearm_WeaponFocus
        };

        case ItemType::TwinBlade: return
        {
            CharPerk::TwinVibroblade_ImprovedCrit,
            CharPerk::TwinVibroblade_Mastery,
            CharPerk::TwinVibroblade_Proficiency,
            CharPerk::TwinVibroblade_WeaponFocus
        };

        case ItemType::MartialArtWeapon: return
        {
            CharPerk::MartialArts_ImprovedCrit,
            CharPerk::MartialArts_Proficiency,
            CharPerk::MartialArts_WeaponFocus
        };

        case ItemType::BlasterPistol: return
        {
            CharPerk::BlasterPistol_Proficiency,
            CharPerk::BlasterPistol_ImprovedCrit,
            CharPerk::BlasterPistol_PlasmaCell,
            CharPerk::BlasterPistol_RapidShot,
            CharPerk::BlasterPistol_WeaponFocus
        };

        case ItemType::BlasterRifle: return
        {
            CharPerk::BlasterRifle_Proficiency,
            CharPerk::BlasterRifle_ImprovedCrit,
            CharPerk::BlasterRifle_RapidReload,
            CharPerk::BlasterRifle_WeaponFocus
        };

        case ItemType::Lightsaber: return
        {
            CharPerk::Lightsaber_Proficiency
        };
    }

    assert(false);
    return {};
}

std::uint8_t GetSPCostForLevel(CharPerk::Enum perk, std::uint8_t level)
{
    switch (perk)
    {
        case CharPerk::BlasterPistol_Proficiency:
        case CharPerk::BlasterRifle_Proficiency:
        case CharPerk::MartialArts_Proficiency:
        case CharPerk::Baton_Proficiency:
        case CharPerk::FinesseVibroblade_Proficiency:
        case CharPerk::Lightsaber_Proficiency:
        case CharPerk::Vibroblade_Proficiency:
        case CharPerk::SaberStaff_Proficiency:
        case CharPerk::TwinVibroblade_Proficiency:
        case CharPerk::HeavyVibroblade_Proficiency:
        case CharPerk::Polearm_Proficiency:
        {
            if (level < 4) return 3;
            else if (level < 7) return 4;
            else if (level < 9) return 5;
            else return 6;
        }

        case CharPerk::BlasterPistol_ImprovedCrit:
        case CharPerk::BlasterRifle_ImprovedCrit:
        case CharPerk::MartialArts_ImprovedCrit:
        case CharPerk::Baton_ImprovedCrit:
        case CharPerk::FinesseVibroblade_ImprovedCrit:
        case CharPerk::Vibroblade_ImprovedCrit:
        case CharPerk::SaberStaff_ImprovedCrit:
        case CharPerk::TwinVibroblade_ImprovedCrit:
        case CharPerk::HeavyVibroblade_ImprovedCrit:
        case CharPerk::Polearm_ImprovedCrit:
        {
            return 3;
        }

        case CharPerk::BlasterPistol_WeaponFocus:
        case CharPerk::BlasterRifle_WeaponFocus:
        case CharPerk::MartialArts_WeaponFocus:
        case CharPerk::Baton_WeaponFocus:
        case CharPerk::FinesseVibroblade_WeaponFocus:
        case CharPerk::Vibroblade_WeaponFocus:
        case CharPerk::SaberStaff_WeaponFocus:
        case CharPerk::TwinVibroblade_WeaponFocus:
        case CharPerk::HeavyVibroblade_WeaponFocus:
        case CharPerk::Polearm_WeaponFocus:
        {
            return level == 1 ? 3 : 4;
        }

        case CharPerk::BlasterPistol_PlasmaCell:
        {
            if (level < 3) return 2;
            else if (level < 5) return 3;
            else if (level < 7) return 4;
            else if (level < 9) return 5;
            else if (level < 10) return 6;
            else return 7;
        }

        case CharPerk::BlasterPistol_RapidShot:
        {
            return 4;
        }
        
        case CharPerk::BlasterRifle_RapidReload:
        {
            return 4;
        }

        case CharPerk::Firearms_PointBlankShot:
        {
            return 2;
        }
        
        case CharPerk::Firearms_ZenMarksmanship:
        {
            return 3;
        }

        case CharPerk::FinesseVibroblade_WeaponFinesse:
        {
            return 3;
        }

        case CharPerk::OneHanded_DualWielding:
        {
            if (level == 1) return 3;
            else if (level == 2) return 4;
            else if (level == 3) return 6;
        }

        case CharPerk::Stance_ShieldOath:
        {
            return 8;
        }

        case CharPerk::Stance_SwordOath:
        {
            return 8;
        }

        case CharPerk::TwinVibroblade_Mastery:
        {
            if (level == 1) return 3;
            else if (level == 2) return 4;
            else if (level == 3) return 6;
        }         
    }

    assert(false);
    return 0;
}

std::uint8_t GetMaxPerkLevel(CharPerk::Enum perk)
{
    switch (perk)
    {
        case CharPerk::BlasterPistol_Proficiency:
        case CharPerk::BlasterRifle_Proficiency:
        case CharPerk::MartialArts_Proficiency:
        case CharPerk::Baton_Proficiency:
        case CharPerk::FinesseVibroblade_Proficiency:
        case CharPerk::Lightsaber_Proficiency:
        case CharPerk::Vibroblade_Proficiency:
        case CharPerk::SaberStaff_Proficiency:
        case CharPerk::TwinVibroblade_Proficiency:
        case CharPerk::HeavyVibroblade_Proficiency:
        case CharPerk::Polearm_Proficiency:
        case CharPerk::BlasterPistol_PlasmaCell:
        {
            return 10;
        }

        case CharPerk::OneHanded_DualWielding:
        case CharPerk::TwinVibroblade_Mastery:
        {
            return 3;
        }
    }
    
    return 1;
}

std::uint8_t GetMaxSkillLevel(CharSkill::Enum skill)
{
    switch (skill)
    {
        case CharSkill::Crafting_Weaponsmith:
        case CharSkill::Crafting_Armorsmith:
        case CharSkill::Crafting_Fabrication:
        case CharSkill::Crafting_Engineering:
        case CharSkill::Gathering_Scavenging:
        case CharSkill::Utility_Medicine:
        case CharSkill::Utility_Farming:
        {
            return 50;
        }

        case CharSkill::Crafting_Cooking:
        {
            return 30;
        }
    }

    return 100;
}

CharSkill::Enum GetGoverningSkill(ItemType::Enum type)
{
    switch (type)
    {
        case ItemType::Vibroblade: return CharSkill::Melee_OneHanded;
        case ItemType::FinesseVibroblade: return CharSkill::Melee_OneHanded;
        case ItemType::Baton: return CharSkill::Melee_OneHanded;
        case ItemType::HeavyVibroblade: return CharSkill::Melee_TwoHanded;
        case ItemType::Polearm: return CharSkill::Melee_TwoHanded;
        case ItemType::TwinBlade: return CharSkill::Melee_TwinBlades;
        case ItemType::MartialArtWeapon: return CharSkill::Melee_MartialArts;
        case ItemType::BlasterPistol: return CharSkill::Ranged_Blasters;
        case ItemType::BlasterRifle: return CharSkill::Ranged_Blasters;
		case ItemType::Saberstaff: return CharSkill::Melee_Lightsabers;
        case ItemType::Lightsaber: return CharSkill::Melee_Lightsabers;
    }

    return CharSkill::Invalid;
}

CharPerk::Enum GetWeaponSpecPerk(ItemType::Enum type)
{
	switch (type)
	{
		case ItemType::Vibroblade: return CharPerk::Vibroblade_WeaponFocus;
		case ItemType::FinesseVibroblade: return CharPerk::FinesseVibroblade_WeaponFocus;
		case ItemType::Baton: return CharPerk::Baton_WeaponFocus;
		case ItemType::HeavyVibroblade: return CharPerk::HeavyVibroblade_WeaponFocus;
		case ItemType::Saberstaff: return CharPerk::SaberStaff_WeaponFocus;
		case ItemType::Polearm: return CharPerk::Polearm_WeaponFocus;
		case ItemType::TwinBlade: return CharPerk::TwinVibroblade_WeaponFocus;
		case ItemType::MartialArtWeapon: return CharPerk::MartialArts_WeaponFocus;
		case ItemType::BlasterPistol: return CharPerk::BlasterPistol_WeaponFocus;
		case ItemType::BlasterRifle: return CharPerk::BlasterRifle_WeaponFocus;
		case ItemType::Lightsaber: return CharPerk::Lightsaber_WeaponFocus;
	}

	return CharPerk::Invalid;
}

CharPerk::Enum GetWeaponProficiencyPerk(ItemType::Enum type)
{
	switch (type)
    {
        case ItemType::Vibroblade: return CharPerk::Vibroblade_Proficiency;
		case ItemType::FinesseVibroblade: return CharPerk::FinesseVibroblade_Proficiency;
		case ItemType::Baton: return CharPerk::Baton_Proficiency;
		case ItemType::HeavyVibroblade: return CharPerk::HeavyVibroblade_Proficiency;
		case ItemType::Saberstaff: return CharPerk::SaberStaff_Proficiency;
		case ItemType::Polearm: return CharPerk::Polearm_Proficiency;
		case ItemType::TwinBlade: return CharPerk::TwinVibroblade_Proficiency;
		case ItemType::MartialArtWeapon: return CharPerk::MartialArts_Proficiency;
		case ItemType::BlasterPistol: return CharPerk::BlasterPistol_Proficiency;
		case ItemType::BlasterRifle: return CharPerk::BlasterRifle_Proficiency;
		case ItemType::Lightsaber: return CharPerk::Lightsaber_Proficiency;
    }

	return CharPerk::Invalid;
}

CharPerk::Enum GetWeaponCritPerk(ItemType::Enum type)
{
	switch (type)
	{
		case ItemType::Vibroblade: return CharPerk::Vibroblade_ImprovedCrit;
		case ItemType::FinesseVibroblade: return CharPerk::FinesseVibroblade_ImprovedCrit;
		case ItemType::Baton: return CharPerk::Baton_ImprovedCrit;
		case ItemType::HeavyVibroblade: return CharPerk::HeavyVibroblade_ImprovedCrit;
		case ItemType::Saberstaff: return CharPerk::SaberStaff_ImprovedCrit;
		case ItemType::Polearm: return CharPerk::Polearm_ImprovedCrit;
		case ItemType::TwinBlade: return CharPerk::TwinVibroblade_ImprovedCrit;
		case ItemType::MartialArtWeapon: return CharPerk::MartialArts_ImprovedCrit;
		case ItemType::BlasterPistol: return CharPerk::BlasterPistol_ImprovedCrit;
		case ItemType::BlasterRifle: return CharPerk::BlasterRifle_ImprovedCrit;
		case ItemType::Lightsaber: return CharPerk::Lightsaber_ImprovedCrit;
	}

	return CharPerk::Invalid;
}

CharBackground::Enum GetBackgroundForBABBonus(ItemType::Enum type)
{
	switch (type)
    {
		case ItemType::BlasterPistol: return CharBackground::Smuggler;
		case ItemType::BlasterRifle: return CharBackground::Sharpshooter;
		case ItemType::MartialArtWeapon: return CharBackground::TerasKasi;
		case ItemType::Baton: return CharBackground::SecurityOfficer;
		case ItemType::Vibroblade: return CharBackground::Berserker;
		case ItemType::FinesseVibroblade: return CharBackground::Duelist;
		case ItemType::HeavyVibroblade: return CharBackground::Soldier;
    }

	return CharBackground::Invalid;
}

std::string ToString(ItemType::Enum type)
{
	switch (type)
    {
        case ItemType::Vibroblade: return "Vibroblade";
		case ItemType::FinesseVibroblade: return "FinesseVibroblade";
		case ItemType::Baton: return "Baton";
		case ItemType::HeavyVibroblade: return "HeavyVibroblade";
		case ItemType::Saberstaff: return "Saberstaff";
		case ItemType::Polearm: return "Polearm";
		case ItemType::TwinBlade: return "TwinVibroblade";
		case ItemType::MartialArtWeapon: return "MartialArts";
		case ItemType::BlasterPistol: return "BlasterPistol";
		case ItemType::BlasterRifle: return "BlasterRifle";
		case ItemType::Lightsaber: return "Lightsaber";
    }

	return "Invalid";
}

bool IsRanged(ItemType::Enum type)
{
    return type == ItemType::BlasterPistol || type == ItemType::BlasterRifle;
}

int GetPerkLevel(const std::vector<LevelledPerk>* perks, CharPerk::Enum perk)
{
	if (perk == CharPerk::Invalid)
	{
		return 0;
	}

    for (const auto& perkLevelled : *perks)
    {
        if (perkLevelled.perk == perk)
        {
            return perkLevelled.level;
        }
    }

    return 0;
}

int GetPrimary(CharSkill::Enum skill)
{
    switch (skill)
    {
        case CharSkill::Melee_OneHanded:
        case CharSkill::Melee_TwoHanded:
        case CharSkill::Melee_TwinBlades:
        case CharSkill::Melee_MartialArts:
            return BONUS_STR;

        case CharSkill::Melee_Lightsabers:
        case CharSkill::Ranged_Blasters:
        case CharSkill::Ranged_Throwing:
        case CharSkill::Defense_LightArmor:
        case CharSkill::Crafting_Weaponsmith:
        case CharSkill::Crafting_Engineering:
            return BONUS_DEX;

        case CharSkill::Defense_HeavyArmor:
        case CharSkill::Defense_Shields:
        case CharSkill::Crafting_Armorsmith:
        case CharSkill::Gathering_Harvesting:
        case CharSkill::Gathering_Scavenging:
        case CharSkill::Utility_Farming:
            return BONUS_CON;

        case CharSkill::Crafting_Fabrication:
        case CharSkill::Crafting_Cooking:
        case CharSkill::Utility_Medicine:
        case CharSkill::Utility_ItemRepair:
        case CharSkill::Force_DarkSide:
            return BONUS_INT;

        case CharSkill::Force_LightSide:
            return BONUS_WIS;

        case CharSkill::Defense_ForceArmor:
            return BONUS_CHA;
    }

    return BONUS_NONE;
}

int GetSecondary(CharSkill::Enum skill)
{
    switch (skill)
    {   
        case CharSkill::Ranged_Throwing:
        case CharSkill::Defense_HeavyArmor:
        case CharSkill::Defense_Shields:
        case CharSkill::Gathering_Harvesting:
            return BONUS_STR;

        case CharSkill::Melee_OneHanded:
        case CharSkill::Melee_MartialArts:
        case CharSkill::Utility_ItemRepair:
            return BONUS_DEX;

        case CharSkill::Melee_TwoHanded:
        case CharSkill::Melee_TwinBlades:
        case CharSkill::Defense_LightArmor:
        case CharSkill::Crafting_Fabrication:
            return BONUS_CON;

        case CharSkill::Crafting_Weaponsmith:
        case CharSkill::Crafting_Armorsmith:
        case CharSkill::Force_LightSide:
            return BONUS_INT;

        case CharSkill::Ranged_Blasters:
        case CharSkill::Defense_ForceArmor:
        case CharSkill::Crafting_Engineering:
        case CharSkill::Gathering_Scavenging:
        case CharSkill::Utility_Medicine:
        case CharSkill::Force_DarkSide:
            return BONUS_WIS;

        case CharSkill::Melee_Lightsabers:
        case CharSkill::Crafting_Cooking:
        case CharSkill::Utility_Farming:
            return BONUS_CHA;  
    }

    return BONUS_NONE;
}

int GetTertiary(CharSkill::Enum skill)
{
    switch (skill)
    {
        case CharSkill::Defense_ForceArmor:
            return BONUS_INT;

        case CharSkill::Utility_Farming:
            return BONUS_WIS;
            
        case CharSkill::Utility_Medicine:
        case CharSkill::Force_DarkSide:
        case CharSkill::Force_LightSide:
            return BONUS_CHA;
    }

    return BONUS_NONE;
}