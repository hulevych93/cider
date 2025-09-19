// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dynamic-slicer.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

namespace cider {
namespace dslicer {

std::vector<recorder::Action> run_d_slicing_fast_tracks(
    const BatchTracksDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace)
{
    auto currentSpace = deepCopy(actionSpace);

    double baseline = settings.baseline;
    if (baseline <= std::numeric_limits<double>::epsilon()) {
        baseline = settings.objFunc(currentSpace).coverage;
    }

    const auto curCov = settings.objFunc(currentSpace);
    if (curCov.coverage + 1e-9 < baseline) {
        // вихідний сценарій вже не досягає базового порогу
        return currentSpace;
    }

    FineObjectiveValue fineRes = settings.fineObjFunc(currentSpace);
    if (fineRes.fineCoveredTracks.empty()) return currentSpace;

    // визначаємо реальний trackCount
    size_t trackCount = 0;
    for (const auto& v : fineRes.fineCoveredTracks) {
        if (!v.empty()) { trackCount = v.size(); break; }
    }
    if (trackCount == 0) {
        std::cout << "[Slice-Batch-Trace] No coverage tracks found\n";
        return currentSpace;
    }

    struct Step final {
        recorder::Action action;
        std::vector<std::uint8_t> coverage; // 1 тільки для унікальних блоків цього кроку
    };

    std::vector<Step> steps;
    steps.reserve(currentSpace.size());
    for (size_t i = 0; i < currentSpace.size(); ++i) {
        std::vector<std::uint8_t> cov = fineRes.fineCoveredTracks[i];
        if (cov.empty()) cov.assign(trackCount, 0);
        steps.push_back({ currentSpace[i], std::move(cov) });
    }

    std::cout << "[Slice-Batch-Trace] Original length=" << steps.size()
              << " tracks=" << trackCount << " baseline=" << baseline << "\n";

    // частоти покриття треків у поточному наборі кроків
    std::vector<int> freq(trackCount, 0);
    for (const auto& s : steps)
        for (size_t j = 0; j < trackCount; ++j)
            if (s.coverage[j]) ++freq[j];

    const size_t batchSize = std::max<size_t>(1, 30);
    size_t removedTotal = 0;

    // снапшот пачки
    std::vector<Step>     snapSteps;
    std::vector<int>      snapFreq;
    size_t                snapI = 0;
    bool                  hasSnapshot = false;
    size_t                pendingInBatch = 0;

    auto takeSnapshot = [&](size_t i) {
        snapSteps   = steps;
        snapFreq    = freq;
        snapI       = i;
        hasSnapshot = true;
        pendingInBatch = 0;
    };

    auto buildScenario = [&]() {
        std::vector<recorder::Action> out;
        out.reserve(steps.size());
        for (auto& s : steps) out.push_back(s.action);
        return out;
    };

    size_t i = 0;
    while (i < steps.size()) {
        if (!hasSnapshot) takeSnapshot(i);

        const auto& cov = steps[i].coverage;
        bool removable = true;
        int uniq = 0, dup = 0;

        for (size_t j = 0; j < trackCount; ++j) {
            if (!cov[j]) continue;
            if (freq[j] == 1) { removable = false; ++uniq; }
            else { ++dup; }
        }

        if (removable) {
            // тимчасово видаляємо (частина «непідтвердженої» пачки)
            for (size_t j = 0; j < trackCount; ++j)
                if (cov[j]) --freq[j];

            std::cout << "  [Slice-Batch-Trace] Removing step " << i
                      << " (uniq=" << uniq << ", dup=" << dup << ")\n";

            steps.erase(steps.begin() + i);  // не інкрементуємо i, бо елементи зсунулись
            ++pendingInBatch;

            // досягли межі пачки або кінець списку → перевірка
            if (pendingInBatch >= batchSize || i >= steps.size()) {
                auto scenario = buildScenario();
                auto newCov = settings.objFunc(scenario);
                std::cout << "  [Check@" << (removedTotal + pendingInBatch)
                          << "] coverage=" << newCov.coverage
                          << " baseline=" << baseline << "\n";

                if (newCov.coverage + 1e-9 < baseline) {
                    // ❌ Ролбек ВСІЄЇ пачки (атомарно)
                    std::cout << "[WARN] Drop detected → rollback last "
                              << pendingInBatch << " removals\n";
                    steps   = std::move(snapSteps);
                    freq    = std::move(snapFreq);
                    // щоб не зациклитись на тій самій позиції – зсуваємось далі
                    i       = std::min(snapI + 1, steps.size());
                    hasSnapshot = false;    // нову пачку почнемо зі свіжим снапшотом
                    pendingInBatch = 0;
                } else {
                    // ✅ Коміт пачки
                    removedTotal += pendingInBatch;
                    hasSnapshot = false;
                    pendingInBatch = 0;
                    // i не змінюємо: ми йдемо далі по зсунутих кроках
                }
            }
        } else {
            std::cout << "  [Slice-Batch-Trace] Keeping  step " << i
                      << " (uniq=" << uniq << ", dup=" << dup << ")\n";
            ++i;
        }
    }

    // результат
    std::vector<recorder::Action> result;
    result.reserve(steps.size());
    for (auto& s : steps) result.push_back(s.action);

    auto finalCov = settings.objFunc(result);
    std::cout << "[Slice-Batch-Trace] Final length=" << result.size()
              << " coverage=" << finalCov.coverage
              << " baseline=" << baseline << "\n";

    return result;
}

}  // namespace dslicer
}  // namespace cider
