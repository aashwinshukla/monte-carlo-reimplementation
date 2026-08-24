import random

# ============================================================
# UNFAIR ROULETTE — European and American variants
# Builds on the FairRoulette class from folder 02.
# ============================================================

class FairRoulette():
    def __init__(self):
        self.pockets = []
        for i in range(1, 37):
            self.pockets.append(i)
        self.ball = None
        self.pocketOdds = len(self.pockets) - 1   # 35 to 1 payout

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
        self.pockets.append('0')       # adds one zero → 37 pockets

    def __str__(self):
        return 'European Roulette'


class AmRoulette(EuRoulette):
    def __init__(self):
        EuRoulette.__init__(self)
        self.pockets.append('00')      # adds double zero → 38 pockets

    def __str__(self):
        return 'American Roulette'


# ============================================================
# SIMULATION
# ============================================================

def playRoulette(game, numSpins, pocket, bet, toPrint=False):
    totPocket = 0
    for i in range(numSpins):
        game.spin()
        totPocket += game.betPocket(pocket, bet)
    if toPrint:
        print(numSpins, 'spins of', game)
        print('Expected return betting', pocket, '=',
              str(round(100 * totPocket / numSpins, 4)) + '%\n')
    return totPocket / numSpins


def simulate(numTrials, numSpins):
    games = (FairRoulette(), EuRoulette(), AmRoulette())
    print(f'Simulate {numTrials} Trials of {numSpins} spins each')
    for game in games:
        totReturn = 0
        for t in range(numTrials):
            totReturn += playRoulette(game, numSpins, 2, 1)
        avgReturn = round(100 * totReturn / numTrials, 4)
        print(f'  Exp. return for {game} = {avgReturn}%')
    print()


for numSpins in (1000, 10000, 100000, 1000000):
    simulate(20, numSpins)


# ============================================================
# SAMPLE OUTPUT (your numbers will vary slightly):
# ============================================================

# Simulate 20 Trials of 1000 spins each
#   Exp. return for Fair Roulette     =  6.56%
#   Exp. return for European Roulette = -2.26%
#   Exp. return for American Roulette = -8.92%

# Simulate 20 Trials of 10000 spins each
#   Exp. return for Fair Roulette     = -1.234%
#   Exp. return for European Roulette = -4.168%
#   Exp. return for American Roulette = -5.752%

# Simulate 20 Trials of 100000 spins each
#   Exp. return for Fair Roulette     =  0.8144%
#   Exp. return for European Roulette = -2.6506%
#   Exp. return for American Roulette = -5.113%

# Simulate 20 Trials of 1000000 spins each
#   Exp. return for Fair Roulette     = -0.0723%
#   Exp. return for European Roulette = -2.7329%
#   Exp. return for American Roulette = -5.212%

# As spins increase, results converge to the true house edge:
#   Fair Roulette     → 0%     (no edge)
#   European Roulette → -2.70% (1/37 house edge)
#   American Roulette → -5.26% (2/38 house edge)
#
# No betting strategy can overcome this.
# The edge is baked into the wheel itself.
