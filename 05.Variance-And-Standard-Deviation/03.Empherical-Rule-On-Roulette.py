import random
import math

# ============================================================
# APPLYING THE EMPIRICAL RULE TO ROULETTE SIMULATIONS
# Uses variance, standard deviation, and 95% confidence
# intervals to quantify how reliable our simulation results are.
# ============================================================

# ------------------------------------------------------------
# ROULETTE CLASSES (carried forward from folder 04)
# ------------------------------------------------------------

class FairRoulette():
    def __init__(self):
        self.pockets = []
        for i in range(1, 37):
            self.pockets.append(i)
        self.ball = None
        self.pocketOdds = len(self.pockets) - 1   # 35 to 1

    def spin(self):
        self.ball = random.choice(self.pockets)

    def betPocket(self, pocket, amt):
        if str(pocket) == str(self.ball):
            return amt * self.pocketOdds
        else:
            return -amt

    def __str__(self):
        return 'Fair Roulette'


class EuRoulette(FairRoulette):
    def __init__(self):
        FairRoulette.__init__(self)
        self.pockets.append('0')       # 37 pockets

    def __str__(self):
        return 'European Roulette'


class AmRoulette(EuRoulette):
    def __init__(self):
        EuRoulette.__init__(self)
        self.pockets.append('00')      # 38 pockets

    def __str__(self):
        return 'American Roulette'


# ------------------------------------------------------------
# HELPER FUNCTIONS
# ------------------------------------------------------------

def findPocketReturn(game, numTrials, numSpins, toPrint):
    """
    Runs numTrials independent trials of numSpins spins each.
    Returns a list of the mean return per spin for each trial.
    """
    pocketReturns = []
    for t in range(numTrials):
        totReturn = 0
        for s in range(numSpins):
            game.spin()
            totReturn += game.betPocket(2, 1)
        pocketReturns.append(totReturn / numSpins)
    return pocketReturns


def getMeanAndStd(data):
    """
    Returns the mean and standard deviation of a list of values.
    Uses (N-1) denominator (sample standard deviation).
    """
    mean = sum(data) / len(data)
    variance = sum((x - mean) ** 2 for x in data) / (len(data) - 1)
    std = math.sqrt(variance)
    return mean, std


# ------------------------------------------------------------
# SIMULATION
# ------------------------------------------------------------

numTrials = 20
resultDict = {}
games = (FairRoulette, EuRoulette, AmRoulette)

# initialise result storage for each game
for G in games:
    resultDict[G().__str__()] = []

for numSpins in (1000, 10000, 100000, 1000000):
    print(f'\nSimulate betting a pocket for {numTrials} trials of {numSpins} spins each')
    for G in games:
        pocketReturns = findPocketReturn(G(), numTrials, numSpins, False)
        mean, std = getMeanAndStd(pocketReturns)

        # 95% confidence interval: mean +/- 1.96 * std
        # Note: std here is already the std of trial means (standard error),
        # because findPocketReturn returns one mean per trial.
        ci = 1.96 * std

        resultDict[G().__str__()].append((numSpins, 100 * mean, 100 * std))

        print(f'  Exp. return for {G()} = {round(100 * mean, 3)}%,'
              f' +/-{round(100 * ci, 3)}% with 95% confidence')


# ============================================================
# SAMPLE OUTPUT (your numbers will vary slightly):
# ============================================================

# Simulate betting a pocket for 20 trials of 1000 spins each
#   Exp. return for Fair Roulette     =  3.68%,  +/-27.189% with 95% confidence
#   Exp. return for European Roulette = -5.5%,   +/-35.042% with 95% confidence
#   Exp. return for American Roulette = -4.24%,  +/-26.494% with 95% confidence

# Simulate betting a pocket for 20 trials of 10000 spins each
#   Exp. return for Fair Roulette     =  1.2%,   +/-8.731%  with 95% confidence
#   Exp. return for European Roulette = -2.56%,  +/-9.114%  with 95% confidence
#   Exp. return for American Roulette = -5.11%,  +/-8.902%  with 95% confidence

# Simulate betting a pocket for 20 trials of 100000 spins each
#   Exp. return for Fair Roulette     =  0.125%, +/-3.999%  with 95% confidence
#   Exp. return for European Roulette = -3.313%, +/-3.515%  with 95% confidence
#   Exp. return for American Roulette = -5.594%, +/-4.287%  with 95% confidence

# Simulate betting a pocket for 20 trials of 1000000 spins each
#   Exp. return for Fair Roulette     =  0.012%, +/-0.846%  with 95% confidence
#   Exp. return for European Roulette = -2.679%, +/-0.948%  with 95% confidence
#   Exp. return for American Roulette = -5.176%, +/-1.214%  with 95% confidence


# ============================================================
# ANALYSIS OF RESULTS
# ============================================================

# --- WHAT THE CONFIDENCE INTERVAL TELLS US ---
#
# At 1,000 spins, the confidence intervals are enormous (~±27-35%).
# The mean estimates are all over the place.
# You can't distinguish Fair from European from American Roulette
# at this sample size — the noise drowns out the signal.
#
# At 100,000 spins the picture becomes clear:
#   Fair Roulette     →  0.125%  (should be   0%)     CI: ±3.999%
#   European Roulette → -3.313%  (should be -2.70%)   CI: ±3.515%
#   American Roulette → -5.594%  (should be -5.26%)   CI: ±4.287%
# The true values are inside the confidence interval every time.
#
# At 1,000,000 spins the intervals are very tight (~±1%):
#   Fair Roulette     →  0.012%   → almost exactly 0%
#   European Roulette → -2.679%   → almost exactly -2.70%
#   American Roulette → -5.176%   → almost exactly -5.26%
# The Law of Large Numbers has fully kicked in.

# --- THE SHRINKING INTERVAL ---
#
# Notice the CI width shrinks as numSpins increases:
#   1,000 spins    → ~±27-35%
#   10,000 spins   → ~±9%
#   100,000 spins  → ~±4%
#   1,000,000 spins → ~±1%
#
# This follows the standard error formula: SE = σ / sqrt(N)
# Every 10x increase in spins shrinks the interval by ~sqrt(10) ≈ 3.16x
# To halve the interval you need 4x the spins — slow but guaranteed.

# --- FAIR ROULETTE CONVERGES TO 0% ---
#
# With no house edge, the expected return is exactly 0%.
# The simulation confirms this — at 1,000,000 spins it's 0.012%.
# The deviation from 0 is just residual randomness, well within the CI.

# --- EUROPEAN vs AMERICAN ---
#
# European: house edge = 1/37 ≈ 2.703%  → simulation gives -2.679% ✓
# American: house edge = 2/38 ≈ 5.263%  → simulation gives -5.176% ✓
# The simulation is accurately recovering the true theoretical values.
# This is Monte Carlo working exactly as intended.
