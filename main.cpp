#include "brute_force.h"
#include "simulation.h"
#include "matplotlibcpp.h"

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
        auto getColourString = []() -> std::string
        {
			constexpr static const char* colours[] = 
			{
				"#000000", "#FFFF00", "#1CE6FF", "#FF34FF", "#FF4A46", "#008941", "#006FA6", "#A30059",
				"#FFDBE5", "#7A4900", "#0000A6", "#63FFAC", "#B79762", "#004D43", "#8FB0FF", "#997D87",
				"#5A0007", "#809693", "#FEFFE6", "#1B4400", "#4FC601", "#3B5DFF", "#4A3B53", "#FF2F80",
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
			static int s_Index = 0;
            return colours[s_Index++];
        };

        std::vector<SimulationResult> results;
        std::vector<std::string> resultNames;
        std::vector<std::string> resultColours;

		for (int i = 0; i < ItemType::EnumCount; ++i)
		{
			CharLayout charLayout;
			charLayout.itemType = static_cast<ItemType::Enum>(i);
			charLayout.background = GetBackgroundForBABBonus(charLayout.itemType);
			charLayout.skills.push_back({ GetGoverningSkill(charLayout.itemType), 100 });
			charLayout.perks.push_back({ CharPerk::Stance_SwordOath, 1 });

			CharPerk::Enum profiencyPerk = GetWeaponProficiencyPerk(charLayout.itemType);
			CharPerk::Enum specPerk = GetWeaponSpecPerk(charLayout.itemType);
			CharPerk::Enum critPerk = GetWeaponCritPerk(charLayout.itemType);

			if (profiencyPerk != CharPerk::Invalid)
			{
				charLayout.perks.push_back({ profiencyPerk, 10 });
			}

			if (specPerk != CharPerk::Invalid)
			{
				charLayout.perks.push_back({ specPerk, 2 });
			}

			if (critPerk != CharPerk::Invalid)
			{
				charLayout.perks.push_back({ critPerk, 1 });
			}			

			StatLayout statsLayout = BuildStats(charLayout);
			statsLayout.str = 80;
			statsLayout.dex = 80;

			if (charLayout.itemType == ItemType::FinesseVibroblade)
			{
				statsLayout.str = 60;
				charLayout.perks.push_back({ CharPerk::FinesseVibroblade_WeaponFinesse, 1 });
			}

			if (charLayout.itemType == ItemType::BlasterRifle)
			{
				charLayout.perks.push_back({ CharPerk::BlasterRifle_RapidReload, 1 });
			}

			if (charLayout.itemType == ItemType::TwinBlade)
			{
				charLayout.perks.push_back({ CharPerk::TwinVibroblade_Mastery , 3 });
			}

			if (charLayout.itemType == ItemType::Lightsaber || charLayout.itemType == ItemType::Saberstaff)
			{
				statsLayout.cha = 60;
			}

			if (charLayout.itemType == ItemType::Saberstaff)
			{
				// TEMP - fix!
				charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding , 3 });
			}

			std::string format = ToString(charLayout.itemType);
			results.emplace_back(DoSimulation(charLayout, statsLayout));
			resultNames.emplace_back(format);
			resultColours.emplace_back(getColourString());

			if (charLayout.itemType == ItemType::Lightsaber)
			{
				charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding, 3 });
				results.emplace_back(DoSimulation(charLayout, statsLayout));
				resultNames.emplace_back(format + " (DualWield)");
				resultColours.emplace_back(getColourString());
			}

			if (charLayout.itemType == ItemType::FinesseVibroblade)
			{
				charLayout.perks.push_back({ CharPerk::OneHanded_DualWielding, 3 });
				results.emplace_back(DoSimulation(charLayout, statsLayout));
				resultNames.emplace_back(format + " (DualWield)");
				resultColours.emplace_back(getColourString());
			}

			if (charLayout.itemType == ItemType::BlasterPistol)
			{
				charLayout.perks.push_back({ CharPerk::BlasterPistol_RapidShot, 1 });
				results.emplace_back(DoSimulation(charLayout, statsLayout));
				resultNames.emplace_back(format + " (w/ RapidShot)");
				resultColours.emplace_back(getColourString());

				charLayout.perks.push_back({ CharPerk::BlasterPistol_PlasmaCell, 10 });
				results.emplace_back(DoSimulation(charLayout, statsLayout));
				resultNames.emplace_back(format + " (w/ RapidShot, PlasmaCell)");
				resultColours.emplace_back(getColourString());
			}

			if (charLayout.itemType == ItemType::BlasterRifle)
			{
				charLayout.perks.push_back({ CharPerk::BlasterRifle_PlasmaCell, 10 });
				results.emplace_back(DoSimulation(charLayout, statsLayout));
				resultNames.emplace_back(format + " (w/ PlasmaCell)");
				resultColours.emplace_back(getColourString());
			}
		}

        matplotlibcpp::title("Average damage per round (with maxed skills, perks, 80 main stat, 60 off stat)");
        matplotlibcpp::figure_size(1920, 1080);

        std::vector<double> ac;
        for (int opponentAc = 0; opponentAc < 100; ++opponentAc)
        {
            ac.emplace_back(opponentAc);
        }

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
        matplotlibcpp::save("D:/damage.png");
    }
}