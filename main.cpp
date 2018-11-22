#include "brute_force.h"
#include "simulation.h"
#include "matplotlibcpp.h"

#include <set>
#include <string>

int main()
{
    constexpr bool bruteforce = false;

    if (bruteforce)
    {
		SetupBruteForceSearcher();

        BruteForceJob job;
        job.itemType = ItemType::BlasterPistol;
        job.skillCandidates =
        {
            CharSkill::Melee_OneHanded,
            CharSkill::Melee_TwoHanded,
            CharSkill::Melee_TwinBlades,
            CharSkill::Melee_MartialArts,
            CharSkill::Melee_Lightsabers,

            CharSkill::Ranged_Blasters,
            CharSkill::Ranged_Throwing,

            CharSkill::Defense_LightArmor,
            CharSkill::Defense_HeavyArmor,
            CharSkill::Defense_Shields,
            CharSkill::Defense_ForceArmor,

            CharSkill::Crafting_Weaponsmith,
            CharSkill::Crafting_Armorsmith,
            CharSkill::Crafting_Fabrication,
            CharSkill::Crafting_Cooking,
            CharSkill::Crafting_Engineering,

            CharSkill::Gathering_Harvesting,
            CharSkill::Gathering_Scavenging,

            CharSkill::Utility_Medicine,
            CharSkill::Utility_ItemRepair,
            CharSkill::Utility_Farming,

            CharSkill::Force_DarkSide,
            CharSkill::Force_LightSide,
        };
        job.perkCandidates = GetPerksForItemType(job.itemType);

        if (IsRanged(job.itemType))
        {
            job.perkCandidates.emplace_back(CharPerk::Firearms_PointBlankShot);
            job.perkCandidates.emplace_back(CharPerk::Firearms_ZenMarksmanship);
        }
        else
        {
            job.perkCandidates.emplace_back(CharPerk::OneHanded_DualWielding);
        }

        for (int background = 0; background < CharBackground::EnumCount; ++background)
        {
            job.background = static_cast<CharBackground::Enum>(background);
            QueueBruteForceJob(job);
        }

		std::vector<SimulationResult> results = CollectTopBruteForceResults();
    }
	else
	{
		std::set<CharSkill::Enum> categories;
		for (int i = 0; i < ItemType::EnumCount; ++i)
		{
			categories.insert(GetGoverningSkill(static_cast<ItemType::Enum>(i)));
		}

		auto doCategory = [](CharSkill::Enum category)
		{
			static int s_ColourIndex;
			s_ColourIndex = 0;

			std::vector<SimulationResult> results;
			std::vector<std::string> resultNames;
			std::vector<std::string> resultColours;

			auto doResult = [&results, &resultNames, &resultColours](CharLayout& charLayout, StatLayout& statsLayout, const char* extra = nullptr)
			{
				auto getColourString = []() -> std::string
				{
					constexpr static const char* colours[] =
					{
						"#000000", "#FFFF00", "#1CE6FF", "#FF34FF", "#FF4A46", "#008941", "#006FA6", "#A30059",
						"#FFDBE5", "#7A4900", "#0000A6", "#63FFAC", "#B79762", "#004D43", "#8FB0FF", "#997D87",
						"#5A0007", "#809693", "#71CAB2", "#1B4400", "#4FC601", "#3B5DFF", "#4A3B53", "#FF2F80",
						"#61615A", "#BA0900", "#6B7900", "#00C2A0", "#FFAA92", "#FF90C9", "#B903AA", "#D16100",
						"#DDEFFF", "#000035", "#7B4F4B", "#A1C299", "#300018", "#0AA6D8", "#013349", "#00846F",
						"#372101", "#FFB500", "#C2FFED", "#A079BF", "#CC0744", "#C0B9B2", "#C2FF99", "#001E09",
						"#00489C", "#6F0062", "#0CBD66", "#EEC3FF", "#456D75", "#B77B68", "#7A87A1", "#788D66",
						"#885578", "#FAD09F", "#FF8A9A", "#D157A0", "#BEC459", "#456648", "#0086ED", "#886F4C",

						"#34362D", "#B4A8BD", "#00A6AA", "#452C2C", "#636375", "#A3C8C9", "#FF913F", "#938A81",
						"#575329", "#00FECF", "#B05B6F", "#8CD0FF", "#3B9700", "#04F757", "#C8A1A1", "#1E6E00",
						"#7900D7", "#A77500", "#6367A9", "#A05837", "#6B002C", "#772600", "#D790FF", "#9B9700",
						"#549E79", "#FFF69F", "#201625", "#72418F", "#BC23FF", "#99ADC0", "#3A2465", "#922329",
						"#5B4534", "#FDE8DC", "#404E55", "#0089A3", "#CB7E98", "#A4E804", "#324E72", "#6A3A4C",
						"#83AB58", "#001C1E", "#D1F7CE", "#004B28", "#C8D0F6", "#A3A489", "#806C66", "#222800",
						"#BF5650", "#E83000", "#66796D", "#DA007C", "#FF1A59", "#8ADBB4", "#1E0200", "#5B4E51",
						"#C895C5", "#320033", "#FF6832", "#66E1D3", "#CFCDAC", "#D0AC94", "#7ED379", "#012C58"
					};

					return colours[s_ColourIndex++];
				};

				std::string format = ToString(charLayout.itemType);

				if (statsLayout.str > 10)
				{
					format += " [STR:" + std::to_string(statsLayout.str) + "]";
				}

				if (statsLayout.dex > 10)
				{
					format += " [DEX:" + std::to_string(statsLayout.dex) + "]";
				}

				if (statsLayout.cha > 10)
				{
					format += " [CHA:" + std::to_string(statsLayout.cha) + "]";
				}

				if (extra)
				{
					format += std::string(" ") + extra;
				}

				results.emplace_back(DoSimulation(charLayout, statsLayout));
				resultNames.emplace_back(format);
				resultColours.emplace_back(getColourString());
			};

			for (int i = 0; i < ItemType::EnumCount; ++i)
			{
				CharLayout charLayout;
				charLayout.itemType = static_cast<ItemType::Enum>(i);

				CharSkill::Enum governingSkill = GetGoverningSkill(charLayout.itemType);
				if (category != governingSkill && category != CharSkill::Invalid)
				{
					continue;
				}

				charLayout.background = GetBackgroundForBABBonus(charLayout.itemType);
				charLayout.skills.push_back({ governingSkill, GetMaxSkillLevel(governingSkill) });

				if (IsRanged(charLayout.itemType))
				{
					charLayout.perks.push_back({ CharPerk::Stance_PrecisionTargeting, GetMaxPerkLevel(CharPerk::Stance_PrecisionTargeting) });
				}

				CharPerk::Enum profiencyPerk = GetWeaponProficiencyPerk(charLayout.itemType);
				CharPerk::Enum specPerk = GetWeaponSpecPerk(charLayout.itemType);
				CharPerk::Enum critPerk = GetWeaponCritPerk(charLayout.itemType);

				if (profiencyPerk != CharPerk::Invalid)
				{
					charLayout.perks.push_back({ profiencyPerk, GetMaxPerkLevel(profiencyPerk) });
				}

				if (specPerk != CharPerk::Invalid)
				{
					charLayout.perks.push_back({ specPerk, GetMaxPerkLevel(specPerk) });
				}

				if (critPerk != CharPerk::Invalid)
				{
					charLayout.perks.push_back({ critPerk, GetMaxPerkLevel(critPerk) });
				}

				StatLayout statsLayout = BuildStats(charLayout);

				statsLayout.str = 10;
				statsLayout.dex = 10;
				statsLayout.con = 10;
				statsLayout.intelligence = 10;
				statsLayout.wis = 10;
				statsLayout.cha = 10;

				switch (charLayout.itemType)
				{
					case ItemType::Vibroblade: statsLayout.str = 80; break;
					case ItemType::FinesseVibroblade: statsLayout.str = 30; statsLayout.dex = 80; break;
					case ItemType::Baton: statsLayout.str = 80; break;
					case ItemType::HeavyVibroblade: statsLayout.str = 80; break;
					case ItemType::Polearm: statsLayout.str = 80; break;
					case ItemType::TwinBlade: statsLayout.str = 80; break;
					case ItemType::MartialArtWeapon: statsLayout.str = 80; break;
					case ItemType::BlasterPistol: statsLayout.dex = 80; break;
					case ItemType::BlasterRifle: statsLayout.dex = 80; break;
					case ItemType::Saberstaff: statsLayout.str = 30; statsLayout.dex = 60; statsLayout.cha = 40; break;
					case ItemType::Lightsaber: statsLayout.str = 30; statsLayout.dex = 60; statsLayout.cha = 40; break;
					default: assert(false); break;
				}

				if (charLayout.itemType == ItemType::FinesseVibroblade)
				{
					charLayout.perks.push_back({ CharPerk::FinesseVibroblade_WeaponFinesse, GetMaxPerkLevel(CharPerk::FinesseVibroblade_WeaponFinesse) });
				}

				if (charLayout.itemType == ItemType::BlasterRifle)
				{
					charLayout.perks.push_back({ CharPerk::BlasterRifle_RapidReload, GetMaxPerkLevel(CharPerk::BlasterRifle_RapidReload) });
				}

				if (charLayout.itemType == ItemType::TwinBlade)
				{
					charLayout.perks.push_back({ CharPerk::TwinVibroblade_Mastery , GetMaxPerkLevel(CharPerk::TwinVibroblade_Mastery) });
				}

				if (charLayout.itemType == ItemType::Saberstaff)
				{
					charLayout.perks.push_back({ CharPerk::SaberStaff_Mastery , GetMaxPerkLevel(CharPerk::SaberStaff_Mastery) });
				}

				doResult(charLayout, statsLayout);

				if (charLayout.itemType == ItemType::Lightsaber ||charLayout.itemType == ItemType::FinesseVibroblade || 
					charLayout.itemType == ItemType::Vibroblade || charLayout.itemType == ItemType::Baton)
				{
					charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding, GetMaxPerkLevel(CharPerk::OneHanded_DualWielding) });
					doResult(charLayout, statsLayout, "(DualWield)");
				}

				if (charLayout.itemType == ItemType::Vibroblade)
				{
					RemovePerk(&charLayout.perks, CharPerk::OneHanded_DualWielding);

					charLayout.perks.push_back({ CharPerk::Vibroblade_PowerAttack, 1 });
					doResult(charLayout, statsLayout, "(PowerAttack)");

					charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding, GetMaxPerkLevel(CharPerk::OneHanded_DualWielding) });
					doResult(charLayout, statsLayout, "(PowerAttack, DualWield)");

					RemovePerk(&charLayout.perks, CharPerk::OneHanded_DualWielding);
					RemovePerk(&charLayout.perks, CharPerk::Vibroblade_PowerAttack);

					charLayout.perks.push_back({ CharPerk::Vibroblade_PowerAttack, 2 });
					doResult(charLayout, statsLayout, "(ImpPowerAttack)");

					charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding, GetMaxPerkLevel(CharPerk::OneHanded_DualWielding) });
					doResult(charLayout, statsLayout, "(ImpPowerAttack, DualWield)");
				}

				if (charLayout.itemType == ItemType::Lightsaber || charLayout.itemType == ItemType::Saberstaff)
				{
					RemovePerk(&charLayout.perks, CharPerk::OneHanded_DualWielding);

					statsLayout.str = 80;
					statsLayout.dex = 10;
					statsLayout.cha = 30;

					doResult(charLayout, statsLayout);

					if (charLayout.itemType == ItemType::Lightsaber)
					{
						charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding, GetMaxPerkLevel(CharPerk::OneHanded_DualWielding) });
						doResult(charLayout, statsLayout, "(DualWield)");
					}
				}

				if (charLayout.itemType == ItemType::BlasterPistol || charLayout.itemType == ItemType::BlasterRifle)
				{
					charLayout.perks.push_back({ CharPerk::Firearms_PlasmaCell, GetMaxPerkLevel(CharPerk::Firearms_PlasmaCell) });
					doResult(charLayout, statsLayout, "(w/ PlasmaCell)");

					if (charLayout.itemType == ItemType::BlasterPistol)
					{
						charLayout.perks.push_back({ CharPerk::BlasterPistol_RapidShot, GetMaxPerkLevel(CharPerk::BlasterPistol_RapidShot) });
						doResult(charLayout, statsLayout, "(w/ PlasmaCell, RapidShot)");
					}
				}
			}

			matplotlibcpp::title("Average damage per round (with maxed skills, perks, 80 main stat, 60 off stat)");
			matplotlibcpp::figure_size(1920, 1080);

			std::vector<double> ac;
			for (int opponentAc = 0; opponentAc < 100; ++opponentAc)
			{
				ac.emplace_back(opponentAc);
			}

			if (!results.empty())
			{
				for (int i = 0; i < results.size(); ++i)
				{
					std::vector<double> damageLow;
					std::vector<double> damageAvg;
					std::vector<double> damageHigh;

					damageLow.reserve(100);
					damageAvg.reserve(100);
					damageHigh.reserve(100);

					for (int j = 0; j < 100; ++j)
					{
						damageLow.emplace_back(results[i].dmgLow[j]);
						damageAvg.emplace_back(results[i].dmgAvg[j]);
						damageHigh.emplace_back(results[i].dmgHigh[j]);
					}

					std::map<std::string, std::string> style =
					{
						{ "linestyle", "--" },
					{ "color", resultColours[i] + "20" }
					};

					matplotlibcpp::plot(ac, damageLow, style);
					matplotlibcpp::named_plot(resultNames[i], ac, damageAvg, resultColours[i]);
					matplotlibcpp::plot(ac, damageHigh, style);
				}

				matplotlibcpp::xlim(0, 100);
				matplotlibcpp::xlabel("Opponent AC");
				matplotlibcpp::ylabel("Damage per round");
				matplotlibcpp::legend();

				std::string filename = "damage";
				if (category != CharSkill::Invalid)
				{
					filename += "_" + ToString(category);
				}
				filename += ".png";

				matplotlibcpp::save("D://" + filename);
			}
		};

		for (CharSkill::Enum category : categories)
		{
			doCategory(category);
		}

		doCategory(CharSkill::Invalid);
	}
}