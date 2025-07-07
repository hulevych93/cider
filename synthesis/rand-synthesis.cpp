#include "rand-synthesis.h"

namespace cider {
namespace synthesis {

bool synthesize(std::mt19937& gen,
                const synthesis::RandSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                const recorder::Actions& initial,
                recorder::Actions& out) {
  out.clear();

  TestScenario scenario(gen, initial, objFunc);

  const auto actionChoosing = [&](const synthesis::TestScenario& testCase) {
    return testCase.getRandomAction();
  };

  details::synthesize(settings, actionChoosing, scenario);

  out = scenario.getResult();

  return true;
}

}  // namespace synthesis
}  // namespace cider
