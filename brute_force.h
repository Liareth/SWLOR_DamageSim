#pragma once

#include "simulation.h"

struct BruteForceJob
{
	ItemType::Enum itemType;
	CharBackground::Enum background;
	std::vector<CharSkill::Enum> skillCandidates;
	std::vector<CharPerk::Enum> perkCandidates;
};

void SetupBruteForceSearcher();
void QueueBruteForceJob(const BruteForceJob& layout);
std::vector<SimulationResult> CollectTopBruteForceResults();
