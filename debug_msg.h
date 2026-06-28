#ifndef DEBUG_MSG_H
#define DEBUG_MSG_H

#include <iostream>
#include <sstream>
#include <chrono>

/**
 * @file debug_msg.h
 * @brief Отладочный макрос DBG() для вывода сообщений в stderr.
 *
 * Включается/отключается через макрос препроцессора ENABLE_DEBUG_MESSAGES,
 * который должен быть определён в сборке (задаётся в CMakeLists.txt).
 *
 * Использование:
 *   DBG() << "x = " << x << ", y = " << y;
 *   DBG() << "Энергия: " << energy << " шаг: " << step;
 *
 * Для форматированного вывода (как printf):
 *   DBG_F("x = %d, y = %f", x, y);
 *
 * Для вывода с пометкой "WARNING":
 *   DBG_W() << "Значение вышло за пределы: " << val;
 *
 * Для вывода с пометкой "ERROR":
 *   DBG_E() << "Ошибка: " << msg;
 *
 * Для rate-limited версий (не чаще 1 раза в N секунд глобально):
 *   DBG_RL()  << "сообщение";
 *   DBG_W_RL() << "warning";
 *   DBG_E_RL() << "error";
 *   DBG_F_RL("fmt %d", val);
 */

// ──────────────────────────────────────────────────
// Настройка rate limiting (секунды)
// ──────────────────────────────────────────────────
#ifndef DEBUG_RATE_LIMIT_SECONDS
    /**
     * @brief Минимальный интервал (в секундах) между любыми отладочными
     *        сообщениями. Если любое debug-сообщение было выведено, то
     *        следующие DEBUG_RATE_LIMIT_SECONDS секунд все rate-limited
     *        сообщения игнорируются (глобальный rate limit).
     *
     * Определите этот макрос ДО включения заголовка, чтобы переопределить
     * значение по умолчанию (2 секунды).
     */
    #define DEBUG_RATE_LIMIT_SECONDS 2
#endif

// ──────────────────────────────────────────────────
// Логика включения/отключения
// ──────────────────────────────────────────────────
#ifdef ENABLE_DEBUG_MESSAGES

    // ──────────────────────────────────────────────
    // Вспомогательная функция для rate limiting
    // ──────────────────────────────────────────────
    namespace dbg_rate_limit {

        /**
         * @brief Проверяет глобальный rate limit.
         *
         * Возвращает true, если с последнего ВЫВЕДЕННОГО debug-сообщения
         * прошло не менее DEBUG_RATE_LIMIT_SECONDS секунд.
         *
         * Если возвращает true, то автоматически обновляет время
         * последнего вывода (блокирует последующие сообщения).
         *
         * @note Определение функции находится в debug_msg.cpp, чтобы
         *       статические переменные были едиными для всей программы.
         */
        bool check_global_rate_limit();

    } // namespace dbg_rate_limit

    // ──────────────────────────────────────────────
    // Базовый класс DebugOutput (без изменений)
    // ──────────────────────────────────────────────

    /**
     * @brief Вспомогательный класс для вывода отладочных сообщений.
     *
     * Собирает данные через operator<<, а в деструкторе выводит
     * в std::cerr с информацией о файле, функции и строке.
     */
    class DebugOutput {
    public:
        DebugOutput(const char* file, int line, const char* func,
                    const char* prefix = "[DEBUG]")
            : m_prefix(prefix)
        {
            // Сохраняем только имя файла (без пути)
            const char* basename = file;
            for (const char* p = file; *p; ++p) {
                if (*p == '/' || *p == '\\')
                    basename = p + 1;
            }
            m_stream << m_prefix << " "
                     << basename << ":" << line
                     << " (" << func << ") ";
        }

        virtual ~DebugOutput() {
            if (!m_suppress) {
                m_stream << std::endl;
                std::cerr << m_stream.str();
            }
        }

        // Оператор<< для любых типов
        template <typename T>
        DebugOutput& operator<<(const T& val) {
            m_stream << val;
            return *this;
        }

        // Специализация для манипуляторов (std::endl, std::hex, ...)
        DebugOutput& operator<<(std::ostream& (*manip)(std::ostream&)) {
            manip(m_stream);
            return *this;
        }

    protected:
        std::ostringstream m_stream;
        const char* m_prefix;
        bool m_suppress = false;
    };

    // ──────────────────────────────────────────────
    // Rate-limited версия DebugOutput
    // ──────────────────────────────────────────────

    /**
     * @brief Класс-обёртка над DebugOutput с глобальным rate limiting.
     *
     * Если с последнего выведенного debug-сообщения прошло меньше
     * DEBUG_RATE_LIMIT_SECONDS секунд, сообщение подавляется —
     * деструктор ничего не выводит в stderr.
     *
     * Rate limit общий для ВСЕХ rate-limited сообщений (глобальный
     * счётчик времени последнего вывода).
     */
    class RateLimitedDebugOutput : public DebugOutput {
    public:
        RateLimitedDebugOutput(const char* file, int line, const char* func,
                               const char* prefix = "[DEBUG]")
            : DebugOutput(file, line, func, prefix)
        { }

        ~RateLimitedDebugOutput() {
            if (dbg_rate_limit::check_global_rate_limit()) {
                m_stream << std::endl;
                std::cerr << m_stream.str();
            }
            // Подавляем вывод в ~DebugOutput(), т.к. мы уже всё вывели (или решили не выводить).
            m_suppress = true;
        }
    };

    // ──────────────────────────────────────────────
    // Обычные макросы (без rate limiting)
    // ──────────────────────────────────────────────

    /**
     * @brief Макрос для отладочного вывода.
     *
     * Пример:
     *   DBG() << "x = " << x;
     *   DBG() << "Энергия: " << energy << " МС-шаг: " << step;
     */
    #define DBG() DebugOutput(__FILE__, __LINE__, __func__)

    /**
     * @brief Макрос для отладочного вывода с пометкой WARNING.
     */
    #define DBG_W() DebugOutput(__FILE__, __LINE__, __func__, "[WARN]")

    /**
     * @brief Макрос для отладочного вывода с пометкой ERROR.
     */
    #define DBG_E() DebugOutput(__FILE__, __LINE__, __func__, "[ERROR]")

    /**
     * @brief Макрос для форматированного отладочного вывода (как printf).
     *
     * Пример:
     *   DBG_F("x = %d, y = %f", x, y);
     */
    #define DBG_F(fmt, ...)                                                 \
        do {                                                                \
            fprintf(stderr, "[DEBUG] %s:%d (%s) " fmt "\n",                \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);           \
        } while (0)

    // ──────────────────────────────────────────────
    // Rate-limited макросы (глобальный контроль частоты)
    // ──────────────────────────────────────────────

    /**
     * @brief Макрос для rate-limited отладочного вывода.
     *
     * Выводит сообщение не чаще 1 раза в DEBUG_RATE_LIMIT_SECONDS секунд.
     * Rate limit глобальный — если любое rate-limited сообщение было
     * выведено, следующие N секунд все rate-limited сообщения игнорируются.
     *
     * Пример:
     *   DBG_RL() << "x = " << x;
     */
    #define DBG_RL()   RateLimitedDebugOutput(__FILE__, __LINE__, __func__)

    /**
     * @brief Макрос для rate-limited вывода с пометкой WARNING.
     */
    #define DBG_W_RL() RateLimitedDebugOutput(__FILE__, __LINE__, __func__, "[WARN]")

    /**
     * @brief Макрос для rate-limited вывода с пометкой ERROR.
     */
    #define DBG_E_RL() RateLimitedDebugOutput(__FILE__, __LINE__, __func__, "[ERROR]")

    /**
     * @brief Макрос для rate-limited форматированного вывода (как printf).
     *
     * Пример:
     *   DBG_F_RL("x = %d, y = %f", x, y);
     */
    #define DBG_F_RL(fmt, ...)                                              \
        do {                                                                \
            if (dbg_rate_limit::check_global_rate_limit()) {                        \
                fprintf(stderr, "[DEBUG] %s:%d (%s) " fmt "\n",            \
                        __FILE__, __LINE__, __func__, ##__VA_ARGS__);       \
            }                                                               \
        } while (0)

#else
    // ── Режим "всё выключено" ──
    // Макросы раскрываются в пустоту, компилятор не генерирует код.

    #define DBG()            if (false) std::cerr
    #define DBG_W()          if (false) std::cerr
    #define DBG_E()          if (false) std::cerr
    #define DBG_F(fmt, ...)  ((void)0)

    #define DBG_RL()         if (false) std::cerr
    #define DBG_W_RL()       if (false) std::cerr
    #define DBG_E_RL()       if (false) std::cerr
    #define DBG_F_RL(fmt, ...) ((void)0)

#endif // ENABLE_DEBUG_MESSAGES

#endif // DEBUG_MSG_H