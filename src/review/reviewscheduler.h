#pragma once

#include <QDate>

enum class ReviewRating
{
    Again = 1,
    Hard = 2,
    Good = 3,
    Easy = 4
};

struct ReviewSchedule
{
    QDate nextReviewDate;
    int intervalDays = 1;
    double easeFactor = 2.3;
    int lapseCount = 0;
    double retrievability = 0.0;
};

class ReviewScheduler
{
public:
    static ReviewSchedule schedule(ReviewRating rating,
                                   int previousIntervalDays,
                                   double previousEaseFactor,
                                   int previousLapseCount,
                                   const QDate &lastReviewDate,
                                   const QDate &today = QDate::currentDate());
};
