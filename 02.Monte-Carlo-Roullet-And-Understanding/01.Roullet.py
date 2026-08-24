import random

# I took reference and information from Prof. John Guttag's lecture on Monte Carlo Experiments.
# Most chapters in this repo (except the final re-implementation) are notes from his lecture.

# ============================================================
# MONTE CARLO SIMULATION
# A method of estimating the value of an unknown quantity
# using the principles of inferential statistics.
# ============================================================

# What is Inferential Statistics?
#   Population : a set of examples
#   Sample     : a proper subset of a population
#   Key Fact   : a random sample tends to exhibit the same
#                properties as the population it was drawn from.

# If we toss a coin and get 100 heads in a row, why do we become
# confident the next flip will also be heads?
# Whereas if we get 50 heads, then 48 tails, then 2 heads — why
# is our confidence different?

# Answer: as variance grows, we need larger samples to have the
# same degree of confidence.
# In the first case we never saw tails, so we assumed maybe
# that's just how the world is.

# ============================================================
# ROULETTE — a well-known game. Let's build a fair simulation.
# ============================================================

class FairRoulette():
    def __init__(self):
        self.pockets = []
        for i in range(1, 37):    # ball can land on 1 to 36
            self.pockets.append(i)
        self.ball = None
        self.pocketOdds = len(self.pockets) - 1  # 35 to 1 payout

    def spin(self):
        self.ball = random.choice(self.pockets)

    def betPocket(self, pocket, amt):
        if str(pocket) == str(self.ball):
            return amt * self.pocketOdds
        else:
            return -amt

    def __str__(self):
        return 'Fair Roulette'


def playRoulette(game, numSpins, pocket, bet, toPrint):
    totPocket = 0
    for i in range(numSpins):
        game.spin()
        totPocket += game.betPocket(pocket, bet)
    if toPrint:
        print(numSpins, 'spins of', game)
        print('Expected return betting', pocket, '=',
              str(round(100 * totPocket / numSpins, 3)) + '%\n')
    return (totPocket / numSpins)


game = FairRoulette()
for numSpins in (100, 1000000):
    for i in range(3):
        playRoulette(game, numSpins, 2, 1, True)


# ============================================================
# RESULTS (sample output — yours will vary due to randomness):
# ============================================================

# 100 spins of Fair Roulette
# Expected return betting 2 = -100.0%

# 100 spins of Fair Roulette
# Expected return betting 2 = 44.0%

# 100 spins of Fair Roulette
# Expected return betting 2 = -28.0%

# 1000000 spins of Fair Roulette
# Expected return betting 2 = -0.046%

# 1000000 spins of Fair Roulette
# Expected return betting 2 = 0.602%

# 1000000 spins of Fair Roulette
# Expected return betting 2 = 0.7964%

# With 100 spins the outcome is highly variable — very uncertain.
# With 1,000,000 spins the expected return closes in on 0%
# (the correct answer for a fair game), and the results are also
# much closer together — i.e., lower variance.
