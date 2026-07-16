#include "review/reviewscheduler.h"

#include <cmath>

namespace {
// easeFactor 越大，答对后复习间隔增长得越快。
// 设置上下限可以防止连续评分导致间隔增长过慢或过快。
constexpr double kDefaultEaseFactor = 2.3;
constexpr double kMinimumEaseFactor = 1.3;
constexpr double kMaximumEaseFactor = 3.0;
}

ReviewSchedule ReviewScheduler::schedule(ReviewRating rating,
                                          int previousIntervalDays,
                                          double previousEaseFactor,
                                          int previousLapseCount,
                                          const QDate &lastReviewDate,
                                          const QDate &today)
{
    ReviewSchedule result;

    // 对数据库中的历史状态进行容错：间隔和遗忘次数不能为负数，
    // 无效的难度因子回退到默认值，并始终限制在允许范围内。
    const int previousInterval = qMax(0, previousIntervalDays); // 归一化后的上次间隔（天）。
    const double previousEase = previousEaseFactor > 0.0
        ? previousEaseFactor
        : kDefaultEaseFactor; // 有效的历史难度因子。
    result.easeFactor = qBound(kMinimumEaseFactor, previousEase, kMaximumEaseFactor);
    result.lapseCount = qMax(0, previousLapseCount);

    // 使用指数遗忘曲线 R = exp(-t / S) 估计当前记忆保持率：
    // t 是距上次复习的实际天数，S 用上一次复习间隔近似表示记忆稳定性。
    // 没有有效复习日期时，用计划间隔代替实际经过天数。
    const int elapsedDays = lastReviewDate.isValid() // 距上次复习实际经过的天数。
        ? qMax(0, lastReviewDate.daysTo(today))
        : previousInterval;
    const double stability = qMax(1, previousInterval); // 记忆稳定性 S，使用历史间隔近似。
    result.retrievability = std::exp(-elapsedDays / stability);

    // 复习越晚，保持率越低，successfulDelayBonus 越大。
    // 若用户此时仍能答对，就适当放大下一次复习间隔。
    const double successfulDelayBonus = 1.0 - result.retrievability; // 延迟后仍答对的间隔奖励。

    if (previousInterval == 0) {
        // 新单词没有可供倍增的历史间隔，因此按首次评分设置固定间隔。
        switch (rating) {
        case ReviewRating::Again:
            result.intervalDays = 1;
            result.easeFactor -= 0.20;
            ++result.lapseCount;
            break;
        case ReviewRating::Hard:
            result.intervalDays = 2;
            result.easeFactor -= 0.15;
            break;
        case ReviewRating::Good:
            result.intervalDays = 3;
            result.easeFactor += 0.05;
            break;
        case ReviewRating::Easy:
            result.intervalDays = 5;
            result.easeFactor += 0.15;
            break;
        }
    } else {
        // 已学习单词根据评分调整间隔：
        // Again 重置；Hard 缓慢增长；Good 按难度因子正常增长；
        // Easy 在难度因子的基础上获得额外增长。
        switch (rating) {
        case ReviewRating::Again:
            result.intervalDays = 1;
            result.easeFactor -= 0.20;
            ++result.lapseCount;
            break;
        case ReviewRating::Hard:
            result.intervalDays = qMax(previousInterval + 1,
                                       qRound(previousInterval * (1.20 + 0.10 * successfulDelayBonus)));
            result.easeFactor -= 0.15;
            break;
        case ReviewRating::Good:
            result.intervalDays = qMax(previousInterval + 1,
                                       qRound(previousInterval * (result.easeFactor + 0.30 * successfulDelayBonus)));
            result.easeFactor += 0.05;
            break;
        case ReviewRating::Easy:
            result.intervalDays = qMax(previousInterval + 2,
                                       qRound(previousInterval * (result.easeFactor + 0.80 + 0.40 * successfulDelayBonus)));
            result.easeFactor += 0.15;
            break;
        }
    }

    // 评分改变难度因子后再次执行边界限制，并由新间隔得到下次日期。
    result.easeFactor = qBound(kMinimumEaseFactor, result.easeFactor, kMaximumEaseFactor);
    result.nextReviewDate = today.addDays(result.intervalDays);
    return result;
}
