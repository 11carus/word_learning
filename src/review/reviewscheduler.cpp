#include "review/reviewscheduler.h"

#include <cmath>

namespace {
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
    const int previousInterval = qMax(0, previousIntervalDays);
    const double previousEase = previousEaseFactor > 0.0 ? previousEaseFactor : kDefaultEaseFactor;
    result.easeFactor = qBound(kMinimumEaseFactor, previousEase, kMaximumEaseFactor);
    result.lapseCount = qMax(0, previousLapseCount);

    const int elapsedDays = lastReviewDate.isValid()
        ? qMax(0, lastReviewDate.daysTo(today))
        : previousInterval;
    const double stability = qMax(1, previousInterval);
    result.retrievability = std::exp(-elapsedDays / stability);
    const double successfulDelayBonus = 1.0 - result.retrievability;

    if (previousInterval == 0) {
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

    result.easeFactor = qBound(kMinimumEaseFactor, result.easeFactor, kMaximumEaseFactor);
    result.nextReviewDate = today.addDays(result.intervalDays);
    return result;
}
