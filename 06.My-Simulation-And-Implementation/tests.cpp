void test1() {
    int in_circle = 0;
    int total_trials = 10;

    // Define distribution outside the loop
    std::uniform_real_distribution<double> realDist(-1.0, 1.0);

    for (int i = 0; i < total_trials; i++) {
        double x = realDist(gen);
        double y = realDist(gen);

        // x^2 + y^2 <= 1
        double c = (x * x) + (y * y);
        
        if (c <= 1.0) {
            in_circle++;
        }
    }

    std::cout << "Probability Ratio of area of circle / area of square = PI/4\n";
    std::cout << "Probability Ratio * 4 = PI\n";

    // Use floating-point division (10.0 instead of 10)
    double estimate = (static_cast<double>(in_circle) / total_trials) * 4.0;

    std::cout << "In this test of 10 Tries the Pi estimate is: " << estimate << "\n";
    std::cout << "Theoretical Error Bound (~1/sqrt(N)): " << (1.0 / std::sqrt(total_trials)) << "\n";
}