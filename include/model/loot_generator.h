#pragma once
#include <chrono>
#include <cmath>
#include <functional>

namespace loot_gen {

/*
 *  Генератор трофеев
 */
class LootGenerator {
public:
    using RandomGenerator = std::function<double()>;
    using TimeInterval = std::chrono::milliseconds;

    /*
     * base_interval - базовый отрезок времени > 0
     * probability - вероятность появления трофея в течение базового интервала времени
     * random_generator - генератор псевдослучайных чисел в диапазоне от [0 до 1]
     */
    LootGenerator(TimeInterval base_interval, double probability,
                  RandomGenerator random_gen = DefaultGenerator)
            : base_interval_(base_interval)
            , probability_(probability)
            , random_generator_(std::move(random_gen)) {
    }

    const auto& getPeriod() { return base_interval_; }

    /*
     * Возвращает количество трофеев, которые должны появиться на карте спустя
     * заданный промежуток времени.
     * Количество трофеев, появляющихся на карте не превышает количество мародёров.
     *
     * time_delta - отрезок времени, прошедший с момента предыдущего вызова Generate
     * loot_count - количество трофеев на карте до вызова Generate
     * looter_count - количество мародёров на карте
     */
    unsigned Generate(TimeInterval time_delta, unsigned loot_count, unsigned looter_count){
        time_without_loot_ += time_delta;
        const unsigned loot_shortage = loot_count > looter_count ? 0u : looter_count - loot_count;
        const double ratio = std::chrono::duration<double>{time_without_loot_} / base_interval_;
        const double probability = std::clamp((1.0 - std::pow(1.0 - probability_, ratio)) * random_generator_(), 0.0, 1.0);
        const unsigned generated_loot = static_cast<unsigned>(std::round(loot_shortage * probability));
        if (generated_loot > 0) {
            time_without_loot_ = {};
        }
        return generated_loot;
    }

private:
    static double DefaultGenerator() noexcept {
        return 1.0;
    };
    TimeInterval base_interval_;
    double probability_;
    TimeInterval time_without_loot_{};
    RandomGenerator random_generator_;
};

}  // namespace loot_gen