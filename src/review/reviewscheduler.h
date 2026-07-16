#pragma once

#include <QDate>

enum class ReviewRating
{
    Again = 1, // 完全忘记，需要重置为短间隔。
    Hard = 2,  // 回忆困难，只小幅增加间隔。
    Good = 3,  // 正常记住，按难度因子增加间隔。
    Easy = 4   // 轻松记住，额外增加间隔。
};

struct ReviewSchedule
{
    QDate nextReviewDate;             // 本次计算得到的下次复习日期。
    int intervalDays = 1;            // 从今天到下次复习的间隔，单位：天。
    double easeFactor = 2.3;         // 难度因子；数值越大，答对后的间隔增长越快。
    int lapseCount = 0;              // 累计遗忘次数，即选择 Again 的次数。
    double retrievability = 0.0;     // 当前记忆保持率，取值范围为 [0, 1]。
};

class ReviewScheduler
{
public:
    // rating：本次自评结果。
    // previousIntervalDays：上一次安排的复习间隔（天）。
    // previousEaseFactor：计算前的难度因子。
    // previousLapseCount：计算前的累计遗忘次数。
    // lastReviewDate：上一次实际复习日期；无历史记录时可以是无效日期。
    // today：本次调度基准日期，默认取系统当前日期。
    static ReviewSchedule schedule(ReviewRating rating,
                                   int previousIntervalDays,
                                   double previousEaseFactor,
                                   int previousLapseCount,
                                   const QDate &lastReviewDate,
                                   const QDate &today = QDate::currentDate());
};
