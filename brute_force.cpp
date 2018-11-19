#include "brute_force.h"
#include "concurrentqueue.h"

#include <atomic>
#include <thread>

moodycamel::ConcurrentQueue<BruteForceJob> g_SimWorkQueue;
std::vector<SimulationResult> g_TopResults;
std::atomic<bool> g_Working = false;

std::vector<std::unique_ptr<std::thread>> g_Workers;
std::atomic<bool> g_WorkerFlag = false;

void WorkerThreadFunc()
{
	std::vector<SimulationResult> simResultCache;

	auto insert = [](SimulationResult& result)
	{
		constexpr int maxResults = 5;

		auto comparator = [](const SimulationResult& lhs, const SimulationResult& rhs) -> bool
		{
			return lhs.dmgAvg[60] > rhs.dmgAvg[60];
		};

		if (g_TopResults.size() == maxResults && !comparator(*g_TopResults.rbegin(), result))
		{
			return;
		}

		for (int i = 0; i < maxResults; ++i)
		{
			if (i >= g_TopResults.size() || comparator(g_TopResults[i], result))
			{
				g_TopResults.insert(std::begin(g_TopResults) + i, result);
				break;
			}
		}

		if (g_TopResults.size() > maxResults)
		{
			g_TopResults.pop_back();
		}
	};

	while (g_Working)
	{
		BruteForceJob job;
		while (g_SimWorkQueue.try_dequeue(job))
		{
			CharLayout charLayout;
			charLayout.itemType = job.itemType;
			charLayout.background = job.background;

			int skillCombos = static_cast<int>(std::pow(2, job.skillCandidates.size()));
			int perkCombos = static_cast<int>(std::pow(2, job.perkCandidates.size()));

			int spMax = 250;

			if (charLayout.background == CharBackground::Freelancer)
			{
				spMax += 3;
			}

			for (int skillCombo = 0; skillCombo < skillCombos; ++skillCombo)
			{
				charLayout.skills.clear();
				int spUsed = 0;

				for (int skillCandidateIndex = 0; skillCandidateIndex < job.skillCandidates.size(); ++skillCandidateIndex)
				{
					if (skillCombo >> skillCandidateIndex & 1)
					{
						LevelledSkill skill;
						skill.skill = job.skillCandidates[skillCandidateIndex];
						skill.level = GetMaxSkillLevel(skill.skill);

						spUsed += skill.level;

						if (spUsed > spMax)
						{
							skill.level -= spUsed - spMax;
						}

						charLayout.skills.emplace_back(std::move(skill));

						if (spUsed >= spMax)
						{
							break;
						}
					}
				}

				// Early elimination - this won't be a top candidate because we aren't using all our SP, so drop it.
				if (spUsed < spMax)
				{
					continue;
				}

				for (int perkCombo = 0; perkCombo < 16; ++perkCombo)
				{
					charLayout.perks.clear();

					for (int perkCandidateIndex = 0; perkCandidateIndex < job.perkCandidates.size(); ++perkCandidateIndex)
					{
						if (perkCombo >> perkCandidateIndex & 1)
						{
							LevelledPerk perk;
							perk.perk = job.perkCandidates[perkCandidateIndex];
							perk.level = GetMaxPerkLevel(perk.perk);
							charLayout.perks.emplace_back(std::move(perk));
						}
					}

					StatLayout statLayout = BuildStats(charLayout);
					simResultCache.emplace_back(DoSimulation(charLayout, statLayout));

					bool expected = false;
					if (simResultCache.size() > 50000 && g_WorkerFlag.compare_exchange_weak(expected, true))
					{
						for (auto& result : simResultCache)
						{
							insert(result);
						}

						g_WorkerFlag.store(false);
						simResultCache.clear();
					}
				}
			}
		}
	}

	bool expected = false;
	while (!g_WorkerFlag.compare_exchange_strong(expected, true))
	{
	}

	for (auto& result : simResultCache)
	{
		insert(result);
	}

	g_WorkerFlag = false;

}

void SetupBruteForceSearcher()
{
	g_Working = true;

	for (int i = 0; i < static_cast<int>(std::thread::hardware_concurrency()) - 1; ++i)
	{
		g_Workers.emplace_back(std::make_unique<std::thread>(&WorkerThreadFunc));
	}
}

void QueueBruteForceJob(const BruteForceJob& layout)
{
	g_SimWorkQueue.enqueue(layout);
}

std::vector<SimulationResult> CollectTopBruteForceResults()
{
	g_Working = false;

	for (auto& worker : g_Workers)
	{
		worker->join();
	}
	g_Workers.clear();

	assert(g_SimWorkQueue.size_approx() == 0);
	
	return results;
}