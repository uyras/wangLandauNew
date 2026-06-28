/**
 * @file debug_msg.cpp
 * @brief Определение функции rate limiting для отладочных макросов.
 *
 * Функция вынесена в отдельную единицу трансляции, чтобы статические
 * переменные (s_last_output, s_min_interval) были едиными для всей
 * программы, а не дублировались в каждой единице трансляции,
 * включающей debug_msg.h.
 *
 * @see debug_msg.h
 */

#include "debug_msg.h"

#include <chrono>

namespace dbg_rate_limit {

bool check_global_rate_limit() {
    using clock = std::chrono::steady_clock;
    using seconds = std::chrono::duration<double>;

    // ВАЖНО: инициализируем нулевым time_point (эпоха часов), а не time_point::min().
    // time_point::min() — это минимально представимое значение, и разница
    // now - time_point::min() может вызвать переполнение при приведении к double.
    static clock::time_point s_last_output{};
    static const double s_min_interval = DEBUG_RATE_LIMIT_SECONDS;

    auto now = clock::now();
    double elapsed = std::chrono::duration_cast<seconds>(
        now - s_last_output).count();

    if (elapsed >= s_min_interval) {
        s_last_output = now;
        return true;  // можно выводить
    }

    return false;  // слишком часто — игнорируем
}

} // namespace dbg_rate_limit